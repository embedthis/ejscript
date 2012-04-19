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
    aggregates(base)
    sources(base)
    proxies(base)
    files(base)
    frameworks(base)
    groups(base)
    targets(base)
    scripts(base)
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
    makeid('ID_Project')
    makeid('ID_ProjectConfigList')
    makeid('ID_ProjectDebug')
    makeid('ID_ProjectRelease')

    let targets = []
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type == 'exe' || target.type == 'lib') {
            targets.push(target.name)
        }
    }
    //  MOB - need "_"
    bit.targets.Products = { enable: true, type: 'xcode-products', xcode: true, name: 'Products', depends: targets }

/*
    name == 'All' || 'Prep' || 'Products'
    type == 'All' || 'Prep' || 'Products'
    type
        group - unused
        xcode - unused
        xcode-products

    MOB - review 
        type: xtarget, build
        xcode: true
        Names: All, Products, Prep
 */
    bit.targets.All = { enable: true, type: 'build', xcode: true, name: 'All' }
    bit.targets.Prep = { enable: true, type: 'build', xcode: true, name: 'Prep', 'generate-xcode': "
[ ! -x ${INC_DIR} ] && mkdir -p ${INC_DIR} ${OBJ_DIR} ${LIB_DIR} ${BIN_DIR}
[ ! -f ${INC_DIR}/buildConfig.h ] && cp mpr-macosx-buildConfig.h ${INC_DIR}/buildConfig.h
if ! diff ${INC_DIR}/buildConfig.h mpr-macosx-buildConfig.h >/dev/null ; then
cp mpr-macosx-buildConfig.h ${INC_DIR}/buildConfig.h
fi
        "
    }
    etargets = []
    bit.targets.All.depends = etargets

    for each (target in bit.targets) {
        if (!target.enable) continue
        let type = target.type
        if (type == 'lib' || type == 'exe' || type == 'build') {
            target.xtype = 'xtarget'
        }
        if (target.type == 'exe') {
            etargets.push(target.name)
        }
        target.depends ||= []
        if (target.name != 'Prep') {
            target.depends.push('Prep')
        }
    }
}

function term() {
    delete bit.targets.Products
    delete bit.targets.All
    delete bit.targets.Prep
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

function aggregates(base: Path) {
    output('\n/* Begin PBXAggregateTarget section */')
    let section = '\t\t${TID} /* ${TNAME} */ = {
			isa = PBXAggregateTarget;
			buildConfigurationList = ${BCL} /* Build configuration list for PBXAggregateTarget "${TNAME}" */;
			buildPhases = (
				${SID}
			);
			dependencies = ('
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'build') {
            continue
        }
        let bcl = makeid('ID_BuildConfigList:' + target.name)
        let tid = makeid('ID_NativeTarget:' + target.name)
        let sid = target.type == 'build' ? (makeid('ID_ShellScript:' + target.name) + ',') : ''
        output(section.expand(ids, eo).expand({TID: tid, BCL: bcl, TNAME: target.name, SID: sid}))

        let depSection = '\t\t\t\t${DID} /* PBXTargetDependency ${DNAME} */,'
        for each (item in target.depends) {
            dep = bit.targets[item]
            if (!dep) {
                continue
            }
            if (dep.type != 'exe' && dep.type != 'lib' && dep.type != 'build') {
                continue
            }
            let did = makeid('ID_TargetDependency:' + target.name + '-on-' + dep.name)
print(did, target.name, 'on', dep.name)
            output(depSection.expand({DID: did, DNAME: dep.name}))
        }
        let footer = '\t\t\t);
			name = ${TNAME};
			productName = ${TNAME};
		};'
        output(footer.expand({TNAME: target.name}))
    }
    output('/* End PBXAggregateTarget section */')
}

