/*
   vstudio.es -- Support functions for generating VS projects
        
   Exporting: vstudio()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
    
require ejs.unix
    
var out: Stream

const TOOLS_VERSION = '4.0'
const PROJECT_FILE_VERSION = 10.0.30319.1
const SOL_VERSION = '11.00'
const XID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
const PREP = 'if not exist ${OUTDIR}\\obj md ${OUTDIR}\\obj\r
if not exist ${OUTDIR}\\bin md ${OUTDIR}\\bin\r
if not exist ${OUTDIR}\\inc md ${OUTDIR}\\inc\r
if not exist ${OUTDIR}\\inc\\bit.h copy ..\\${settings.product}-${platform.os}-bit.h ${OUTDIR}\\inc\\bit.h\r
if not exist ${OUTDIR}\\bin\\libmpr.def xcopy /Y /S *.def ${OUTDIR}\\bin\r
'
var prepTarget
var Base 

public function vstudio(base: Path) {
    //  MOB refactor
    Base = base
    bit.TOOLS_VERSION = TOOLS_VERSION
    bit.PROJECT_FILE_VERSION = PROJECT_FILE_VERSION
    for each (n in ['WIN_CFG', 'WIN_BIN', 'WIN_BITS', 'WIN_FLAT', 'WIN_INC', 'WIN_LIB', 'WIN_OBJ', 'WIN_PACKS', 
            'WIN_PKG', 'WIN_REL', 'WIN_SRC', 'WIN_TOP']) {
        bit[n] = wpath(bit[n].portable.relativeTo(base))
    }
    for each (n in ["BIN", "CFG", "FLAT", "INC", "LIB", "OBJ", "PACKS", "PKG", "REL", "SRC", "TOP"]) {
        bit[n] = bit[n].relativeTo(base)
    }
    let projects = []
    /* Create a temporary prep target as the first target */
    prepTarget = {
        type: 'vsprep',
        path: Path('always'),
        name: 'prep',
        enable: true,
        custom: PREP,
        includes: [], libraries: [], libpaths: [],
    }
    projBuild(projects, base, prepTarget)
    for each (target in bit.targets) {
        projBuild(projects, base, target)
    }
    propBuild(base)
    solBuild(projects, base)
}

function solBuild(projects, base: Path) {
    let path = base.joinExt('sln').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))
    output('Microsoft Visual Studio Solution File, Format Version ' + SOL_VERSION)
    output('# Visual Studio 2010')

    for each (target in projects) {
        target.guid = target.guid.toUpper()
        output('Project("' + XID + '") = "' + target.name + '", "' + 
            wpath(base.basename.join(target.name).joinExt('vcxproj', true)) + '", "{' + target.guid + '}"')
        /* Everything depends on prep */
        if (target != prepTarget) {
            let dep = prepTarget
            dep.guid = dep.guid.toUpper()
            output('\tProjectSection(ProjectDependencies) = postProject')
            output('\t\t{' + dep.guid + '} = {' + dep.guid + '}')
            output('\tEndProjectSection')
        }
        for each (dname in target.depends) {
            let dep = bit.targets[dname]
            if (!dep || !dep.guid) {
                continue
            }
            dep.guid = dep.guid.toUpper()
            output('\tProjectSection(ProjectDependencies) = postProject')
            output('\t\t{' + dep.guid + '} = {' + dep.guid + '}')
            output('\tEndProjectSection')
        }
        output('EndProject')
    }
    output('
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Debug|x64 = Debug|x64
		Release|Win32 = Release|Win32
		Release|x64 = Release|x64
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution')

    for each (target in projects) {
		output('{' + target.guid + '}.Debug|Win32.ActiveCfg = Debug|Win32')
		output('{' + target.guid + '}.Debug|Win32.Build.0 = Debug|Win32')
		output('{' + target.guid + '}.Debug|x64.ActiveCfg = Debug|x64')
		output('{' + target.guid + '}.Debug|x64.Build.0 = Debug|x64')
		output('{' + target.guid + '}.Release|Win32.ActiveCfg = Release|Win32')
		output('{' + target.guid + '}.Release|Win32.Build.0 = Release|Win32')
		output('{' + target.guid + '}.Release|x64.ActiveCfg = Release|x64')
		output('{' + target.guid + '}.Release|x64.Build.0 = Release|x64')
    }
	output('EndGlobalSection

  GlobalSection(SolutionProperties) = preSolution
    HideSolutionNode = FALSE
  EndGlobalSection
EndGlobal')
    out.close()
}

function propBuild(base: Path) {
    productPropBuild(base)
    debugPropBuild(base)
    releasePropBuild(base)
    archPropBuild(base, 'x86')
    archPropBuild(base, 'x64')
}

function productPropBuild(base: Path) {
    let path = base.join('debug.props').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))
    output('<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>$(IncDir);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>WIN32;_WINDOWS;_REENTRANT;_MT;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalLibraryDirectories>$(OutDir);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
    </Link>
  </ItemDefinitionGroup>
</Project>')
    out.close()
}

