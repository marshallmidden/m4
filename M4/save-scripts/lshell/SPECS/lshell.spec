Name:           lshell
Version:        0.9.16.python3
Release:        6%{?dist}
Summary:        A Python-based limited shell

License:        GPLv3+
URL:            https://github.com/ghantoos/lshell
Source0:        http://downloads.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
Patch0:         lshell-parsec-help.patch
Patch1:         grp-work-around.patch
BuildArch:      noarch

BuildRequires:  python34-devel
BuildRequires:  python34-setuptools
Requires(pre):  shadow-utils

%description
lshell provides a limited shell configured per user. The configuration
is done quite simply using a configuration file.

%prep
%setup -q
%patch0 -p1
%patch1 -p1
#Fix permission
chmod -x CHANGES

%build
%define __python /usr/bin/python3
%{__python3} setup.py build

%install
%{__python3} setup.py install -O1 --skip-build --root=%{buildroot}
# Doc files at the wrong place
rm %{buildroot}%{_defaultdocdir}/lshell/{CHANGES,COPYING,README}

%pre
getent group lshell >/dev/null || groupadd -r lshell

%post
grep -q '^%{_bindir}/%{name}$' %{_sysconfdir}/shells || \
    echo '%{_bindir}/%{name}' >> %{_sysconfdir}/shells

%postun
if [ $1 -eq 0 ]; then
    sed -i '/^\/%{_bindir}\/%{name}$/d' %{_sysconfdir}/shells
fi

%files
%doc CHANGES COPYING README
%{_mandir}/man*/*.*
%{_bindir}/%{name}
%config(noreplace) %{_sysconfdir}/%{name}.conf
%config(noreplace) %{_sysconfdir}/logrotate.d/%{name}
%{python_sitelib}/lshell/
%{python_sitelib}/%{name}*.egg-info

%changelog

* Mon Aug 10 2020 Caleb Vaale
- Patch to get around db access issues

* Mon Jul 13 2020 Marshall Midden
- Patch to get around 'help help' bug (which starts a confusing circle -- why not python3). *gawk*

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.16-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Oct 29 2014 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.16-4
- Add to shells (rhbz#1111074)

* Wed Oct 29 2014 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.16-3
- Add group (rhbz#1109511)

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.16-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun Sep 08 2013 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.16-1
- Updated to new upstream version 0.9.16

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.15.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Wed Jun 26 2013 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.15.1-4
- Spec file updated

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.15.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.15.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Mar 19 2012 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.15.1-1
- Updated to new upstream version 0.9.15.1

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.14-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.14-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Wed Nov 10 2010 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.14-1
- Updated to new upstream version 0.9.14

* Sun Oct 17 2010 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.13-1
- Updated to new upstream version 0.9.13

* Fri Jul 30 2010 Thomas Spura <tomspur@fedoraproject.org> - 0.9.12-3
- Rebuilt for https://fedoraproject.org/wiki/Features/Python_2.7/MassRebuild

* Sat Jul 03 2010 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.12-2
- Removed setuptools
- Marked log file as config

* Sun Jun 06 2010 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.12-1
- Added logging support
- Updated macros
- Updated to new upstream version 0.9.12

* Mon Mar 15 2010 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.10-1
- Updated to new upstream version 0.9.10

* Sun Mar 07 2010 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.9-1
- Removed compression format from man page
- Updated to new upstream version 0.9.9

* Sun Dec 20 2009 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.8-1
- Updated to new upstream version 0.9.8

* Thu Nov 26 2009 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.7-1
- Updated to new upstream version 0.9.7

* Fri Aug 14 2009 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.5-1
- Updated to new upstream version

* Tue Jul 07 2009 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.4-1
- Updated to new upstream version 0.9.4

* Wed Apr 15 2009 Fabian Affolter <mail@fabian-affolter.ch> - 0.9.3-1
- Initial package for Fedora
