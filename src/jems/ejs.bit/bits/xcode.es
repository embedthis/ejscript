/*
   xcode.es -- Support functions for generating Xcode projects
        
   Exporting: xcode()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
    
require ejs.unix
    
var out: Stream

const ARCH_VERSION = '1'
const OBJ_VERSION = 46

/*
    Keys:
 */
var ids = {}
var eo = {fill: '${}'}
var gbase
var gnext = 0

//  MOB - move all templates out here

public function xcode(base: Path) {
    // bit.ARCH_VERSION = ARCH_VERSION
    // bit.OBJ_VERSION = OBJ_VERSION

    let name = base.basename
    base = base.dirname
    init(base, name)
    projHeader(base)
    allTargets(base)
    sources(base)
    proxies(base)
    files(base)
    frameworks(base)
    groups(base)
    targets(base)
    project(base)
    sourcesBuildPhase(base)
    targetDependencies(base)
    projectConfigSection(base)
    targetConfigSection()
    term()
    // dump(ids)
}

function init(base, name) {
    let dir = base.join(name).joinExt('xcodeproj').relative
    trace('Generate', dir)
    dir.makeDir()
    let proj = dir.join('project.pbxproj')
    out = TextStream(File(proj, 'wt'))

    let guidpath = dir.join('project.guid')
    if (guidpath.exists) {
        gbase = guidpath.readString().trim()
    } else {
        gbase = ('%08x%08x' % [Date().ticks, Date().ticks]).toUpper()
        guidpath.write(gbase)
    }
    
    makeid('ID_Products')

    //  MOB - do we want debug & release?
    makeid('ID_Project')
    makeid('ID_ProjectConfigList')
    makeid('ID_ProjectDebug')
    makeid('ID_ProjectRelease')
    /*
    makeid('ID_TargetGroupProduct')
    */

    let targets = []
    for each (target in bit.targets) {
        if (target.type == 'exe' || target.type == 'lib') {
            targets.push(target.name)
        }
    }
    //  MOB - should have "_" prefix and suffix
    bit.targets.ProductsGroup = { type: 'group', name: 'Products', depends: targets }

    targets = []
    for each (target in bit.targets) {
        if (target.type == 'exe') {
            targets.push(target.name)
        }
    }
    //  MOB - should have "_" prefix and suffix
    bit.targets.All = { type: 'all', name: 'All', depends: targets }
}

function term() {
    delete bit.targets.ProductsGroup
    delete bit.targets.All
}

function projHeader(base: Path) {
    output('// !$*UTF8*$!')
    output('{
    archiveVersion = 1;
    classes = {
    };
    objectVersion = 46;
    objects = {')
}

function allTargets(base: Path) {
    output('\n/* Begin PBXAggregateTarget section */')

    let section = '\t\t${TID} /* All */ = {
			isa = PBXAggregateTarget;
			buildConfigurationList = ${BCL} /* Build configuration list for PBXAggregateTarget "All" */;
			buildPhases = (
				/* MOB - replace with UID, for shell scripts */
			);
			dependencies = ('
    let bcl = makeid('ID_BuildConfigList:All')
    let tid = makeid('ID_NativeTarget:All')
    output(section.expand(ids, eo).expand({TID: tid, BCL: bcl}))

    section = '\t\t\t\t${DID} /* PBXTargetDependency ${TNAME} */,'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
            continue
        }
        let did = makeid('ID_TargetDependency:' + target.name)
        output(section.expand({DID: did, TNAME: target.name}))
    }
    output('\t\t\t);
			name = All;
			productName = All;
    	};
/* End PBXAggregateTarget section */')
}

function sources(base: Path) {
    let section = '
/* Begin PBXBuildFile section */'

    output(section)
    section = '\t\t${BID} /* ${PATH} in Sources */ = {isa = PBXBuildFile; fileRef = ${REF} /* ${NAME} */; };'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
            continue
        } 
        for each (file in target.files) {
            let obj = bit.targets[file]
            if (obj) {
                let bid = makeid('ID_BuildFile:' + obj.name)
                for each (src in obj.files) {
                    let ref = makeid('ID_TargetSrc:' + src)
                    output(section.expand({BID: bid, REF: ref, PATH: src.basename, NAME: src.basename}))
                }
            }
        }
    }
    /*
        Emit a framework reference for library targets that are depended upon. See PBXFrameworksBuildPhase.
     */
    section = '\t\t${BID} /* ${PATH} for ${TNAME} */ = {isa = PBXBuildFile; fileRef = ${REF} /* ${NAME} */; };'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'group') {
           continue
        } 
        for each (item in target.libraries) {
            let dep = bit.targets['lib' + item]
            if (dep && dep.type == 'lib') {
                let bid = makeid('ID_TargetFramework:' + target.name + '-on-' + dep.name)
                let path = dep.path.relativeTo(base)
                let ref = getmakeid('ID_TargetRef:' + path)
                output(section.expand({BID: bid, REF: ref, PATH: dep.path.basename, 
                    TNAME: target.name, NAME: dep.name}))
            }
        }
    }
    output('/* End PBXBuildFile section */')
}

