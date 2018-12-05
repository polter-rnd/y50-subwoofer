Name:           y50-subwoofer
Version:        0.1.7
Release:        2%{?dist}
Summary:        Daemon for adjusting Lenovo Y50 subwoofer

License:        GPLv2+
URL:            https://github.com/polter-rnd/y50-subwoofer
Source0:        https://github.com/polter-rnd/y50-subwoofer/archive/v%{version}.zip

BuildRequires:  gcc
BuildRequires:  alsa-lib-devel
BuildRequires:  systemd-units


%description
Small daemon that automatically turns Lenovo Y50 subwoofer on/off
and sets it volume according to the master channel.


%prep
%autosetup


%build
make CFLAGS="%{optflags}"


%install
rm -rf $RPM_BUILD_ROOT
%{__install} -Dm 755 %{_builddir}/%{name}-%{version}/%{name} $RPM_BUILD_ROOT/%{_bindir}/%{name}
%{__install} -Dm 644 %{_builddir}/%{name}-%{version}/%{name}.service $RPM_BUILD_ROOT/%{_unitdir}/%{name}.service
%{__install} -Dm 644 %{_builddir}/%{name}-%{version}/%{name}-resume.service $RPM_BUILD_ROOT/%{_unitdir}/%{name}-resume.service


%post
/bin/systemctl enable %{name}.service
/bin/systemctl enable %{name}-resume.service
/bin/systemctl start %{name}.service


%preun
/bin/systemctl stop %{name}.service
/bin/systemctl disable %{name}-resume.service
/bin/systemctl disable %{name}.service


%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}
%{_unitdir}/%{name}.service
%{_unitdir}/%{name}-resume.service


%changelog
* Thu Dec 5 2018 Paul Artsishevsky <polter.rnd@gmail.com> 0.1.5-2
- Updated to Fedora 29

* Mon Jan 2 2017 Paul Artsishevsky <polter.rnd@gmail.com> 0.1.5-1
- Updated to Fedora 25
- Added resume service to restart y50-subwoofer on wake up

* Sun Jul 17 2016 Paul Artsishevsky <polter.rnd@gmail.com> 0.1.3-1
- Initial version of the package
