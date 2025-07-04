%define DIRNAME avi
%define SPECNAME smartmet-plugin-%{DIRNAME}
Summary: SmartMet aviation message plugin
Name: %{SPECNAME}
Version: 25.7.2
Release: 1%{?dist}.fmi
License: FMI
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-avi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# https://fedoraproject.org/wiki/Changes/Broken_RPATH_will_fail_rpmbuild
%global __brp_check_rpaths %{nil}

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: %{smartmet_boost}-devel
BuildRequires: smartmet-library-spine-devel >= 25.2.18
BuildRequires: smartmet-library-timeseries-devel >= 25.2.18
BuildRequires: smartmet-library-macgyver-devel >= 25.2.18
BuildRequires: smartmet-library-timeseries-devel >= 25.2.18
BuildRequires: smartmet-engine-avi-devel >= 25.2.20
BuildRequires: smartmet-engine-authentication-devel >= 25.7.2
BuildRequires: bzip2-devel
BuildRequires: zlib-devel
Requires: libconfig17
Requires: smartmet-library-macgyver >= 25.2.18
Requires: smartmet-library-timeseries >= 25.2.18
Requires: smartmet-library-spine >= 25.2.18
Requires: smartmet-engine-avi >= 25.2.20
Requires: smartmet-engine-authentication >= 25.7.2

#TestRequires: smartmet-utils-devel
#TestRequires: smartmet-test-db
#TestRequires: smartmet-library-spine-plugin-test >= 25.2.18

%if 0%{?rhel} && 0%{rhel} == 8
Requires: libpqxx >= 1:7.7.0, libpqxx < 1:7.8.0
BuildRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
#TestRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
%else
%if 0%{?rhel} && 0%{rhel} == 9
Requires: libpqxx >= 1:7.9.0, libpqxx < 1:7.10.0
BuildRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
#TestRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
%else
%if 0%{?rhel} && 0%{rhel} >= 10
Requires: libpqxx >= 1:7.10.0, libpqxx < 1:7.11.0
BuildRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
#TestRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
%else
Requires: libpqxx
BuildRequires: libpqxx-devel
#TestRequires: libpqxx-devel
%endif
%endif
%endif

Provides: %{SPECNAME}
Obsoletes: smartmet-brainstorm-aviplugin < 16.11.1
Obsoletes: smartmet-brainstorm-aviplugin-debuginfo < 16.11.1

%description
SmartMet Aviation plugin

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{SPECNAME}

%build -q -n %{SPECNAME}
make %{_smp_mflags}

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0775,root,root,0775)
%{_datadir}/smartmet/plugins/%{DIRNAME}.so
%defattr(0664,root,root,0775)

%changelog
* Wed Jul  2 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.7.2-1.fmi
- Update according to smartmet-engine-authentivation ABI changes

* Thu Feb 20 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.2.20-1.fmi
- Update to gdal-3.10, geos-3.13 and proj-9.5

* Wed Feb 19 2025 Pertti Kinnia <pertti.kinnia@fmi.fi> - 25.2.19-1.fmi
- Do not allow use of multiple location options (BRAINSTORM-3136)

* Fri Nov  8 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.11.8-1.fmi
- Repackage due to smartmet-library-spine ABI changes

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-1.fmi
- Update to gdal-3.8, geos-3.12, proj-94 and fmt-11

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-1.fmi
- Replace many boost library types with C++ standard library ones

* Thu May 16 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.16-1.fmi
- Clean up boost date-time uses