function debugPropBuild(base: Path) {
    let path = base.join('debug.props').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))

    //  MOB - remove MultiThreadedDebugDll
    output('<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros">
    <Config>debug</Config>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <PreprocessorDefinitions>_DEBUG;BLD_DEBUG;DEBUG_IDE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Optimization>Disabled</Optimization>
      <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
      <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <BuildMacro Include="Config">
    <Value>$(Config)</Value>
    <EnvironmentVariable>true</EnvironmentVariable>
  </BuildMacro>
  </ItemGroup>
</Project>')
    out.close()
}

function releasePropBuild(base: Path) {
    let path = base.join('release.props').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))
    //  MOB - remove MultiThreadedDebugDll
    output('<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros">
    <Config>release</Config>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <Optimization>MinSpace</Optimization>
      <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <FunctionLevelLinking>true</FunctionLevelLinking>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>false</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <BuildMacro Include="Config">
      <Value>$(Config)</Value>
      <EnvironmentVariable>true</EnvironmentVariable>
    </BuildMacro>
  </ItemGroup>
</Project>')
    out.close()
}

function archPropBuild(base: Path, arch) {
    let path = base.join(arch + '.props').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))

    output('<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros">
    <CfgDir>..\\..\\win-' + arch + '-$(Config)</CfgDir>
    <BinDir>$(CfgDir)\\bin</BinDir>
    <IncDir>$(CfgDir)\\inc</IncDir>
    <ObjDir>$(CfgDir)\\obj</ObjDir>
  </PropertyGroup>
  <ItemGroup>
    <BuildMacro Include="CfgDir">
      <Value>$(CfgDir)</Value>
      <EnvironmentVariable>true</EnvironmentVariable>
    </BuildMacro>
    <BuildMacro Include="BinDir">
      <Value>$(BinDir)</Value>
      <EnvironmentVariable>true</EnvironmentVariable>
    </BuildMacro>
    <BuildMacro Include="IncDir">
      <Value>$(IncDir)</Value>
      <EnvironmentVariable>true</EnvironmentVariable>
    </BuildMacro>
    <BuildMacro Include="ObjDir">
      <Value>$(ObjDir)</Value>
      <EnvironmentVariable>true</EnvironmentVariable>
    </BuildMacro>
  </ItemGroup>
</Project>')
/* UNUSED
  <PropertyGroup Label="Features">
    <Abs_CfgDir>$([System.IO.Path]::GetFullPath($(CfgDir))</Abs_CfgDir>
  </PropertyGroup>
 */
    out.close()
}

function projBuild(projects: Array, base: Path, target) {
    if (target.vsbuilt || !target.enable || target.nogen) {
        return
    }
    if (target.type != 'exe' && target.type != 'lib' && target.type != 'vsprep') {
        if (!(target.type == 'build')) {
            return
        }
    }
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (dep && dep.enable && !dep.vsbuilt) {
            projBuild(projects, base, dep)
        }
    }
    target.project = base.join(target.name).joinExt('vcxproj', true).relative
    trace('Generate', target.project)
    projects.push(target)
    out = TextStream(File(target.project, 'wt'))
    projHeader(base, target)
    projConfig(base, target)
    projSources(base, target)
    projSourceHeaders(base, target)
    projLink(base, target)
    projDeps(base, target)
    projFooter(base, target)
    out.close()
    target.vsbuilt = true
}