function proxies(base: Path) {
    output('\n/* Begin PBXContainerItemProxy section */')
    let section = '\t\t${PID} /* PBXContainerItemProxy ${TNAME} */ = {
			isa = PBXContainerItemProxy;
			containerPortal = ${ID_Project} /* Project object */;
			proxyType = 1;
			remoteGlobalIDString = ${TID};
			remoteInfo = ${TNAME};
		};'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
            continue
        }
        let pid = makeid('ID_TargetProxy:' + target.name)
        let tid = makeid('ID_NativeTarget:' + target.name)
        output(section.expand(ids, eo).expand({PID: pid, TID: tid, TNAME: target.name}))
    }
    output('/* End PBXContainerItemProxy section */')
}

function files(base: Path) {
    output('\n/* Begin PBXFileReference section */')
    let lib = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.dylib"; includeInIndex = 0; path = ${PATH}; sourceTree = BUILT_PRODUCTS_DIR; };'
    let exe = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.executable"; includeInIndex = 0; path = ${PATH}; sourceTree = BUILT_PRODUCTS_DIR; };'
    let source = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.c; name = ${NAME}; path = ${PATH}; sourceTree = "<group>"; };'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
            continue
        } 
        for each (file in target.files) {
            let obj = bit.targets[file]
            /*
                Emit all sources
             */
            if (obj) {
                for each (src in obj.files) {
                    let path = src.relativeTo(base)
                    let ref = getid('ID_TargetSrc:' + src)
                    output(source.expand({REF: ref, NAME: src.basename, PATH: src}))
                }
            }
        }
        let path = target.path.relativeTo(base)
        let ref = getmakeid('ID_TargetRef:' + path)
        if (target.type == 'lib') {
            //  MOB - may need full path if not using built products
            output(lib.expand({REF: ref, NAME: target.name, PATH: path.basename.joinExt(bit.ext.shobj)}))
        } else if (target.type == 'exe') {
            //  MOB - may need full path if not using built products
            //  MOB _ can use sourceTree = BUILT_PRODUCTS_DIR and then path is path.basename
            output(exe.expand({REF: ref, NAME: target.name, PATH: path.basename.joinExt(bit.ext.exe)}))
        }
    }
    output('/* End PBXFileReference section */')
}

/*
    This is for frameworks and libraries used by targets
 */
function frameworks(base: Path) {
    output('\n/* Begin PBXFrameworksBuildPhase section */')
    let section = '\t\t${FID} /* Frameworks and Libraries for ${TNAME} */ = {
            isa = PBXFrameworksBuildPhase;
            buildActionMask = 2147483647;
            files = (
${LIBS}
            );
            runOnlyForDeploymentPostprocessing = 0;
        };'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'group') {
           continue
        } 
        let fid = makeid('ID_Frameworks:' + target.name)
        let libs = []
        for each (item in target.libraries) {
            let dep = bit.targets['lib' + item]
            if (dep && dep.type == 'lib') {
                let id = getid('ID_TargetFramework:' + target.name + '-on-' + dep.name)
                libs.push('\t\t\t\t' + id + ' /* ' + dep.name + ' */,')
            }
        }
        output(section.expand(ids, eo).expand({FID: fid, LIBS: libs.join('\n') + '\t\t\t\t', TNAME: target.name}))
    }
    output('/* End PBXFrameworksBuildPhase section */')
}


