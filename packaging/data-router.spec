Name:       data-router
Summary:    Data Router
Version:    0.2.17
Release:    1
Group:      Connectivity/Service
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	data-router.manifest
BuildRequires: cmake
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(vconf)
Requires(post): /usr/bin/vconftool

%description
Working as a Router for USB communication
For USB serial communication, reads/writes usb node and routes them to Socket client application.

%prep
%setup -q
cp %{SOURCE1001} .

%build
%cmake .
make %{?jobs:-j%jobs}

%install
%make_install


%post
/usr/bin/vconftool set -t int memory/data_router/osp_serial_open "0" -u 0 -i -f


%files
%manifest %{name}.manifest
%defattr(-, root, root)
%license LICENSE
/usr/bin/data-router