function sources(base: Path) {
    let section = '
/* Begin PBXBuildFile section */'

    output(section)
    section = '\t\t${BID} /* ${PATH} in Sources */ = {isa = PBXBuildFile; fileRef = ${REF} /* ${NAME} */; };'

    for each (target in bit.targets) {
        if (!target.enable) continue
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
        Must do this for Products
     */
    section = '\t\t${BID} /* ${PATH} for ${TNAME} */ = {isa = PBXBuildFile; fileRef = ${REF} /* ${NAME} */; };'
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'xcode-products') {
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
    let section = '\t\t${PID} /* PBXContainerItemProxy ${DNAME} from ${TNAME} */ = {
			isa = PBXContainerItemProxy;
			containerPortal = ${ID_Project} /* Project object */;
			proxyType = 1;
			remoteGlobalIDString = ${DID};
			remoteInfo = ${DNAME};
		};'
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'build') {
            continue
        }
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (!dep || (dep.type != 'lib' && dep.type != 'exe' && dep.type != 'build')) continue
            let pid = makeid('ID_TargetProxy:' + target.name + '-on-' + dep.name)
            let did = getmakeid('ID_NativeTarget:' + dep.name)
            output(section.expand(ids, eo).expand({PID: pid, DID: did, TNAME: target.name, DNAME: dep.name}))
        }
    }
    output('/* End PBXContainerItemProxy section */')
}

function files(base: Path) {
    output('\n/* Begin PBXFileReference section */')
    let lib = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.dylib"; includeInIndex = 0; path = ${PATH}; sourceTree = BUILT_PRODUCTS_DIR; };'
    let exe = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.executable"; includeInIndex = 0; path = ${PATH}; sourceTree = BUILT_PRODUCTS_DIR; };'
    let source = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.c; name = ${NAME}; path = ${PATH}; sourceTree = "<group>"; };'

    for each (target in bit.targets) {
        if (!target.enable) continue
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
    This is for frameworks and libraries used by targets. Also used by Products.
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
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'xcode-products') {
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
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'xcode-products') {
           continue
        } 
        let ref = makeid('ID_TargetGroup:' + target.name)
        let name = target.name != 'Products' ? (target.name) : target.name
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
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'xcode-products') {
           continue
        } 
        let gid = getid('ID_TargetGroup:' + target.name)
        let name = target.name != 'Products' ? (target.name) : target.name
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
        if (!target.enable) continue
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
                if (dep.type == 'exe' || dep.type == 'lib' || dep.type == 'build') {
                    deplist.push(dep)
                }
            }
        }
        let deps = deplist.map(function(dep) '\t\t\t\t' + 
            getmakeid('ID_TargetDependency:' + target.name + '-on-' + dep.name) + 
            ' /* ' + dep.name + ' */,').join('\n') + '\t\t\t\t'
        output(section.expand(bit, eo).expand(ids, eo).expand({
            TNAME: target.name, TID: tid, BCL: bcl, REF: ref, SID: sid, FID: fid, PTYPE: ptype, DEPS: deps,
        }))
    }
    output('/* End PBXNativeTarget section */')
}

function scripts(base) {
    output('\n/* Begin PBXShellScriptBuildPhase section */')
    let section = '\t\t${SID} /* ShellScript for ${TNAME} */ = {
			isa = PBXShellScriptBuildPhase;
			buildActionMask = 2147483647;
			files = (
			);
			inputPaths = (
${INPUTS}
			);
			outputPaths = (
${OUTPUTS}
			);
			runOnlyForDeploymentPostprocessing = 0;
			shellPath = ${SHELL};
			shellScript = ${CMD};
		};'
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'build') {
            continue
        }
        //  Inputs: target.files, target.depends
        let inputs = ''
        //  Outputs: target.path
        let outputs = ''
        let shell = '/bin/bash'
        let cmd = target['generate-xcode'] || target['generate-sh']
        if (!cmd) {
            if (target.scripts && target.scripts.build) {
                shell = '"/usr/bin/env ' + target.scripts.build[0].shell + '"'
                cmd = target.scripts.build[0].script
            } else {
                cmd = 'true'
            }
        }
        let sid = getid('ID_ShellScript:' + target.name)
        cmd = cmd.toJSON()
        output(section.expand({SID: sid, CMD: cmd, INPUTS: inputs, OUTPUTS: outputs, TNAME: target.name, SHELL: shell}))
    }
    output('/* End PBXShellScriptBuildPhase section */')
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
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'build' && target.name != 'All') {
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
        if (!target.enable) continue
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

/*
    There is a dependency entry for each dependency relationship
 */
function targetDependencies(base: Path) {
    output('\n/* Begin PBXTargetDependency section */')
    let section = '\t\t${TDID} /* PBXTargetDependency ${TNAME} depends on ${DNAME} */ = {
			isa = PBXTargetDependency;
			target = ${DID} /* ${DNAME} */;
			targetProxy = ${PID} /* PBXContainerItemProxy */;
		};'
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'build') {
           continue
        }
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (!dep || (dep.type != 'lib' && dep.type != 'exe' && dep.type != 'build')) continue
            let tdid = getmakeid('ID_TargetDependency:' + target.name + '-on-' + dep.name)
            let pid = getid('ID_TargetProxy:' + target.name + '-on-' + dep.name)
            let did = getid('ID_NativeTarget:' + dep.name)
            output(section.expand({TDID: tdid, DID: did, TNAME: target.name, DNAME: dep.name, PID: pid}))
        }
    }
    output('/* End PBXTargetDependency section */')
}