function projHeader(base, target) {
    bit.SUBSYSTEM = (target.rule == 'gui') ? 'Windows' : 'Console'
    bit.INC = target.includes ? target.includes.map(function(path) wpath(path.relativeTo(base))).join(';') : ''

    output('<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="${TOOLS_VERSION}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />

  <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'" Label="PropertySheets">
    <Import Project="product.props" />
    <Import Project="debug.props" />
    <Import Project="x86.props" />
  </ImportGroup>
  <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|Win32\'" Label="PropertySheets">
    <Import Project="product.props" />
    <Import Project="release.props" />
    <Import Project="x86.props" />
  </ImportGroup>
  <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Debug|x64\'" Label="PropertySheets">
    <Import Project="product.props" />
    <Import Project="debug.props" />
    <Import Project="x64.props" />
  </ImportGroup>
  <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'" Label="PropertySheets">
    <Import Project="product.props" />
    <Import Project="release.props" />
    <Import Project="x64.props" />
  </ImportGroup>')

    if (target.type == 'lib' || target.type == 'exe') {
        output('
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>${INC};%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>')

    /* UNUSED
        //  MOB - remove MultiThreadedDebugDLL below
        if (bit.platform.profile == 'debug') {
            output('  <PreprocessorDefinitions>WIN32;_DEBUG;_WINDOWS;BLD_DEBUG;DEBUG_IDE;_REENTRANT;_MT;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>')
        } else {
            output('  <PreprocessorDefinitions>WIN32;_WINDOWS;_REENTRANT;_MT;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>')
    }
    */
        output('    </ClCompile>')

        output('    <Link>
      <AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalLibraryDirectories>$(OutDir);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <SubSystem>${SUBSYSTEM}</SubSystem>')

        /* UNUSED
        if (bit.platform.profile == 'debug') {
          output('      <GenerateDebugInformation>true</GenerateDebugInformation>')
        } else {
          output('      <GenerateDebugInformation>false</GenerateDebugInformation>')
        }
        */
        output('    </Link>
  </ItemDefinitionGroup>')
    }
  //MOB <ItemGroup />')
}

function projConfig(base, target) {
    bit.PTYPE = (target.type == 'exe') ? 'Application' : 'DynamicLibrary'
    let guid = bit.dir.proj.join('.' + target.name + '.guid')
    if (guid.exists) {
        target.guid = guid.readString().trim()
    } else {
        target.guid = Cmd('uuidgen').response.toLower().trim()
        guid.write(target.guid)
    }
    bit.GUID = target.guid
    bit.CTOK = '$(Configuration)'
    bit.PTOK = '$(Platform)'
    bit.STOK = '$(SolutionDir)'
    bit.OTOK = '$(OutDir)'
    bit.UTOK = '$(UserRootDir)'
    bit.VTOK = '$(VCTargetsPath)'
    bit.NAME = target.name
    bit.OUTDIR = wpath(bit.dir.cfg.relativeTo(base))

    output('
  <PropertyGroup Label="Globals">
    <ProjectGuid>{${GUID}}</ProjectGuid>
    <RootNamespace />
    <Keyword>Win32Proj</Keyword>
  </PropertyGroup>

  <Import Project="${VTOK}\Microsoft.Cpp.Default.props" />
  <Import Project="${VTOK}\Microsoft.Cpp.props" />')

    output('
  <ItemGroup Label="ProjectConfigurations">')
    for each (vtype in ['Win32', 'x64']) {
        for each (vcfg in ['Debug', 'Release']) {
            bit.VTYPE = vtype
            bit.VCFG = vcfg

            output('    <ProjectConfiguration Include="${VCFG}|${VTYPE}">
      <Configuration>${VCFG}</Configuration>
      <Platform>${VTYPE}</Platform>
    </ProjectConfiguration>')
        }
    }
    output('  </ItemGroup>
')

    for each (vtype in ['Win32', 'x64']) {
        for each (vcfg in ['Debug', 'Release']) {
            bit.VTYPE = vtype
            bit.VCFG = vcfg
            output('  <PropertyGroup Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'" Label="Configuration">
    <ConfigurationType>${PTYPE}</ConfigurationType>
    <CharacterSet>NotSet</CharacterSet>
  </PropertyGroup>')
        }
    }

    output('
  <PropertyGroup>
    <_ProjectFileVersion>${PROJECT_FILE_VERSION}</_ProjectFileVersion>')
    for each (vtype in ['Win32', 'x64']) {
        for each (vcfg in ['Debug', 'Release']) {
            bit.VTYPE = vtype
            bit.VCFG = vcfg
            output('
    <OutDir Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'">$(BinDir)\\</OutDir>
    <IntDir Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'">$(ObjDir)\\${NAME}\\</IntDir>
    <CustomBuildBeforeTargets Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'">PreBuildEvent</CustomBuildBeforeTargets>')
        }
    }
    output('  </PropertyGroup>')
}

function projSourceHeaders(base, target) {
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (!dep || dep.type != 'header') continue
        output('
  <ItemGroup>')
        output('    <ClInclude Include="' + wpath(dep.path) + '" />')
        output('  </ItemGroup>')
    }
}

function projSources(base, target) {
    if (target.sources) {
        output('  
  <ItemGroup>')
        for each (file in target.files) {
            let obj = bit.targets[file]
            if (obj) {
                for each (src in obj.files) {
                    let path = src.relativeTo(base)
                    output('    <ClCompile Include="' + wpath(path) + '" />')
                }
            }
        }
        output('  </ItemGroup>')
    }
}

function projResources(base, target) {
    if (target.resources) {
        output('<ItemGroup>')
        for each (resource in target.resources) {
            output('  <ResourceCompile Include="' + wpath(resource) + '" />')
        }
        output('</ItemGroup>')
    }
}

function projLink(base, target) {
    if (target.type == 'lib') {
        let def = base.join(target.path.basename.toString().replace(/dll$/, 'def'))
        let newdef = Path(target.path.toString().replace(/dll$/, 'def'))
        if (newdef.exists) {
            cp(newdef, def)
        }
        if (def.exists) {
            bit.DEF = wpath(def.relativeTo(base))
            output('
  <ItemDefinitionGroup>
    <Link>
      <ModuleDefinitionFile>${DEF}</ModuleDefinitionFile>
    </Link>
  </ItemDefinitionGroup>')
        } else {
            trace('Warn', 'Missing ' + def)
        }
    }
    bit.LIBS = target.libraries ? mapLibs(target.libraries - bit.defaults.libraries).join(';') : ''
    bit.LIBPATHS = target.libpaths ? target.libpaths.map(function(p) wpath(p)).join(';') : ''
    output('
  <ItemDefinitionGroup>
  <Link>
    <AdditionalDependencies>${LIBS};%(AdditionalDependencies)</AdditionalDependencies>
    <AdditionalLibraryDirectories>$(OutDir);${LIBPATHS};%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
  </Link>')
    projCustomBuildStep(base, target)
    output('  </ItemDefinitionGroup>')
}

/*
    Emit a custom build step for exporting headers and the prep build step
 */
function projCustomBuildStep(base, target) {
    let outfile
    if (target.path) {
        bit.OUT = outfile = wpath(target.path.relativeTo(base))
    } else {
        outfile = 'always'
    }
    if (target.home) {
        bit.WIN_HOME = wpath(target.home.relativeTo(base))
        bit.HOME = target.home.relativeTo(base)
    }
    let command = target.custom || ''
    if (target.depends) {
        command += exportHeaders(base, target)
    }
    if (target['generate-vs']) {
        command += target['generate-vs']
    } else if (target['generate-nmake']) {
        let ncmd = target['generate-nmake']
// print("NCMD", ncmd)
        ncmd = ncmd.replace(/^[ \t]*/mg, '').trim().replace(/^-md /m, 'md ').replace(/^-rd /m, 'rd ')
        command += ncmd
    }
    command = command.replace(/^[ \t]*/mg, '').trim()
    if (command != '') {
try {
        //  MOB UNUSED <Outputs Condition="\'${CTOK}|${PTOK}\'==\'Debug|${VTYPE}\'">' + wpath(outfile) + '</Outputs>

        output('
  <CustomBuildStep>
    <Command>' + command + '</Command>
    <Outputs>' + wpath(outfile) + '</Outputs>
  </CustomBuildStep>')
} catch (e) {
    print(e + '\n' + 'in ' + target.name + ' ' + cmd)
    throw e
}
    }
}

function exportHeaders(base, target) {
    let cmd = ''
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (!dep || dep.type != 'header') continue
        for each (file in dep.files) {
            /* Use the directory in the destination so Xcopy won't ask if file or directory */
            cmd += 'xcopy /Y /S /D ' + wpath(file.relativeTo(base)) + ' ' + 
                wpath(dep.path.relativeTo(base).parent) + '\r\n'
        }
    }
    return cmd
}

function projDeps(base, target) {
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (!dep) {
            if (bit.packs[dname] || dname == 'build') {
                continue
            }
            throw 'Missing dependency ' + dname + ' for target ' + target.name
        }
        if (dep.type != 'exe' && dep.type != 'lib') {
            continue
        }
        if (!dep.enable) {
            continue
        }
        if (!dep.guid) {
            throw 'Missing guid for ' + dname
        }
        bit.DEP = dname
        bit.GUID = dep.guid
        output('
<ItemGroup>
  <ProjectReference Include="${DEP}.vcxproj">
  <Project>${GUID}</Project>
  <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
  </ProjectReference>
</ItemGroup>')
    }
}

function projFooter(base, target) {
    output('\n  <Import Project="${VTOK}\Microsoft.Cpp.targets" />')
    output('  <ImportGroup Label="ExtensionTargets">\n  </ImportGroup>\n\n</Project>')
}

function output(line: String) {
    out.writeLine(line.expand(bit))
}

function wpath(path): Path {
    path = path.relative.name
    path = path.replace(bit.dir.inc.relativeTo(Base), '$(IncDir)')
    path = path.replace(bit.dir.obj.relativeTo(Base), '$(ObjDir)')
    path = path.replace(bit.dir.bin.relativeTo(Base), '$(BinDir)')
    return Path(path.toString().replace(/\//g, '\\'))
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