function groups(base: Path) {
    output('\n/* Begin PBXGroup section */')
    let section = '\t\t${GID} /* ${NAME} */ = {
            isa = PBXGroup;
            children = ('
    output(section.expand({GID: makeid('ID_Group'), NAME: 'Top'}))

    let groupItem = '\t\t\t\t${REF} /* ${NAME} */,'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'group') {
           continue
        } 
        let ref = makeid('ID_TargetGroup:' + target.name)
        let name = target.name != 'Products' ? (target.name + '-source') : target.name
        output(groupItem.expand({REF: ref, NAME: name}))
    }
    output('\t\t\t);
            sourceTree = "<group>";
        };')

    let groupFooter = '\t\t\t);
            name = "${NAME}";
            path = ${PATH};
            sourceTree = "<absolute>";
        };'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'group') {
           continue
        } 
        let gid = getid('ID_TargetGroup:' + target.name)
        let name = target.name != 'Products' ? (target.name + '-source') : target.name
        output(section.expand({GID: gid, NAME: name}))
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (dep) {
                if (dep.type == 'obj') {
                    for each (src in dep.files) {
                        let ref = getid('ID_TargetSrc:' + src)
                        output(groupItem.expand({REF: ref, NAME: src.basename}))
                    }
                } else if ((dep.type == 'exe' || dep.type == 'lib') && target.name == 'Products') {
                    let path = dep.path.relativeTo(base)
                    let ref = getid('ID_TargetRef:' + path)
                    output(groupItem.expand({REF: ref, NAME: dep.name}))
                }
            }
        }
        output(groupFooter.expand({NAME: name, PATH: bit.dir.src}))
    }
    output('/* End PBXGroup section */')
}

function targets(base) {
    output('\n/* Begin PBXNativeTarget section */')
    let section = '\t\t${TID} /* ${TNAME} */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = ${BCL} /* Build configuration list for PBXNativeTarget "${TNAME}" */;
			buildPhases = (
				${SID} /* Sources */,
				${FID} /* Frameworks */,
			);
			buildRules = (
			);
			dependencies = (
${DEPS}
			);
			name = ${TNAME};
			productName = ${settings.product};
			productReference = ${REF} /* ${TNAME} */;
			productType = "${PTYPE}";
		};'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
           continue
        } 
        let sid = makeid('ID_NativeSources:' + target.name)
        let fid = getid('ID_Frameworks:' + target.name)
        let tid = getid('ID_NativeTarget:' + target.name)
        let bcl = makeid('ID_BuildConfigList:' + target.name)
        let path = target.path.relativeTo(base)
        let ref = getid('ID_TargetRef:' + path)
        let ptype = (target.type == 'exe') ? 'com.apple.product-type.tool' : 'com.apple.product-type.library.dynamic';

        let deplist = []
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (dep) {
                if (dep.type == 'exe' || dep.type == 'lib') {
                    deplist.push(dep)
                }
            }
        }
        let deps = deplist.map(function(dep) '\t\t\t\t' + getmakeid('ID_TargetDependency:' + dep.name) + 
            + ' /* ' + dep.name + ' */,').join('\n') + '\t\t\t\t'
        output(section.expand(bit, eo).expand(ids, eo).expand({
            TNAME: target.name, TID: tid, BCL: bcl, REF: ref, SID: sid, FID: fid, PTYPE: ptype, DEPS: deps,
        }))
    }
    output('/* End PBXNativeTarget section */')
}

function project(base) {
    let section = '
/* Begin PBXProject section */
		${ID_Project} /* Project object */ = {
			isa = PBXProject;
			attributes = {
				LastUpgradeCheck = 0430;
				ORGANIZATIONNAME = "${settings.company}";
			};
			buildConfigurationList = ${ID_ProjectConfigList} /* Build configuration list for PBXProject "${settings.product}" */;
			compatibilityVersion = "Xcode 3.2";
			developmentRegion = English;
			hasScannedForEncodings = 0;
			knownRegions = (
				en,
			);
			mainGroup = ${ID_Group};
			productRefGroup = ${ID_TargetGroup:Products} /* Products */;
			projectDirPath = "";
			projectRoot = "";
			targets = ('

    output(section.expand(bit, eo).expand(ids))

    section = '\t\t\t\t${TID} /* ${TNAME} */,'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.name != 'All') {
           continue
        }
        let tid = getid('ID_NativeTarget:' + target.name)
        output(section.expand({TID: tid, TNAME: target.name}))
    }
    output('\t\t\t);
		};
/* End PBXProject section */')
}