function prepareSettings(base, o, debug: Boolean) {
    let options = {}

    if (!o.linker) {
        return ''
    }
    let libs = []
    for each (lname in o.libraries) {
        dep = bit.targets['lib' + lname]
        if (dep && dep.type == 'lib') {
            continue
        }
        libs.push(lname)
    }
    let flags = o.linker.filter(function(e) e != '-g') + libs.map(function(lib) '-l' + lib)
    if (flags.length > 0) {
        options.linker = '\n\t\t\t\tOTHER_LDFLAGS = (\n' + 
            flags.map(function(f) '\t\t\t\t\t"' + f + '",').join('\n') + '\n\t\t\t\t);\n'
    }
    if (o.includes.length > 0) {
        options.includes = '\n\t\t\t\tHEADER_SEARCH_PATHS = (\n' + 
            o.includes.map(function(f) '\t\t\t\t\t"' + f.relativeTo(base) + '",').join('\n') + '\n\t\t\t\t);\n'
    }
    if (o.libpaths.length > 0) {
        options.libpaths = '\n\t\t\t\tLIBRARY_SEARCH_PATHS = (\n' + 
            o.libpaths.map(function(f) '\t\t\t\t\t"' + f.relativeTo(base) + '",').join('\n') + '\n\t\t\t\t);'
    }
    let defines = debug ? o.defines : o.defines.clone().removeElements('-DDEBUG')
    if (defines.length > 0) {
        options.defines = '\n\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (\n' + 
            defines.map(function(f) '\t\t\t\t\t"' + f.replace('-D', '') + '",').join('\n') + 
            '\n\t\t\t\t\t"$(inherited)",\n\t\t\t\t);'
    }
    let result = ''
    if (options.includes) result += options.includes
    if (options.defines) result += options.defines
    if (options.libpaths) result += options.libpaths
    if (options.linker) result += options.linker
    return result.trimStart('\n')
}

function projectConfigSection(base) {
    let common_settings = '
                /* Common Settings */
				ALWAYS_SEARCH_USER_PATHS = NO;
				ARCHS = "$(ARCHS_STANDARD_64_BIT)";
                CURRENT_PROJECT_VERSION = ${settings.version};
                DYLIB_COMPATIBILITY_VERSION = "$(CURRENT_PROJECT_VERSION)";
                DYLIB_CURRENT_VERSION = "$(CURRENT_PROJECT_VERSION)";
				GCC_C_LANGUAGE_STANDARD = gnu99;
				GCC_VERSION = com.apple.compilers.llvm.clang.1_0;
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_64_TO_32_BIT_CONVERSION = ${WARN_64_TO_32};
				GCC_WARN_UNINITIALIZED_AUTOS = ${WARN_UNUSED};
				GCC_WARN_UNUSED_VARIABLE = ${WARN_UNUSED};
                GCC_WARN_UNUSED_FUNCTION = ${WARN_UNUSED};
                GCC_WARN_UNUSED_LABEL = ${WARN_UNUSED};
                LD_DYLIB_INSTALL_NAME = "@rpath/$(EXECUTABLE_PATH)";
				MACOSX_DEPLOYMENT_TARGET = 10.7;
				SDKROOT = macosx;

                CFG_DIR = "${CFG_DIR}";
                BIN_DIR = "${BIN_DIR}";
                LIB_DIR = "${LIB_DIR}";
                INC_DIR = "${INC_DIR}";
                OBJ_DIR = "${OBJ_DIR}";
                SRC_DIR = "${SRC_DIR}";

                CONFIGURATION_TEMP_DIR = "$(OBJ_DIR)/tmp/$(CONFIGURATION)";
                CONFIGURATION_BUILD_DIR = "$(BIN_DIR)";
                INSTALL_PATH = "/usr/lib/${settings.product}";
                DSTROOT = "/tmp/${settings.product}.dst";
                OBJROOT = "$(OBJ_DIR)";
                SYMROOT = "$(BIN_DIR)";
    '

    let section = '
/* Begin XCBuildConfiguration section */
		${ID_ProjectDebug} /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
${COMMON_SETTINGS}
                /* Debug Settings */
				COPY_PHASE_STRIP = NO;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_SYMBOLS_PRIVATE_EXTERN = NO;
${DEBUG_SETTINGS}
			};
			name = Debug;
		};
		${ID_ProjectRelease} /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
