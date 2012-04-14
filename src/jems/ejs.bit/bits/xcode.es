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

    print('BASE', base)
    init(base)
    projHeader(base)
    // allTarget(base)
    sources(base)
    files(base)
    frameworks(base)
    groups(base)
    targets(base)
    project(base)
    sourcesBuildPhase(base)
    projectConfigSection()
    targetConfigSection()
    term()
dump(ids)
}

function init(base) {
    let name = base.basename
    let dir = base.joinExt('xcodeproj').relative
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
    
    makeid('ID_All')
    makeid('ID_Products')
    makeid('ID_Sources')
    makeid('ID_Frameworks')

    //  MOB - do we want debug & release?
    makeid('ID_Project')
    makeid('ID_ProjectConfigList')
    makeid('ID_ProjectDebug')
    makeid('ID_ProjectRelease')

    makeid('ID_TargetGroupProduct')

    let targets = []
    for each (target in bit.targets) {
        if (target.type == 'exe' || target.type == 'lib') {
            targets.push(target.name)
        }
    }
    bit.targets.ProductsGroup = {
        type: 'group',
        name: 'Products',
        depends: targets,
    }
}

function term() {
    delete bit.targets.ProductsGroup
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

function allTarget(base: Path) {
    let section = '    ${ID_All} /* All */ = {
        isa = PBXAggregateTarget;
        buildConfigurationList = AA0A1A0E0FD6DD14006A9E86 /* Build configuration list for PBXAggregateTarget "All" */;
        buildPhases = (
            /* MOB - replace with UID, for shell scripts */
        );
        dependencies = ('
    output(section.expand())

    section = '       ${TARGET_ID} /* PBXTargetDependency ' + target.name + ' */,'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
            continue
        }
        target.uid ||= uid()
        output(section.expand())
    }
    output('        );
    name = All;
    productName = All;
    };')
}

function sources(base: Path) {
    let section = '
/* Begin PBXBuildFile section */'

    output(section)
    section = '        ${BID} /* ${PATH} in Sources */ = { isa = PBXBuildFile; fileRef = ${REF} /* ${NAME} */; };'

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
                    output(section.expand({BID: bid, REF: ref, PATH: src, NAME: src}))
                }
            }
        }
    }
    output('/* End PBXBuildFile section */')
}

function files(base: Path) {
    output('\n/* Begin PBXFileReference section */')
    let lib = '        ${REF} /* ${NAME} */ = { isa = PBXBuildFileReference; explicitFileType = compiled.mach-o.dylib; includeInIndex = 0; path = ${PATH}; sourceTree = "<group>"; };'
    let exe = '        ${REF} /* ${NAME} */ = { isa = PBXBuildFileReference; explicitFileType = compiled.mach-o.executable; includeInIndex = 0; path = ${PATH}; sourceTree = "<group>"; };'
    let source = '        ${REF} /* ${NAME} */ = { isa = PBXBuildFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.c; name = ${NAME}; path = ${PATH}; sourceTree = "<group>"; };'

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
        let ref = makeid('ID_TargetRef:' + path)
        if (target.type == 'lib') {
            output(lib.expand({REF: ref, NAME: target.name, PATH: path}))
        } else if (target.type == 'exe') {
            //  MOB _ can use sourceTree = BUILT_PRODUCTS_DIR and then path is path.basename
            output(exe.expand({REF: ref, NAME: target.name, PATH: path}))
        }
    }
    output('/* End PBXFileReference section */')
}

function frameworks(base: Path) {
    let section = '
/* Begin PBXFrameworksBuildPhase section */
        ${ID_Frameworks} /* Frameworks */ = {
            isa = PBXFrameworksBuildPhase;
            buildActionMask = 2147483647;
            files = (
            );
            runOnlyForDeploymentPostprocessing = 0;
        };
/* End PBXFrameworksBuildPhase section */'
    output(section.expand(ids))
}