function sourcesBuildPhase(base: Path) {
    output('\n/* Begin PBXSourcesBuildPhase section */')
    let section = '\t\t${SID} /* ${NAME} Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
${FILES}
			);
			runOnlyForDeploymentPostprocessing = 0;
		};'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
           continue
        }
        let lines = []
        for each (file in target.files) {
            let obj = bit.targets[file]
            for each (src in obj.files) {
                let bid = getid('ID_BuildFile:' + obj.name)
                let fid = getid('ID_TargetSrc:' + src)
                let srcSection = '\t\t\t\t${BID} /* ${NAME} in Sources */,'
                lines.push(srcSection.expand({BID: bid, NAME: src.basename}))
            }
        }
        let files = lines.join('\n')
        let sid = getid('ID_NativeSources:' + target.name)
        output(section.expand({FILES: files, NAME: target.name, SID: sid}, eo).expand(ids))
    }
    output('/* End PBXSourcesBuildPhase section */')
}

function targetDependencies(base: Path) {
    output('\n/* Begin PBXTargetDependency section */')
    let section = '\t\t${DID} /* PBXTargetDependency ${TNAME} */ = {
			isa = PBXTargetDependency;
			target = ${TID} /* ${TNAME} */;
			targetProxy = ${PID} /* PBXContainerItemProxy */;
		};'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
           continue
        }
        if (!ids['ID_TargetDependency:' + target.name]) {
            continue
        }
        let did = getid('ID_TargetDependency:' + target.name)
        let tid = getid('ID_NativeTarget:' + target.name)
        let pid = getid('ID_TargetProxy:' + target.name)
        output(section.expand({DID: did, TID: tid, TNAME: target.name, PID: pid}))
    }
    output('/* End PBXTargetDependency section */')
}

function projectConfigSection(base) {
    let section = '
/* Begin XCBuildConfiguration section */
		${ID_ProjectDebug} /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				ARCHS = "$(ARCHS_STANDARD_64_BIT)";
				COPY_PHASE_STRIP = NO;
				GCC_C_LANGUAGE_STANDARD = gnu99;
GCC_ENABLE_OBJC_EXCEPTIONS = YES;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_SYMBOLS_PRIVATE_EXTERN = NO;
				GCC_VERSION = com.apple.compilers.llvm.clang.1_0;
				GCC_WARN_64_TO_32_BIT_CONVERSION = ${WARN_64_TO_32};
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNINITIALIZED_AUTOS = YES;
				GCC_WARN_UNUSED_VARIABLE = ${WARN_UNUSED};
                GCC_DYNAMIC_NO_PIC = ${NO_PIC};
${SETTINGS}
${DEBUG_SETTINGS}
				MACOSX_DEPLOYMENT_TARGET = 10.7;
				ONLY_ACTIVE_ARCH = YES;
				SDKROOT = macosx;
			};
			name = Debug;
		};
		${ID_ProjectRelease} /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				ARCHS = "$(ARCHS_STANDARD_64_BIT)";
				COPY_PHASE_STRIP = YES;
				DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
				GCC_C_LANGUAGE_STANDARD = gnu99;
/* MOB - remove */
GCC_ENABLE_OBJC_EXCEPTIONS = YES;
				GCC_VERSION = com.apple.compilers.llvm.clang.1_0;
				GCC_WARN_64_TO_32_BIT_CONVERSION = ${WARN_64_TO_32};
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNINITIALIZED_AUTOS = YES;
				GCC_WARN_UNUSED_VARIABLE = ${WARN_UNUSED};
                GCC_DYNAMIC_NO_PIC = ${NO_PIC};
