RecurringGroup : AbstractGroup {
	var <>target, <>addAction, <recurs = true, <>recurred = false;

	*new {  arg target, addAction=\addToHead;
		var group, server, inTarget;
		inTarget = target.asTarget;
		if(inTarget.recurs) {
			server = inTarget.server ? Server.default;
			group = this.basicNew(server);
			group.target = inTarget;
			group.addAction = addAction;
			group.createNode;
			CmdPeriod.add(group);
			ServerTree.add(group);
			^group;
		} {
			"A RecurringGroup must have a recurring node as a target".error;
		};
	}

	asTarget {
		if(recurred.not) {
			nodeID = server.nextNodeID;
			this.createNode
		};
		^this;
	}

	createNode {
		var addNum;
		target = target.asTarget;
		addNum = addActions[addAction];
		if(addNum < 2) { group.group = target } { group.group = target.group };
		server.sendMsg(this.class.creationCmd, nodeID, addActions[addAction],
			target.asTarget.nodeID);
		recurred = true;
		this.changed(\nodeCreated);
	}

	cmdPeriod { recurred = false; }

	doOnServerTree { if(recurred.not, { nodeID = server.nextNodeID; this.createNode }) }

	free {
		CmdPeriod.remove(this);
		ServerTree.remove(this);
		target = nil;
		recurs = false;
		recurred = nil;
	} // I don't really want to be a zombie

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