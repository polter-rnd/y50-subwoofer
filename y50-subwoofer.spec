Name:           y50-subwoofer
Version:        0.1.3
Release:        1%{?dist}
Summary:        Daemon for adjusting Lenovo Y50 subwoofer

License:        GPLv2+
URL:            https://github.com/polter-rnd/y50-subwoofer
Source0:        https://github.com/polter-rnd/y50-subwoofer/archive/v%{version}.zip

BuildRequires:  alsa-lib-devel


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


%post
/bin/systemctl enable %{name}.service 
/bin/systemctl start %{name}.service


%preun
/bin/systemctl stop %{name}.service
/bin/systemctl disable %{name}.service


%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}
%{_unitdir}/%{name}.service


%changelog
* Sun Jul 17 2016 Pavel Artsishevsky <polter.rnd@gmail.com> 0.1.3-1
- Initial version of the package
