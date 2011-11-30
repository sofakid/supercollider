RecurringGroup : AbstractGroup {
	var <target, <>addAction,
		<recurs = true, <>recurred = false,
		goWatcher, endWatcher;

	*new { arg target, addAction=\addToHead;
		^this.basicNew(target.asTarget.server)
			.init(target, addAction)
			.createNode;
	}

	init { |target, addAction=\addToHead|
		var inTarget;
		inTarget = target.asTarget;
		server = inTarget.server ?? { Server.default };
		this.target = inTarget;
		this.addAction = addAction;
		CmdPeriod.add(this);
		ServerTree.add(this);
		if(server.notified) {
			goWatcher = OSCFunc({ |msg|
				if(msg[1] == nodeID) {
					endWatcher.enable;
				};
			}, '/n_go', server.addr);
			endWatcher = OSCFunc({ |msg|
				if(msg[1] == nodeID and: { recurred == true }) {
					isPlaying = false;
					this.free;
				}
			}, '/n_end', server.addr);
		};
	}

	asTarget {
		if(recurred.isNil) {
			MethodError(
"Attempt to reuse a RecurringGroup that was already explicitly freed.

Another RecurringNode that depends on this RecurringGroup still exists and has been freed. To avoid this error in the future, free it or change its target to an active RecurringNode when freeing the parent RecurringGroup.", this
			).throw
		};
		if(recurred.not) {
			this.createNode
		};
		^this;
	}

	createNode {
		var addNum, ok;
		try {
			target = target.asTarget;
			ok = true;
		} { |err|
			err.reportError;
			// couldn't be recreated, so it's no longer valid
			// this may cascade errors
			// if a bunch of other recurring nodes depend on this one
			this.free;
			ok = false;
		};
		if(ok) {
			addNum = addActions[addAction];
			if(addNum < 2) { this.group = target } { this.group = target.group };
			nodeID ?? { nodeID = server.nextNodeID };
			server.sendMsg(this.class.creationCmd, nodeID, addActions[addAction],
				target.asTarget.nodeID);
			recurred = true;
			isPlaying = true;
			this.changed(\nodeCreated);
		};
	}

	cmdPeriod {
		recurred = false;
		endWatcher.tryPerform(\disable);
		isPlaying = false;
		// mildly hacky: server's default group should not reassign id
		// all other groups: invalidate nodeID to assign a new one in createNode
		if(nodeID != 1) { nodeID = nil };
	}

	doOnServerTree {
		if(recurred.not) {
			this.createNode;
		};
	}

	free {
		CmdPeriod.remove(this);
		ServerTree.remove(this);
		// watchers.do(_.free);
		goWatcher.free;
		endWatcher.free;
		if(isPlaying or: { server.notified.not }) {
			super.free
		};
		target = nil;
		recurs = false;
		recurred = nil;
		isPlaying = false;
	}

	target_ { |aTarget|
		if(aTarget.recurs) {
			target = aTarget
		} {
			Error("A RecurringGroup must have a recurring node as a target").throw;
		}
	}

	*creationCmd { ^21 }	//"/g_new"
}

RecurringParGroup : RecurringGroup {
	*creationCmd { ^63 }	//"/p_new"
}

+ Node {
	recurs { ^false }
}

+ RootNode {
	recurs { ^true }
}