#
#	RPM spec file for ${settings.title}
#
Summary: ${settings.title} -- Embeddable JavaScript
Name: ${settings.product}
Version: ${settings.version}
Release: ${settings.buildNumber}
License: Dual GPL/commercial
Group: Development/Other
URL: http://ejscript.org
Distribution: Embedthis
Vendor: Embedthis Software
BuildRoot: ${dir.rpm}/BUILDROOT/${settings.product}-${settings.version}-${settings.buildNumber}.${platform.mappedCpu}
AutoReqProv: no

%description
Embedthis Ejscript is an enhanced, embeddable implementation of JavaScript

%prep

%build

%install
    mkdir -p ${dir.rpm}/BUILDROOT/${settings.product}-${settings.version}-${settings.buildNumber}.${platform.mappedCpu}
    cp -r ${dir.contents}/* ${dir.rpm}/BUILDROOT/${settings.product}-${settings.version}-${settings.buildNumber}.${platform.mappedCpu}

%clean

%files -f binFiles.txt

%post
if [ -x /usr/bin/chcon ] ; then 
	sestatus | grep enabled >/dev/null 2>&1
	if [ $? = 0 ] ; then
		for f in ${prefixes.vapp}/bin/*.so ; do
			chcon /usr/bin/chcon -t texrel_shlib_t $f
		done
	fi
fi
${prefixes.bin}/linkup Install /
ldconfig -n ${prefixes.vapp}/bin/bin/bin/bin

%preun
rm -f ${prefixes.product}/latest
${prefixes.bin}/linkup Remove /

%postun

