ScIDE {
	classvar subListSorter;
	classvar <>currentPath;
	classvar <defaultServer;
	classvar serverController;
	classvar volumeController, suppressAmpResponse = false;
	classvar docRoutine;

	*initClass {
		subListSorter = { | a b | a[0].perform('<', b[0]) };

		Class.initClassTree(Server);

		StartUp.add {
			if (ScIDE.connected) {
				this.handshake
			};
		}
	}

	*connect {|ideName|
		this.prConnect(ideName);
		defer {
			this.handshake
		}
	}

	*handshake {
		this.prSend(\classLibraryRecompiled);
		this.prSend(\requestDocumentList);
		this.prSend(\requestCurrentPath);

		this.defaultServer = Server.default;
		this.sendIntrospection;
	}

	*defaultServer_ {|server|
		serverController.remove;
		serverController = SimpleController(server)
		.put(\serverRunning, { | server, what, extraArg |
			this.prSend(\defaultServerRunningChanged, [
				server.serverRunning, server.addr.hostname, server.addr.port]);
		})
		.put(\default, { | server, what, newServer |
			("changed default server to:" + newServer.name).postln;
			this.defaultServer = newServer;
		});

		volumeController.remove;
		volumeController = SimpleController(server.volume)
		.put(\mute, { | volume, what, muted |
			this.prSend( if(muted, \serverMuted, \serverUnmuted) );
		})
		.put(\amp, { | volume, what, amp |
			if (not(suppressAmpResponse)) {
				this.prSend( \serverAmp, amp.asString );
			};
		})
		.put(\ampRange, { |volume, what, min, max|
			this.prSend( \serverAmpRange, "%,%".format(min, max) );
		});

		defaultServer = server;

		this.prSend(\defaultServerRunningChanged, [
			server.serverRunning, server.addr.hostname, server.addr.port]);
		this.prSend( if (server.volume.isMuted, \serverMuted, \serverUnmuted) );
		this.prSend( \serverAmpRange, "%,%".format(server.volume.min, server.volume.max) );
		this.prSend( \serverAmp, server.volume.volume.asString );
	}

	*request { |id, command, data|
		this.tryPerform(command, id, data);
	}

	*connected {
		_ScIDE_Connected
	}

	*open { |path, charPos = 0, selectionLength = 0|
		this.prSend(\openFile, [path, charPos, selectionLength])
	}

	*setServerVolume { arg volume;
		var suppressed = suppressAmpResponse;
		suppressAmpResponse = true;
		defaultServer.volume = volume;
		suppressAmpResponse = suppressed;
	}

	*sendIntrospection {
		var res = [];
		Class.allClasses.do { |class|
			var classData;
			classData = [
				class.name,
				class.class.name,
				class.superclass !? {class.superclass.name},
				class.filenameSymbol,
				class.charPos,
				class.methods.collect { |m| this.serializeMethodDetailed(m) };
			];
			res = res.add(classData);
		};
		this.prSend(\introspection, res);
	}

	*sendAllClasses { |id|
		this.prSend(id, Class.allClasses.collectAs(_.asString, Array))
	}

	*sendSymbolTable { |id|
		var result, dt;

		dt = {
			result = IdentitySet(16384);
			Class.allClasses.do { | class |
				if (class.isMetaClass.not) {
					result.add(class.name);
				};
				class.methods.do { | method |
					result.add(method.name);
				};
			};

			result = result.collectAs(_.asString, Array)
		}.bench(false);

		"ScIDE: Built symbol table in % seconds\n".postf(dt.asStringPrec(3));

		this.prSend(id, result)
	}

	*completeClass { |id, text|
		var res = [];
		Class.allClasses.do { |class|
			var name = class.name.asString;
			if (name.beginsWith(text)) {
				res = res.add(name);
			};
		};
		if (res.size > 0) {
			this.prSend(id, res);
		};
	}

	*completeClassMethod { |id, text|
		var class, methods, res;
		class = text.asSymbol.asClass;
		if (class.notNil) {
			methods = IdentityDictionary();
			class = class.class;
			while { class.notNil } {
				class.methods.do { |method|
					// methods include operators like "+", but those are
					// actually not valid in the method call syntax
					if (method.name.asString[0].isAlpha &&
						methods[method.name].isNil)
					{
						methods.put(method.name, method);
					};
				};
				class = class.superclass;
			};
			res = methods.values.collect { |m| this.serializeMethod(m) };
			if (res.size > 0) { this.prSend(id, res) };
		}
	}

	*completeMethod { |id, text|
		var res = [];
		Class.allClasses.do { |class|
			class.methods.do { |method|
				var signature;
				var definition;
				if (method.name.asString.beginsWith(text)) {
					res = res.add( this.serializeMethod(method) );
				};
			};
		};
		if (res.size > 0) { this.prSend(id, res) };
	}

	*findMethod { |id, text|
		var cname, mname, tokens, res;
		var class, method;

		tokens = text.split($.);
		if (tokens.size > 1) {
			cname = tokens[0];
			mname = tokens[1];
		}{
			mname = tokens[0];
		};
		if (mname.size < 1) { ^this };

		if (cname.size > 0) {
			class = cname.asSymbol.asClass;
			if (class.isNil) {
				warn("No class named" + cname.asString);
				^this;
			};
			method = class.class.findRespondingMethodFor(mname.asSymbol);
			if (method.isNil) {
				warn("No such method:" + cname.asString ++ "." ++ mname.asString);
				^this;
			};
			this.prSend(id, [this.serializeMethod(method)]);
		}{
			res = [];
			this.allMethodsDo { |method|
				if (method.name.asString == mname) {
					res = res.add( this.serializeMethod(method) );
				};
			};
			if (res.size > 0) {
				this.prSend(id, res)
			}{
				warn("No such method:" + mname.asString);
				^this;
			};
		}
	}

	*serializeMethod { arg method;
		var data = [method.ownerClass.name, method.name];
		if (method.argNames.size > 1) {
			data = data ++ [
				method.argNames.as(Array),
				method.prototypeFrame.collect { |val| val !? val.cs }
			].lace [2..];
		};
		^data;
	}

	*serializeMethodDetailed { arg method;
		var args, data;
		args = [];
		if (method.argNames.size > 1) {
			args = args ++ [
				method.argNames.as(Array),
				method.prototypeFrame.collect { |val|
					val !? {
						if (val.class === Float) { val.asString } { val.cs }
					}
				};
			].lace [2..];
		};
		data = [
			method.ownerClass.name,
			method.name,
			method.filenameSymbol,
			method.charPos,
			args
		];
		^data;
	}

	*allMethodsDo { arg func;
		Class.allClasses.do { |class|
			class.methods.do { |method|
				func.value(method);
			};
		};
	}

	*findReferencesToSymbol {|requestId, symbol|
		var methods;
		var result = SortedList(8, subListSorter);
		var references = Class.findAllReferences(symbol.asSymbol);

		if (references.notNil) {
			methods = IdentitySet.new;
			references.do { | funcDef |
				var homeContext;
				homeContext = if(funcDef.context.isNil) {funcDef} {funcDef.context.homeContext};
				if (homeContext.isKindOf(Method)) {
					methods.add(homeContext);
				};
			};
			methods.do { | method |
				result.add([
					method.ownerClass.name,
					method.name,
					method.filenameSymbol.asString,
					method.charPos + 1
				])
			}
		};

		ScIDE.prSend(requestId, [symbol, result.asArray])
	}

	*openHelpUrl { |url|
		ScIDE.processUrl(url, { |processedUrl|
			this.prSend("openHelpUrl", processedUrl.asString)
		});
	}

	*cmdPeriod { docRoutine.play(AppClock) }

	*processUrl { |urlString, doneAction|
		// NOTE: Copied and modified from HelpBrower:-goTo
		var url, brokenFunc;

		brokenFunc = { |fragment|
			var brokenUrl = URI.fromLocalPath( SCDoc.helpTargetDir++"/BrokenLink.html" );
			brokenUrl.fragment = fragment;
			brokenUrl;
		};

		url = URI(urlString);

		if (docRoutine.notNil) { docRoutine.stop };
		docRoutine = Routine {
			try {
				url = SCDoc.prepareHelpForURL(url) ?? { brokenFunc.(urlString) };
				doneAction.value(url);
			} {|err|
				err.throw;
			};
			CmdPeriod.remove(this);
			docRoutine = nil;
		}.play(AppClock);
		CmdPeriod.add(this);
	}

	// Document Support //////////////////////////////////////////////////

	*newDocument {|title="Untitled", string="", id|
		this.prSend(\newDocument, [title, string, id]);
	}

	*getQUuid {
		_ScIDE_GetQUuid
		this.primitiveFailed
	}

	*getTextByQUuid {|quuid, funcID, start = 0, range = -1|
		this.prSend(\getDocumentText, [quuid, funcID, start, range]);
	}

	*setTextByQUuid {|quuid, funcID, text, start = 0, range = -1|
		this.prSend(\setDocumentText, [quuid, funcID, text, start, range]);
	}

	// PRIVATE ///////////////////////////////////////////////////////////

	*prSend {|id, data|
		_ScIDE_Send
		this.primitiveFailed
	}

	*prConnect {|ideName|
		_ScIDE_Connect
		this.primitiveFailed
	}
}