${COMMON_SETTINGS}
                /* Release Settings */
				COPY_PHASE_STRIP = YES;
				DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
${RELEASE_SETTINGS}
			};
			name = Release;
		};'

    let WARN_UNUSED = defaults.compiler['-Wno-unused-result'] ? 'NO' : 'YES'
    let WARN_64_TO_32 = defaults.compiler['-Wshorten-64-to-32'] ? 'NO' : 'YES'

    common_settings = common_settings.expand(bit, eo).expand(ids, eo).expand({
        CFG_DIR: bit.dir.cfg.relativeTo(base),
        BIN_DIR: bit.dir.bin.relativeTo(base),
        INC_DIR: bit.dir.inc.relativeTo(base),
        LIB_DIR: bit.dir.lib.relativeTo(base),
        OBJ_DIR: bit.dir.obj.relativeTo(base),
        SRC_DIR: bit.dir.src.relativeTo(base),
        WARN_UNUSED: WARN_UNUSED,
        WARN_64_TO_32: WARN_64_TO_32,
    })
    let debug_settings = prepareSettings(base, bit.defaults, true)
    let release_settings = prepareSettings(base, bit.defaults, false)

    output(section.expand(ids, eo).expand({
        COMMON_SETTINGS: common_settings,
        DEBUG_SETTINGS: debug_settings,
        RELEASE_SETTINGS: release_settings,
    }))

    section = '\t\t${TARGET_DEBUG} /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = ${TNAME};
${DEBUG_SETTINGS}
			};
			name = Debug;
		};
		${TARGET_RELEASE} /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = ${TNAME};
${RELEASE_SETTINGS}
			};
			name = Release;
		};'
    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'build' && target.name != 'All') {
           continue
        }
        let tdid = makeid('ID_TargetDebugConfig:' + target.name)
        let trid = makeid('ID_TargetReleaseConfig:' + target.name)

        let targetSettings = {
            compiler: target.compiler - bit.defaults.compiler,
            defines: target.defines - bit.defaults.defines,
            includes: target.includes - bit.defaults.includes,
            libpaths: target.libpaths - bit.defaults.libpaths,
            linker: target.linker - bit.defaults.linker,
            libraries: target.libraries - bit.defaults.libraries,
        }
        debug_settings = prepareSettings(base, targetSettings, true)
        release_settings = prepareSettings(base, targetSettings, false)
        output(section.expand({TNAME: target.name, TARGET_DEBUG: tdid, TARGET_RELEASE: trid, 
            DEBUG_SETTINGS: debug_settings, RELEASE_SETTINGS: release_settings}))
    }
    output('/* End XCBuildConfiguration section */')
}

function targetConfigSection() {
    let section = '
/* Begin XCConfigurationList section */
		${ID_ProjectConfigList} /* Build configuration list for PBXProject "${settings.product}" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${ID_ProjectDebug} /* Debug */,
				${ID_ProjectRelease} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'
    output(section.expand(bit, eo).expand(ids))

    section = '\t\t${BCL} /* Build configuration list for PBXNativeTarget "${TNAME}" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${TARGET_DEBUG} /* Debug */,
				${TARGET_RELEASE} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'

    for each (target in bit.targets) {
        if (!target.enable) continue
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'build' && target.name != 'All') {
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
