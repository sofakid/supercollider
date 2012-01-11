Main : Process {
	// do not change the next lines manually:
	//==== replace with new version from bash script ====
classvar scVersionMajor=3, scVersionMinor=5, scVersionPostfix="beta";
	//==== end replace ====

	var <platform, argv;
	var recvOSCfunc, prRecvOSCFunc;
	var <customPorts;

		// proof-of-concept: the interpreter can set this variable when executing code in a file
		// should be nil most of the time

	startup {
	    var didWarnOverwrite = false;
		// setup the platform first so that class initializers can call platform methods.
		// create the platform, then intialize it so that initPlatform can call methods
		// that depend on thisProcess.platform methods.
		platform = this.platformClass.new;
		platform.initPlatform;

		super.startup;

		// set the 's' interpreter variable to the default server.
		interpreter.s = Server.default;
		GUI.fromID( this.platform.defaultGUIScheme );
		GeneralHID.fromID( this.platform.defaultHIDScheme );
		this.platform.startup;
		StartUp.run;

		("Welcome to SuperCollider" + Main.version
			++ (Platform.ideName.switch(
				"scvim", {", type :SChelp for help"},
				"scel",  {", type ctrl-c ctrl-h for help"},
				"sced",  {", type ctrl-U for help"},
				"scapp", {", type cmd-d for help"}
			) ?? {
				(
					osx: ", type cmd-d for help",
					linux: ", for help type ctrl-c ctrl-h (Emacs) or :SChelp (vim) or ctrl-U (sced/gedit)",
				 	windows: ", press F1 for help",
					iphone: ""
				 ).at(platform.name);

			})
		).postln;

		Main.overwriteMsg.split(Char.nl).drop(-1).collect(_.split(Char.tab)).do {|x|
			if(x[2].beginsWith(Platform.classLibraryDir) and: {x[1].contains("/SystemOverwrites/").not}
			) {
				warn("Extension in '%' overwrites % in main class library.".format(x[1],x[0]));
				didWarnOverwrite = true;
			}
		};
		if(didWarnOverwrite) {
			inform("\nIntentional overwrites must be put in a 'SystemOverwrites' subfolder.")
		}
	}

	shutdown { // at recompile, quit
		Server.quitAll;
		this.platform.shutdown;
		super.shutdown;
	}

	run { // used to be called by command-R, customisation now via CocoaMenuItem
		//interpreter.interpretPrintCmdLine;
	}

	stop { // called by command-.
		CmdPeriod.run;
	}

	hardStop { // called by command alt dot
		CmdPeriod.hardRun;
	}

	recvOSCmessage { arg time, replyAddr, recvPort, msg;
		// this method is called when an OSC message is received.
		recvOSCfunc.value(time, replyAddr, recvPort, msg);
		prRecvOSCFunc.value(time, replyAddr, recvPort, msg);
		OSCresponder.respond(time, replyAddr, msg);
	}

	recvOSCbundle { arg time, replyAddr, recvPort ... msgs;
		// this method is called when an OSC bundle is received.
		msgs.do({ arg msg;
			this.recvOSCmessage(time, replyAddr, recvPort, msg);
		});
	}

	addOSCRecvFunc { |func| prRecvOSCFunc = prRecvOSCFunc.addFunc(func) }
	
	removeOSCRecvFunc { |func| prRecvOSCFunc = prRecvOSCFunc.removeFunc(func) }
	
	replaceOSCRecvFunc { |func, newFunc| prRecvOSCFunc = prRecvOSCFunc.replaceFunc(func, newFunc) }
	
	openUDPPort {|portNum|
		var result;
		result = this.prOpenUDPPort(portNum);
		if(result, { customPorts = customPorts ++ [portNum]; });
		^result;
	}
	
	prOpenUDPPort {|portNum|
		_OpenUDPPort
	}
	
	newSCWindow {
		var win, palette;
		win = SCWindow("construction");
		win.front;
		win.toggleEditMode;
	}

//	override in platform specific extension
//
//	platformClass {
//		^Platform
//	}

	argv {
		^argv ?? { argv = this.prArgv }
	}

	showHelpBrowser {
		HelpBrowser.openBrowsePage;
	}
	showHelpSearch {
		HelpBrowser.openSearchPage(this.getCurrentSelection);
	}
	showHelp {
		HelpBrowser.openHelpFor(this.getCurrentSelection);
	}

	showClassBrowser {
		var string, class, method, words;
		string = interpreter.cmdLine;
		class = string.asSymbol.asClass;
		(class ? Object).browse;
	}

	*version {^[scVersionMajor, ".", scVersionMinor, scVersionPostfix].join}

	*versionAtLeast { |maj, min|
		^if((maj==scVersionMajor) and:{min.notNil}){
			scVersionMinor >= min
		}{
			scVersionMajor >= maj
		};
	}

	*versionAtMost { |maj, min|
		^if((maj==scVersionMajor) and:{min.notNil}){
			scVersionMinor <= min
		}{
			scVersionMajor <= maj
		};
	}

	pid {
		_GetPid
		^this.primitiveFailed
	}

	// PRIVATE
	prArgv {
		_Argv
		^[]
	}

	recompile { platform.recompile }

	escapeWindow { platform.escapeWindow }

	exitFullScreen { platform.exitFullScreen }

	setDeferredTaskInterval { |interval| platform.setDeferredTaskInterval(interval) }

	*overwriteMsg { _MainOverwriteMsg ^this.primitiveFailed }
}