${SETTINGS}
${RELEASE_SETTINGS}
				MACOSX_DEPLOYMENT_TARGET = 10.7;
				SDKROOT = macosx;
                /* MOB OUT = "$(TOP)/out"; */
			};
			name = Release;
		};'
        let defaults = bit.defaults
        let flags = []
        for each (flag in defaults.linker) {
            if (flag == '-g') continue
            flags.push(flag)
        }
        for each (lib in defaults.libraries) {
            flags.push('-l' + lib)
        }
        let settings = '\t\t\t\tOTHER_LDFLAGS = (\n' + 
            flags.map(function(f) '\t\t\t\t\t"' + f + '",').join('\n') + '\n\t\t\t\t);\n'
        settings += '\t\t\t\tHEADER_SEARCH_PATHS = (\n' + 
            defaults.includes.map(function(f) '\t\t\t\t\t"' + f.relativeTo(base) + '",').join('\n') + '\n\t\t\t\t);\n'
        settings += '\t\t\t\tLIBRARY_SEARCH_PATHS = (\n' + 
            defaults.libpaths.map(function(f) '\t\t\t\t\t"' + f.relativeTo(base) + '",').join('\n') + '\n\t\t\t\t);'

        //  MOB - should define BLD_DEBUG and have buildConfig.h have debug/release sections

        let debug_settings = '\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (\n' + 
            defaults.defines.map(function(f) '\t\t\t\t\t"' + f.replace('-D', '') + '",').join('\n') + 
            '\n\t\t\t\t\t"$(inherited)",\n\t\t\t\t);'
        let release_settings = '\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (\n' +
            defaults.defines.map(function(f) '\t\t\t\t\t"' + f.replace('-D', '') + '",').join('\n') + 
            '\n\t\t\t\t\t"$(inherited)",\n\t\t\t\t);'

        NO_PIC = defaults.compiler.contains('-fPIC') ? 'NO' : 'YES'
        WARN_UNUSED = defaults.compiler['-Wno-unused-result'] ? 'NO' : 'YES'
        WARN_64_TO_32 = defaults.compiler['-Wshorten-64-to-32'] ? 'NO' : 'YES'

    output(section.expand(ids, eo).expand({
        NO_PIC: NO_PIC,
        WARN_UNUSED: WARN_UNUSED,
        WARN_64_TO_32: WARN_64_TO_32,
        SETTINGS: settings,
        DEBUG_SETTINGS: debug_settings,
        RELEASE_SETTINGS: release_settings,
    }))

    section = '\t\t${TARGET_DEBUG} /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = ${TNAME};
			};
			name = Debug;
		};
		${TARGET_RELEASE} /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = ${TNAME};
			};
			name = Release;
		};'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.name != 'All') {
           continue
        }
        let tdid = makeid('ID_TargetDebugConfig:' + target.name)
        let trid = makeid('ID_TargetReleaseConfig:' + target.name)
        output(section.expand({TNAME: target.name, TARGET_DEBUG: tdid, TARGET_RELEASE: trid}))
    }
    output('/* End XCBuildConfiguration section */')
}

function targetConfigSection() {
    let section = '
/* Begin XCConfigurationList section */
		${ID_ProjectConfigList} /* Build configuration list for PBXProject "test" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${ID_ProjectDebug} /* Debug */,
				${ID_ProjectRelease} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'
    output(section.expand(ids))

    section = '\t\t${BCL} /* Build configuration list for PBXNativeTarget "demo" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${TARGET_DEBUG} /* Debug */,
				${TARGET_RELEASE} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.name != 'All') {
           continue
        }
        let bcl = getid('ID_BuildConfigList:' + target.name)
        let tid = getid('ID_NativeTarget:' + target.name)
        let tdid = getid('ID_TargetDebugConfig:' + target.name)
        let trid = getid('ID_TargetReleaseConfig:' + target.name)
        output(section.expand(bit, eo).expand(ids, eo).expand({BCL: bcl, TNAME: target.name, TID: tid, 
            TARGET_DEBUG: tdid, TARGET_RELEASE: trid}))
    }

    section = '/* End XCConfigurationList section */
	};
	rootObject = ${ID_Project} /* Project object */;
}'
    output(section.expand(ids))
}


function output(line: String) {
    out.writeLine(line)
}

function uid()
    (gbase + ("%08x" % [gnext++]).toUpper())

function getid(src) {
    if (!ids[src]) {
        throw new Error('Unknown ID for ' + src)
    }
    return ids[src]
}

function makeid(src) {
    if (ids[src]) {
        throw new Error('ID already exists for  ' + src)
    }
    return ids[src] = uid()
}

function getmakeid(src) {
    let id = ids[src] || uid()
    return ids[src] = id
}

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