* Tue May  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.7-1.fmi
- Use Date library (https://github.com/HowardHinnant/date) instead of boost date_time

* Fri Feb 23 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.23-1.fmi
- Full repackaging

* Tue Dec  5 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.12.5-1.fmi
- Repackaged due to an ABI change in SmartMetPlugin

* Fri Jul 28 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.28-1.fmi
- Repackage due to bulk ABI changes in macgyver/newbase/spine

* Tue Mar 21 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.3.21-1.fmi
- Repackage due to authentication engine changes

* Thu Feb  9 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.9-1.fmi
- Add host name to stack traces

* Fri Dec  2 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.12.2-1.fmi
- Update HTTP request method checking and support OPTIONS method

* Thu Oct  6 2022 Pertti Kinnia <pertti.kinnia@fmi.fi> 22.10.6-1.fmi
- Added support for IWXXM messages; BRAINSTORM-905

* Wed Oct  5 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.5-1.fmi
- Do not use boost::noncopyable

* Mon Sep 12 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.9.12-1.fmi
- Silenced several compiler warnings

* Thu Aug 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.25-1.fmi
- Use a generic exception handler for configuration file errors

* Tue Jun 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.21-1.fmi
- Add support for RHEL9, upgrade libpqxx to 7.7.0 (rhel8+) and fmt to 8.1.1

* Tue May 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.24-1.fmi
- Repackaged due to NFmiArea ABI changes

* Thu Apr 28 2022 Andris Pavenis <andris.pavenis@fmi.fi> 22.4.28-1.fmi
- Repackage due to SmartMet::Spine::Reactor ABI changes

* Thu Apr 14 2022 Pertti Kinnia <pertti.kinnia@fmi.fi> - 22.4.14-1.fmi
- Added TAF query time restriction tests (BRAINSTORM-2247, BRAINSTORM-2239). Test can now be run against updated smartmet-test-db (when repackaged) instead of dockerized database (fmidev/smartmet-server-test-db), but test database must first manually be created using '/usr/share/smartmet/test/db/create-local-db.sh db_dir pg_restore collation_C avi.dump' and started using '/usr/share/smartmet/test/db/test-db-ctl.sh path_to_db_dir start -w'. Database host (smartmet-test) must also be changed to path_to_db_dir (using unix socket) in aviengine.conf.in. Shall later modify Makefile and aviengine.conf.in to automatically create/start/use/stop rpm based test database when running tests

* Mon Mar 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.3.21-1.fmi
- Update due to changes in smartmet-library-spine and smartnet-library-timeseries

* Tue Mar  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.8-1.fmi
- Use the new TimeSeries library

* Tue Jan 18 2022 Anssi Reponen <anssi.reponen@fmi.fi> - 22.1.18-1.fmi
- Use DistanceParser for maxdistance URL-parameter (BRAINSTORM-605)

* Thu Nov 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.11-1.fmi
- Repackaged since ValueFormatter ABI changes

* Thu Sep  9 2021 Andris Pavenis <andris.pavenis@fmi.fi> 21.9.9-1.fmi
- Fix require in spec file (libconfig->libconfig17)

* Tue Sep  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> - 21.9.7-1.fmi
- Repackaged due to dependency changes (libconfig -> libconfig17)

* Tue Aug 31 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.31-1.fmi
- Repackaged due to Spine ABI changes

* Tue Aug 17 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.17-1.fmi
- Repackaged due to a modified shutdown API

* Thu Jan 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.14-1.fmi
- Repackaged smartmet to resolve debuginfo issues

* Wed Oct 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.14-1.fmi
- Use new TableFormatter API

* Wed Sep 23 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.23-1.fmi
- Use Fmi::Exception insted of Spine::Exception

* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2

* Wed May 13 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.5.13-1.fmi
- Repackaged since Spine Parameter class ABI changed

* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgraded to Boost 1.69

* Thu Mar 19 2020 Pertti Kinnia <pertti.kinnia@fmi.fi> - 20.3.19-1.fmi
- Added support for excluding finnish SPECIs (BS-1779)

* Thu Sep 26 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.26-1.fmi
- Support for ASAN & TSAN builds

* Tue Jun 25 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.6.25-1.fmi
- Added Access-Control-Allow-Origin header

* Thu Feb 14 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.14-1.fmi
- Added client IP to exception reports

* Tue Dec  4 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.12.4-1.fmi
- Repackaged since Spine::Table size changed

* Mon Aug 13 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.13-1.fmi
- Repackaged since Spine::Location size changed

* Wed Jul 25 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.7.25-1.fmi
- Prefer nullptr over NULL

* Mon Jul 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.7.23-1.fmi
- Repackaged since spine ValueFormatter ABI changed

* Sat Apr  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.7-1.fmi
- Upgrade to boost 1.66

* Tue Mar 20 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.20-1.fmi
- Full recompile of all server plugins

* Mon Aug 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.8.28-1.fmi
- Upgrade to boost 1.65

* Sat Apr  8 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.8-1.fmi
- Simplified error reporting

* Wed Mar 15 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.15-1.fmi
- Recompiled since Spine::Exception changed

* Tue Mar 14 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.14-1.fmi
- Switched to use macgyver StringConversion tools

* Tue Jan 10 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.10-1.fmi
- Using FmiApiKey::getFmiApiKey() to extract request apikey

* Thu Jan  5 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.5-1.fmi
- Query limiting by apikey groups (SOL-4614)
- Note: now requires (and changes to) authentication engine too

* Wed Jan  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.4-1.fmi
- Changed to use renamed SmartMet base libraries

* Wed Nov 30 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.30-1.fmi
- Using test database in test configuration
- No installation for configuration

* Tue Nov  1 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.1-1.fmi
- Namespace and directory name changed

* Tue Sep  6 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.9.6-1.fmi
- New exception handler

* Tue Aug 30 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.8.30-1.fmi
- Base class API change
- Use response code 400 instead of 503

* Mon Aug 15 2016 Markku Koskela <markku.koskela@fmi.fi> - 16.8.15-1.fmi
- The init(),shutdown() and requestHandler() methods are now protected methods
- The requestHandler() method is called from the callRequestHandler() method

* Tue Jun 14 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.14-1.fmi
- Full recompile

* Thu Jun  2 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.2-1.fmi
- Full recompile

* Wed Jun  1 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.1-1.fmi
- Added graceful shutdown

* Fri Mar  4 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.4-1.fmi
- Added new time restriction type (created-valid_to) used to query latest GAFORs.

* Tue Feb  9 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.2.9-1.fmi
- Rebuilt againt the new TimeSeries::Value definition

* Tue Feb  2 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.2.2-1.fmi
- Now using Timeseries None - type

* Sat Jan 23 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.23-1.fmi
- Fmi::TimeZoneFactory API changed

* Mon Jan 18 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.18-1.fmi
- newbase API changed, full recompile

* Mon Dec 14 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.12.14-1.fmi
- Rebuild against new AviEngine

* Thu Dec  3 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.12.3-1.fmi
- Added country-parameter

* Wed Nov 18 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.11.18-1.fmi
- SmartMetPlugin now receives a const HTTP Request

* Mon Nov  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.9-1.fmi
- Using fast case conversion without locale locks when possible

* Mon Oct 26 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.10.26-1.fmi
- Added proper debuginfo packaging

* Mon Oct 12 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.10.12-1.fmi
- Rebuild against the new engine

* Tue Sep 29 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.9.29-1.fmi
- Built against the new engine

* Mon Aug 24 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.24-1.fmi
- Recompiled due to Convenience.h API changes

* Thu Aug 20 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.8.20-1.fmi
- Added tests

* Tue Aug 18 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.18-1.fmi
- Use time formatters from macgyver to avoid global locks from sstreams

* Mon Aug 17 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.17-1.fmi
- Use -fno-omit-frame-pointer to improve perf use

* Fri Aug 14 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.14-1.fmi
- Full recompile due to string formatting changes

* Tue Aug  4 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.8.4-1.fmi
- Initial release of aviation message plugin