MethodOverride {
	var <ownerClass, <selector, <activePath, <overriddenPath;

	*new { arg ownerClass, selector, activePath, overriddenPath;
		^super.newCopyArgs(ownerClass, selector, activePath, overriddenPath)
	}

	*fromLine { arg string;
		var parts = string.split(Char.tab);
		var class, selector;
		#class, selector = parts[0].split($:);
		^this.new(class.asSymbol.asClass, selector, parts[1], parts[2])
	}

	openFiles {
		var path2 = if(overriddenPath.beginsWith("/Common")) {
			Platform.classLibraryDir +/+ overriddenPath
			} { overriddenPath };
		activePath.openTextFile;
		path2.openTextFile;
	}

	*simplifyPath { arg path;
		var extDir, sysExtDir, quarkDir;
		extDir = Platform.userExtensionDir;
		sysExtDir = Platform.systemExtensionDir;
		quarkDir = LocalQuarks.globalPath;
		path = path.replace("'" ++ extDir, "Platform.userExtensionDir ++ '");
		path = path.replace("'" ++ sysExtDir, "Platform.systemExtensionDir ++ '");
		path = path.replace("'" ++ quarkDir, "LocalQuarks.globalPath ++ '");
		^path

	}

	*all {
		var msg = Main.overwriteMsg.drop(-1); // drop last newline
		var lines = msg.split(Char.nl);
		^lines.collect { |line| this.fromLine(line) };
	}

	*printAll { arg simplifyPaths = true;
		var all = this.all;
		var classes = all.collect(_.ownerClass).as(Set);
		if(all.isEmpty) { "There are no overwritten methods in class library".postln; ^this };
		("Overwritten methods in class library:".underlined ++ "\n\n").post;
		classes.do { |class|
			class.asString.underlined.postln;
			all.select { |x| x.ownerClass == class }.do { |x|
				var activePath = x.activePath;
				var overriddenPath = x.overriddenPath;
				if(simplifyPaths) {
					activePath = this.simplifyPath(x.activePath);
					overriddenPath = this.simplifyPath(x.overriddenPath);
				};
				("\t" ++ x.ownerClass.name ++ ":" ++ x.selector).postln;
				("\t\t" ++ activePath).postln;
				("\t\t" ++ overriddenPath).postln;
			};
			"\n".post;
		}
	}

	*printAllShort {
		var all = this.all;
		var classes = all.collect(_.ownerClass).as(Set);
		if(all.isEmpty) { "There are no overwritten methods in class library".postln; ^this };
		("Overwritten methods in class library:".underlined ++ "\n").post;
		classes.do { |class|
			all.select { |x| x.ownerClass == class }.collect { |x| x.selector }.as(Set).do { |x|
				postf("\t%:%\n", class, x);
			}
		}
	}

}
