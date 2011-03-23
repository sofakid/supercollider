SCDoc {
    classvar <helpTargetDir;
    classvar <helpSourceDir;
    classvar <helpBaseDir;
    classvar doc_map = nil;
    classvar <p, <r;
    classvar doWait;
    classvar progressText = nil, progressWindow = nil;
    classvar progressCount = 0, progressMax = 1;
    classvar progressTopic = nil, progressBar = nil, closeButton = nil;
    classvar new_classes = nil;
    classvar didRun = false;

    *helpSourceDir_ {|path|
        helpSourceDir = path.standardizePath;
    }

    *helpBaseDir_ {|path|
        helpBaseDir = path.standardizePath;
    }

    *helpTargetDir_ {|path|
        if(path!=helpTargetDir) {didRun = false};
        helpTargetDir = path.standardizePath;
    }

    *waitForHelp {|func,gui=true|
        if(didRun.not)
            {this.updateAll(doneFunc:func,gui:gui)}
            {func.value};
    }

    *postProgress {|string,setTopic=false|
        var prg = (progressCount/progressMax*100).round(0.1).asString ++ "%";
        if(progressWindow.notNil) {
            if(setTopic, {
                progressTopic.string = string;
                progressText.string = prg;
            }, {
                progressText.string = prg+string;
            });
            progressBar.lo_(0).hi_(progressCount/progressMax);
        };
        (prg+string).postln;
        if(doWait, {0.wait});
    }

    *docMapToJSON {|path|
        var f;

        File.delete(path);
        f = File.open(path,"w");

        f.write("docmap = [\n");
        doc_map.pairsDo {|k,v|
            f.write("{\n");
            v.pairsDo {|k2,v2|
                if(v2.class == List) {
                    v2 = "["+v2.collect{|x|"'"++x++"'"}.join(",")+"]";
                } {
                    v2 = "'"++v2.asString.replace("'","\\'")++"'";
                };
                f.write("'"++k2++"': "++v2++",\n");
            };

            f.write("},\n");
        };
        f.write("]\n");
        f.close;
    }

    *splitList {|txt|
//        ^txt.findRegexp("[-_>#a-zA-Z0-9]+[-_>#/a-zA-Z0-9 ]*[-_>#/a-zA-Z0-9]+").flop[1];
        ^txt.findRegexp("[^, ]+[^,]*[^, ]+").flop[1];
    }

    *initClass {
        this.helpSourceDir_(thisProcess.platform.classLibraryDir.dirname +/+ "HelpSource");
        this.helpBaseDir_(thisProcess.platform.classLibraryDir.dirname +/+ "HelpBase");
        this.helpTargetDir_(thisProcess.platform.userAppSupportDir +/+ "Help");
        r = SCDocRenderer.new;
        r.parser = p = SCDocParser.new;
        doWait = false;
    }

    *makeMethodList {|c,n,tag|
        var l, mets, name, syms;

        (mets = c.methods) !? {
            n.add((tag:tag, children:l=List.new));
            syms = mets.collectAs(_.name,IdentitySet);
            mets.do {|m| //need to iterate over mets to keep the order
                name = m.name;
                if (name.isSetter.not or: {syms.includes(name.asGetter).not}) {
                    l.add((tag:\method, text:name.asString));
                };
            };
        };
    }

    *classHasArKrIr {|c|
        ^#[\ar,\kr,\ir].collect {|m| c.class.findRespondingMethodFor(m).notNil }.reduce {|a,b| a or: b};
    }
    
    *makeArgString {|m|
        var res = "";
        var value;
        var l = m.argNames;
        var last = l.size-1;
        l.do {|a,i|
            if (i>0) { //skip 'this' (first arg)
                if(i==last and: {m.varArgs}) {
                    res = res ++ " ... " ++ a;
                } {
                    if (i>1) { res = res ++ ", " };
                    res = res ++ a;
                    value = m.prototypeFrame[i];
                    if (value.notNil) {
                        value = switch(value.class,
                            Symbol, { "'"++value.asString++"'" },
                            Char, { "$"++value.asString },
                            String, { "\""++value.asString++"\"" },
                            { value.asString }
                        );
                        res = res ++ " = " ++ value.asString;
                    };
                };
            };
        };
        if (res.notEmpty) {
            ^("("++res++")");
        } {
            ^"";
        };
    }

    *handleUndocumentedClasses {
        var n, m, name, cats, dest;
        var destbase = helpTargetDir +/+ "Classes";
        this.postProgress("Generating docs for"+new_classes.size+"undocumented classes...",true);
        new_classes.do {|sym|
            var c = sym.asClass;
            var name = sym.asString;
            dest = destbase +/+ name ++ ".html";
            n = List.new;
            n.add((tag:\class, text:name));
            n.add((tag:\summary, text:""));

            cats = "Undocumented classes";
            if(this.classHasArKrIr(c)) {
                cats = cats ++ ", UGens>Undocumented";
            };
            if(c.categories.notNil) {
                cats = cats ++ ", "++c.categories.join(", ");
            };
            n.add((tag:\categories, text:cats));

            p.root = n;
            this.addToDocMap(p, "Classes" +/+ name);
            
            n.add((tag:\description, children:m=List.new));
            m.add((tag:\prose, text:"This class is missing documentation. Please create and edit HelpSource/Classes/"++name++".schelp", display:\block));

            // FIXME: Remove this when conversion to new help system is done!
            m.add((tag:\prose, text:"Old help file: ", display:\block));
            m.add((tag:\link, text:Help.findHelpFile(name) ?? "not found", display:\inline));

            this.makeMethodList(c.class,n,\classmethods);
            this.makeMethodList(c,n,\instancemethods);
            r.renderHTML(dest,"Classes");

            doc_map["Classes" +/+ name].delete = false;
            doc_map["Classes" +/+ name].methods = r.methods;
            this.tickProgress;
        };
    }
    
/*    *classHash {|c|
        var hash;
        //Problems: probably very slow! Also it doesn't work if one of the superclasses changed..
        hash = (((c.methods ++ c.class.methods) ?? []).collect {|m| m.code}).flatten.asString.hash;
        ^hash;
    }*/

    *addToDocMap {|parser, path|
        var c, x = parser.findNode(\class).text;
        var doc = (
            path:path,
            summary:parser.findNode(\summary).text,
            categories:parser.findNode(\categories).text
        );

        doc.title = if(x.notEmpty,x,{parser.findNode(\title).text});
        
        // check if class is standard, extension or missing
        if(x.notEmpty) {
            c = path.basename.asSymbol.asClass;
            doc.installed = if(c.notNil) {
                if(c.filenameSymbol.asString.beginsWith(thisProcess.platform.classLibraryDir).not)
                    {\extension}
                    {\standard}
            } {\missing};
        };

//        if(x.notEmpty) {
//            doc.hashnum = this.classHash(x.stripWhiteSpace.asSymbol.asClass);
//        };
        
        doc_map[path] = doc;
    }

    *makeCategoryMap {
        var cats, c, map;
        this.postProgress("Creating category map...");
        map = Dictionary.new;
        doc_map.pairsDo {|k,v|
            cats = this.splitList(v.categories);
            cats = cats ? ["Uncategorized"];
            cats.do {|cat|
                if (map[cat].isNil) {
                    map[cat] = List.new;
                };
                map[cat].add(v);
            };

        };
        ^map;
    }

    *readDocMap {
        var path = this.helpTargetDir +/+ "scdoc_cache";
        doc_map = path.load;

        if(doc_map.isNil) {
            doc_map = Dictionary.new;
            "SCDoc: docMap cache not found, created new".postln;
            ^true;
        } {
            "SCDoc: read docMap cache from file".postln;
        };
        ^false;
    }

    *writeDocMap {
        var f, path = this.helpTargetDir +/+ "scdoc_cache";
        File.delete(path);
        f = File.open(path,"w");
        f.write(doc_map.asCompileString);
        f.close;
        "SCDoc: wrote docMap cache".postln;
    }
    
    *docMap {
        if(doc_map.isNil) {
            this.readDocMap;
        };
        ^doc_map;
    }

    *makeProgressWindow {
        if(GUI.scheme.name != \QtGUI, {^nil});
        
        if(progressWindow.isNil) {
            progressWindow = Window("Documentation update",500@200).alwaysOnTop_(true).userCanClose_(false).layout_(QVLayout.new);
            progressWindow.onClose = {progressWindow = nil};
            StaticText(progressWindow).string_("Please wait while updating help files...");
            progressBar = RangeSlider(progressWindow,300@20).orientation_(\horizontal).background_(Color(0.8,0.8,0.8)).knobColor_(Color(0.5,0.5,0.8));
            progressTopic = StaticText(progressWindow).font_(Font.defaultSansFace.boldVariant);
            progressText = TextView(progressWindow).editable_(false);
            closeButton = Button(progressWindow).states_([["Close"]]).enabled_(false).action = {progressWindow.close; progressWindow = nil};
            progressWindow.front;
        };
    }

    *updateFile {|source, rootDir|
        var lastDot = source.findBackwards(".");
        var subtarget = source.copyRange(rootDir.size + 1, lastDot - 1);
        var target = helpTargetDir +/+ subtarget ++".html";
        var folder = target.dirname;
        var ext = source.copyToEnd(lastDot);
        if(source.beginsWith(rootDir).not) {
            error("File location error:\n"++source++"\nis not inside "++rootDir);
            ^nil;
        };
        if(ext == ".schelp", {
            this.postProgress("Parsing "++source);
            p.parseFile(source);
            this.addToDocMap(p,subtarget);
            r.renderHTML(target,subtarget.dirname);
            doc_map[subtarget].methods = r.methods;
            if(subtarget.dirname=="Classes") {
                if(new_classes.includes(subtarget.basename.asSymbol)) {
                    new_classes.remove(subtarget.basename.asSymbol);
                    progressMax = progressMax - 1;
                };
            };
        }, {
            this.postProgress("Copying" + source + "to" + (folder +/+ source.basename));
            ("mkdir -p"+folder.escapeChar($ )).systemCmd;
            ("cp" + source.escapeChar($ ) + folder.escapeChar($ )).systemCmd;
        });
    }
    
    *tickProgress { progressCount = progressCount + 1 }

    *initHelpTargetDir {
        var sysdir = helpBaseDir.escapeChar($ );
        var stamp = helpTargetDir.escapeChar($ )+/+"helpbase_timestamp";
        /*
        NOTE:
        if the system-wide HelpBase dir was updated since last time we based our help-dir on it,
        then copy it again. Any pre-rendered help that was updated (due to a new version of SC, etc)
        will thus be copied over and overwrite the outdated html file in helpTargetDir.

        Note that rsync does not check if sourcefile is newer, only if it differs.
        This means that scdoc_cache, classlist_cache, helpdirlist_cache will also be replaced,
        thus triggering a re-compile of all docs not part of HelpBase (main SC without extensions).
        This is probably a good thing, since a new SC version might mean that SCDoc changed internally
        and that all docs should be re-compiled.
        */
        if(File.exists(helpTargetDir).not or: {("test"+sysdir+"-nt"+stamp+"-o \\! -e"+stamp).systemCmd==0}) {
            this.postProgress("Initializing user's help directory", true);
            if(File.exists(helpBaseDir)) {
                this.postProgress("Basing help tree on pre-rendered help, please wait...");
                ("rsync -ax --link-dest="++sysdir+sysdir++"/"+helpTargetDir.escapeChar($ )+"2>/dev/null").systemCmd;
                this.postProgress("Done, creating timestamp");
                ("touch -r"+sysdir+stamp).systemCmd;
            } {
                this.postProgress("No pre-rendered help found, creating from scratch...");
            }
        };
    }

    *updateAll {|force=false,doneFunc=nil,threaded=true,gui=true,findExtensions=true,useHelpBase=true|
        var func;
        var docmap_path = helpTargetDir.escapeChar($ )+/+"scdoc_cache";
        var classlist_path = helpTargetDir+/+"classlist_cache";

        didRun = true;
        
        func = {
            var helpSourceDirs, fileList, count, maybeDelete, x, f, n, old_classes, current_classes;

            if(useHelpBase) {
                this.initHelpTargetDir;
            };

            if(force.not) {
                force = this.readDocMap;
            } {
                doc_map = Dictionary.new;
            };

            progressMax = 1;
            progressCount = 0;
            maybeDelete = false;
            
            new_classes = IdentitySet.new;

            this.postProgress("Searching for HelpSource folders...",true);

            // check for existence of main helpSourceDir
            if(File.exists(helpSourceDir).not) {
                progressCount = 1;
                this.postProgress(helpSourceDir+"does not exist!\n\nPlease set SCDoc.helpSourceDir to SCDoc's HelpSource folder and run SCDoc.updateAll again.");
                progressWindow !? {
                    progressText.stringColor = Color(1,0,0);
                    progressWindow.userCanClose = true;
                    closeButton.enabled = true;
                };
                ^this;
            };

            // find the set of helpSourceDirs
            helpSourceDirs = Set[helpSourceDir];
            if(findExtensions) {
                [thisProcess.platform.userExtensionDir, thisProcess.platform.systemExtensionDir].do {|dir|
                    helpSourceDirs = helpSourceDirs | ("find -L"+dir.escapeChar($ )+"-name 'HelpSource' -type d -prune")
                        .unixCmdGetStdOutLines.reject(_.isEmpty).asSet;
                };
            };
            this.postProgress(helpSourceDirs);

            this.postProgress("Ensuring help directory structure");
            helpSourceDirs.do {|srcdir|
                this.postProgress("Replicating"+srcdir);
                ("find"+srcdir.escapeChar($ )+"\\( ! -regex '.*/\\..*' \\) -type d").unixCmdGetStdOutLines.do {|dir|
                    x = (helpTargetDir+/+dir.copyToEnd(srcdir.size)).escapeChar($ );
                    this.postProgress("-"+x);
                    ("mkdir -p"+x).systemCmd;
                }
            };

            // get list of new or updated files
            fileList = Dictionary.new;
            count = 0;
            if(force) {
                this.postProgress("Updating all files",true);
                helpSourceDirs.do {|dir|
                    fileList[dir] = ("find -L"+dir.escapeChar($ )+"-type f")
                        .unixCmdGetStdOutLines.reject(_.isEmpty).asSet;
                    count = count + fileList[dir].size;
                };
            } {
                this.postProgress("Searching for new or updated files...",true);
                helpSourceDirs.do {|dir|
                    fileList[dir] = ("find -L"+dir.escapeChar($ )+"-newer"+docmap_path+"\\( ! -regex '.*/\\..*' \\) -type f")
                        .unixCmdGetStdOutLines.reject(_.isEmpty).asSet;
                    count = count + fileList[dir].size;
                };
            };
            this.postProgress("Found"+count+"files in need of update");

            //Read a list of all classes so that we can detect if any new ones where added (extensions).
            old_classes = Object.readArchive(classlist_path);
            current_classes = Class.allClasses.collectAs(_.name,IdentitySet).reject(_.isMetaClassName);
            this.postProgress("Checking for new classes...",true);
            if(old_classes.notNil) {
                current_classes.do{|name|
                    if(old_classes.includes(name).not) {
                        //or: {("test"+name.asClass.filenameSymbol.asString.escapeChar($ )+"-ot"+classlist_path).systemCmd!=0}) {
                        //tried to test if class source file is newer than the class_cache file to catch class file updates,
                        //but it is *very* slow and also won't trigger writing a new classlist_path file so that can't
                        //be used as time reference anyhow..
                        //Would be nice with a way to detect if a class did change though..
                        new_classes.add(name);
                    };
                };
            } {
                // this is a fresh run
                new_classes = current_classes.copy;
            };
            this.postProgress("Found"+new_classes.size+"new classes");

            // add all files in added HelpSource folders (e.g. newly installed quarks/extensions)
            x = Object.readArchive(helpTargetDir+/+"helpdirlist_cache");
            if(x.notNil) {
                (helpSourceDirs - x).do {|dir|
                    if(dir != helpSourceDir) {
                        this.postProgress("Found new HelpSource folder:"+dir);
                        fileList[dir] = fileList[dir] | ("find -L"+dir.escapeChar($ )+"\\( ! -regex '.*/\\..*' \\) -type f")
                            .unixCmdGetStdOutLines.reject(_.isEmpty).asSet;
                    };
                };
                if((x - helpSourceDirs).notEmpty) {
                    maybeDelete = true;
                };
            };
            File.delete(helpTargetDir+/+"helpdirlist_cache");
            helpSourceDirs.writeArchive(helpTargetDir+/+"helpdirlist_cache");

            count = 0;
            helpSourceDirs.do {|dir|
                count = count + fileList[dir].size;
            };
            this.postProgress("Total"+count+"files to process");
            progressMax = count + new_classes.size + 3;

            // parse/render or copy new and updated files
            // NOTE: any class docs processed here will remove that class from the new_classes set
            helpSourceDirs.do {|dir|
                x = fileList[dir].size;
                if(x>0) {
                    if(gui){this.makeProgressWindow};
                    this.postProgress("Processing"+x+"files in"+dir,true);
                    fileList[dir].do {|file|
                        this.updateFile(file,dir);
                        this.tickProgress;
                    };
                };
            };
            
            // use folder mtime to see if there might be any deleted files,
            // NOTE: this will also trigger on added helpfiles but that's ok I guess..
            if(force.not) {
                helpSourceDirs.do {|dir|
                    if(("find -L"+dir.escapeChar($ )+"-type d -newer"+docmap_path).unixCmdGetStdOut.isEmpty.not) {
                        maybeDelete = true;
                    };
                };
                if(maybeDelete or: force or: {old_classes != current_classes}) {
                    if(gui){this.makeProgressWindow};
                    this.postProgress("Help folders changed, scanning for deleted documents...",true);
                    doc_map.do(_.delete=true); // mark all docs in docMap for deletion
                    count = 0;
                    helpSourceDirs.do {|dir|
                        ("find -L"+dir.escapeChar($ )+"-name '*.schelp'").unixCmdGetStdOutLines.reject(_.isEmpty).do {|f|
                            var subtarget = f.copyRange(dir.size + 1, f.findBackwards(".") - 1);
                            doc_map[subtarget].delete = false;
                        };
                    };
                    current_classes.do {|sym|
                        x = doc_map["Classes"+/+sym.asString];
                        x !? {x.delete = false};
                    };

                    doc_map.pairsDo{|k,e|
                        if(e.delete==true, {
                            this.postProgress("Removing"+e.path+"from cache");
                            doc_map.removeAt(k);
                            count = count + 1;
                        });
                        e.removeAt(\delete); //remove the key since we don't need it anymore
                    };
                    this.postProgress("Removed"+count+"documents");
                };
            };

            // generate simple doc for each class in new_classes, which now contains only undocumented *new* classes:
            if(new_classes.notEmpty) {
                if(gui){this.makeProgressWindow};
                this.handleUndocumentedClasses;
            };
            
            if(old_classes != current_classes) {
                if(gui){this.makeProgressWindow};
                File.delete(classlist_path);
                current_classes.writeArchive(classlist_path);
                this.postProgress("Generating Class tree...",true);
                p.overviewClassTree;
                r.renderHTML(helpTargetDir +/+ "Overviews/ClassTree.html","Overviews",false);
                this.tickProgress;
            };

            this.writeDocMap;
            this.tickProgress;
            this.postProgress("Writing Document JSON index...",true);
            this.docMapToJSON(helpTargetDir +/+ "docmap.js");
            progressCount = progressMax;
            
            this.postProgress("Done!",true);
            "SCDoc done!".postln;
            doneFunc.value();
            doWait=false;
            progressWindow !? {
                progressWindow.userCanClose = true;
                closeButton.enabled = true;
            };
        };

        doWait = threaded or: gui;
//        if(gui, {this.makeProgressWindow});
        if(doWait, {
            Routine(func).play(AppClock);
        }, func);
        
    }

    *checkBrokenLinks {
        var f,m;
        var file;
        var check_link = {|link|
            if(("^[a-zA-Z]+://.+".matchRegexp(link) or: (link.first==$/)).not) {
                f = link.split($#);
                m = this.helpTargetDir +/+ f[0] ++ ".html";
                if((f[0]!="") and: {File.exists(m).not}) {
                    postln("Broken link: "++file++": "++link);
                };
            };
        };
        var do_children = {|children|
            children.do {|node|
                switch(node.tag,
                    \link, {
                        check_link.(node.text);
                    },
                    \related, {
                        this.splitList(node.text).do {|l|
                            check_link.(l);
                        };
                    },
                    {
                        node.children !? {
                            do_children.(node.children);
                        }
                    }
                );
            };
        };
        
        PathName(helpSourceDir).filesDo {|path|
            var source = path.fullPath;
            var lastDot = source.findBackwards(".");
            var ext = source.copyToEnd(lastDot);
            if(ext == ".schelp", {
                file = source;
                p.parseFile(source);
                do_children.(p.root);
            });
        };
        postln("Done");
    }
    
    *findClassOrMethod {|str|
        var path = if(str[0].isUpper) {
            if(str.asSymbol.asClass.notNil)
                {"Classes" +/+ str ++ ".html"}
                {"Search.html#" ++ str};
        } {"Overviews/Methods.html#" ++ str};
        ^ this.helpTargetDir +/+ path;
    }
}

+ String {
    stripWhiteSpace {
        var a=0, b=this.size-1;
        while({(this[a]==$\n) or: (this[a]==$\ ) or: (this[a]==$\t)},{a=a+1});
        while({(this[b]==$\n) or: (this[b]==$\ ) or: (this[b]==$\t)},{b=b-1});
        ^this.copyRange(a,b);
    }
	unixCmdGetStdOutLines {
		var pipe, lines, line;

		pipe = Pipe.new(this, "r");
		lines = Array.new;
		line = pipe.getLine;
		while({line.notNil}, {lines = lines.add(line); line = pipe.getLine; });
		pipe.close;

		^lines;
	}

}

+ Method {
    isExtensionOf {|class|
        ^(
            (this.filenameSymbol != class.filenameSymbol)
            and:
                if((class!=Object) and: (class!=Meta_Object),
                    {class.superclasses.includes(this.ownerClass).not},
                    {true})
        );
    }
}