// This is just a stub to provide oft-used functionality such as Document.open()
ScIDEDocument : Document {
	classvar <asyncActions;
	var <quuid, <title, <text;
	var <>textChangedAction;
	*initClass{
		Document.implementationClass = this;
		asyncActions = IdentityDictionary.new;
	}
	*new {|title = "untitled", string = "", makeListener, envir|
		var quuid = ScIDE.getQUuid, doc;
		ScIDE.newDocument(title, string, quuid);
		doc = super.prBasicNew.init(quuid, title, string);
		allDocuments = allDocuments.add(doc);
		^doc
	}

	*syncFromIDE {|quuid, title, chars|
		var doc;
		chars = chars.asAscii;
		if((doc = this.findByQUuid(quuid)).isNil, {
			doc = super.prBasicNew.init(quuid, title, chars);
			allDocuments = allDocuments.add(doc);
		}, {doc.init(quuid, title, chars)});
	}

	*syncDocs {|docInfo| // [quuid, title, string]
		docInfo.do({|info| this.syncFromIDE(*info) });
	}

	*executeAsyncResponse {|funcID ...args|
		var func;
		func = asyncActions[funcID];
		asyncActions[funcID] = nil;
		func.value(*args);
	}

	*findByQUuid {|quuid|
		^allDocuments.detect({|doc| doc.quuid == quuid });
	}

	*setActiveDocByQUuid {|quuid|
		var newCurrent, current;
		newCurrent = this.findByQUuid(quuid);
		current = this.current;
		if((newCurrent === current).not, {
			if(current.notNil, {current.didResignKey});
			newCurrent.didBecomeKey;
		});
	}

	init {|id, argtitle, argstring|
		quuid = id;
		title = argtitle;
		text = argstring;
	}

	initText {|string| text = string }

	updateText {|index, numCharsRemoved, addedChars|
		addedChars = addedChars.asAscii;
		text = text.keep(index) ++ addedChars ++ text.drop(index + numCharsRemoved);
		textChangedAction.value(this, index, numCharsRemoved, addedChars);
	}

	propen {|path, selectionStart, selectionLength, envir|
		if(envir != nil){"ScIDE does not set an environment per document".warn};
		^ScIDE.open(path, selectionStart, selectionLength)
	}

	// asynchronous get
	// range -1 means to the end of the Document
	getText {|action, start = 0, range -1|
		var funcID;
		funcID = ScIDE.getQUuid; // a unique id for this function
		asyncActions[funcID] = action; // pass the text
		ScIDE.getTextByQUuid(quuid, funcID, start, range);
	}

	// asynchronous set
	prSetText {|text, action, start = 0, range -1|
		var funcID;
		funcID = ScIDE.getQUuid; // a unique id for this function
		asyncActions[funcID] = action; // pass the text
		ScIDE.setTextByQUuid(quuid, funcID, text, start, range);
	}

	text_ {|string|
		text = string;
		this.prSetText(text);
	}

	insertText {|string, index = 0|
		text = text.insert(index, string);
		this.prSetText(string, nil, index, 0);
	}

	getChar {|index = 0|
		^text[index];
	}

	setChar {|char, index = 0|
		text = text.keep(index) ++ char ++ text.drop(index + 1);
		this.prSetText(char.asString, nil, index, 1);
	}

	== { |that| ^(this.quuid === that.quuid);}
}