function groups(base: Path) {
    output('\n/* Begin PBXGroup section */')
    let section = '        ${GID} /* ${NAME} */ = {
            isa = PBXGroup;
            children = ('
    output(section.expand({GID: makeid('ID_Group'), NAME: 'Top'}))

    let groupItem = '                ${REF} /* ${NAME} */,'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'group') {
           continue
        } 
        let ref = makeid('ID_TargetGroup:' + target.name)
        output(groupItem.expand({REF: ref, NAME: target.name}))
    }
    output('            );
            sourceTree = "<group>";
        };')

    //  MOB - removed path = ${PATH};
    let groupFooter = '            );
            name = ${NAME};
            sourceTree = "<group>";
        };'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'group') {
           continue
        } 
        let gid = getid('ID_TargetGroup:' + target.name)
        output(section.expand({GID: gid, NAME: target.name}))
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (dep) {
                if (dep.type == 'obj') {
                    for each (src in dep.files) {
                        let ref = getid('ID_TargetSrc:' + src)
                        output(groupItem.expand({REF: ref, NAME: src}))
                    }
                } else {
                    let path = dep.path.relativeTo(base)
                    let ref = getid('ID_TargetRef:' + path)
                    output(groupItem.expand({REF: ref, NAME: dep.name}))
                }
            }
        }
        output(groupFooter.expand({NAME: target.name, PATH: target.path ? target.path.relativeTo(base) : '' }))
    }
    output('/* End PBXGroup section */')
}

function targets(base) {
    output('\n/* Begin PBXNativeTarget section */')
    let section = '        ${TID} /* ${TNAME} */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = ${BCL} /* Build configuration list for PBXNativeTarget "${TNAME}" */;
			buildPhases = (
				${ID_Sources} /* Sources */,
				${ID_Frameworks} /* Frameworks */,
			);
			buildRules = (
			);
			dependencies = (
			);
			name = ${TNAME};
			productName = ${settings.product};
			productReference = ${REF} /* ${TNAME} */;
			productType = "com.apple.product-type.tool";
		};
/* End PBXNativeTarget section */'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
           continue
        } 
        let tid = makeid('ID_NativeTarget:' + target.name)
        let bcl = makeid('ID_BuildConfigList:' + target.name)
        let path = target.path.relativeTo(base)
        let ref = getid('ID_TargetRef:' + path)
        output(section.expand(bit, eo).expand(ids, eo).expand({TNAME: target.name, TID: tid, BCL: bcl, REF: ref}))
    }
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

    section = '                ${TID} /* ${TNAME} */,'
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
           continue
        }
        let tid = getid('ID_NativeTarget:' + target.name)
        output(section.expand({TID: tid, TNAME: target.name}))
    }
    output('            );
		};
/* End PBXProject section */')
}

function sourcesBuildPhase(base: Path) {
    let section = '
/* Begin PBXSourcesBuildPhase section */
		${ID_Sources} /* Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
${FILES}
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXSourcesBuildPhase section */'

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
                let section = '                ${BID} /* ${NAME} in Sources */,'
                lines.push(section.expand({BID: bid, NAME: src}))
            }
        }
        let files = lines.join('\n')
        output(section.expand({FILES: files}, eo).expand(ids))
    }
}

function projectConfigSection() {
//  MOB - where should these come from - from bit.settings ?
    let section = '
/* Begin XCBuildConfiguration section */
		${ID_ProjectDebug} /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				ARCHS = "$(ARCHS_STANDARD_64_BIT)";
				COPY_PHASE_STRIP = NO;
				GCC_C_LANGUAGE_STANDARD = gnu99;
				GCC_DYNAMIC_NO_PIC = NO;
				GCC_ENABLE_OBJC_EXCEPTIONS = YES;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_PREPROCESSOR_DEFINITIONS = (
					"DEBUG=1",
					"$(inherited)",
				);
				GCC_SYMBOLS_PRIVATE_EXTERN = NO;
				GCC_VERSION = com.apple.compilers.llvm.clang.1_0;
				GCC_WARN_64_TO_32_BIT_CONVERSION = YES;
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNINITIALIZED_AUTOS = YES;
				GCC_WARN_UNUSED_VARIABLE = YES;
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
				GCC_ENABLE_OBJC_EXCEPTIONS = YES;
				GCC_VERSION = com.apple.compilers.llvm.clang.1_0;
				GCC_WARN_64_TO_32_BIT_CONVERSION = YES;
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNINITIALIZED_AUTOS = YES;
				GCC_WARN_UNUSED_VARIABLE = YES;
				MACOSX_DEPLOYMENT_TARGET = 10.7;
				SDKROOT = macosx;
			};
			name = Release;
		};'
    output(section.expand(ids))

    section = '        ${TARGET_DEBUG} /* Debug */ = {
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
        if (target.type != 'exe' && target.type != 'lib') {
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

    section = '        ${BCL} /* Build configuration list for PBXNativeTarget "demo" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${TARGET_DEBUG} /* Debug */,
				${TARGET_RELEASE} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'

    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
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
	rootObject = ${ID_Project} /* Project object */;'
    output(section.expand(ids))
}


function output(line: String) {
    out.writeLine(line)
}

function uid() 
    gbase + ("%08x" % [gnext++]).toUpper()

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

function remaekid(src) {
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
