# Code metrics baseline: earthworm

Measured by Tezcatl 0.1.0 (Ubuntu clang version 22.1.8 (++20260714014902+ca7933e47d3a-1~exp1~20260714135019.80)) from the compilation database in `earthworm`: 929 translation units parsed, 0 with errors. 120 database entries for other languages (such as Fortran) were not parsed. 1584 of the 2910 source files under the root were parsed, as a unit or through an include. The other 1326 (`parsed` is `no` in `files.csv`: code for other platforms, parts the build skips) count toward lines of code and nothing else. Every figure is defined in Tezcatl's `docs/metrics.md`; the same data, complete, is in `report.json` and the CSV files next to this report.

Complexity thresholds: flagged over 10, high over 20. Test code is recognised by `**/test/**`, `**/tests/**`, `**/*_test.*`, `**/test_*.*`; it counts toward test lines only.

## Summary

| Module | Files (parsed) | Code lines | Test code lines | Functions | Mean complexity | Max | Over threshold | Documented | In a cycle |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `archiving` | 156 (97) | 35853 | 0 | 357 | 12.50 | 115 | 122 | 39.1% | no |
| `data_exchange` | 155 (95) | 29670 | 0 | 354 | 12.79 | 152 | 116 | 20.7% | no |
| `data_sources` | 477 (177) | 95788 | 615 | 906 | 6.90 | 190 | 158 | 38.7% | no |
| `diagnostic_tools` | 33 (20) | 11145 | 448 | 70 | 18.07 | 181 | 29 | 90.9% | no |
| `display` | 51 (10) | 15665 | 0 | 232 | 9.14 | 133 | 60 | 84.5% | no |
| `eew` | 79 (33) | 14619 | 0 | 81 | 12.36 | 133 | 21 | 40.0% | no |
| `grab_bag` | 41 (15) | 10752 | 0 | 66 | 12.88 | 89 | 27 | 20.4% | no |
| `html` | 1 (0) | 604 | 0 | 0 | 0.00 | 0 | 0 | n/a | no |
| `include` | 185 (129) | 14133 | 0 | 0 | 0.00 | 0 | 0 | 58.4% | no |
| `libsrc` | 182 (123) | 63317 | 203 | 1164 | 9.56 | 711 | 275 | n/a | no |
| `reporting` | 53 (39) | 23546 | 27 | 298 | 11.13 | 157 | 73 | 77.1% | no |
| `seismic_processing` | 401 (332) | 93234 | 168 | 1156 | 12.05 | 193 | 324 | 68.8% | no |
| `system_control` | 24 (8) | 2310 | 0 | 21 | 7.05 | 19 | 4 | n/a | no |
| `third-party/b64` | 4 (3) | 217 | 0 | 3 | 10.67 | 22 | 1 | 100.0% | no |
| `third-party/filterpicker` | 20 (10) | 1751 | 244 | 12 | 9.33 | 52 | 3 | 13.6% | no |
| `third-party/lib330` | 60 (60) | 25027 | 0 | 482 | 6.21 | 166 | 58 | 55.0% | no |
| `third-party/lib660` | 42 (42) | 11009 | 0 | 251 | 6.59 | 99 | 40 | 55.5% | no |
| `third-party/libcrypto` | 7 (7) | 375 | 0 | 9 | 2.67 | 9 | 0 | 71.4% | no |
| `third-party/libdali` | 14 (13) | 3625 | 0 | 73 | 8.73 | 45 | 26 | 38.2% | no |
| `third-party/libgd` | 23 (22) | 10563 | 0 | 62 | 6.82 | 25 | 13 | 26.8% | no |
| `third-party/libmseed` | 60 (27) | 25303 | 3726 | 700 | 6.92 | 196 | 97 | 66.8% | no |
| `third-party/libslink` | 280 (271) | 125709 | 0 | 2477 | 5.27 | 155 | 282 | 80.1% | no |
| `third-party/mysql-connector` | 509 (0) | 192971 | 16654 | 0 | 0.00 | 0 | 0 | n/a | no |
| `third-party/q660util` | 16 (16) | 4216 | 0 | 79 | 5.75 | 66 | 10 | 52.5% | no |
| `third-party/qlib2` | 35 (33) | 9618 | 0 | 198 | 7.47 | 79 | 41 | 50.0% | no |
| `third-party/sqlite` | 2 (2) | 130545 | 0 | 1978 | 7.02 | 702 | 272 | 45.5% | no |
| **Total** | 2910 (1584) | 951565 | 22085 | 11029 | 8.03 | 711 | 2052 | 57.3% |  |

## Lines of code

Production: 951565 code lines of 1476878 physical (351905 comment, 173408 blank). Test: 22085 code lines of 30546.

| Module | Files | Parsed | Physical | Code | Comment | Blank | Test physical | Test code |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `archiving` | 156 | 97 | 56523 | 35853 | 13617 | 7053 | 0 | 0 |
| `data_exchange` | 155 | 95 | 45702 | 29670 | 10654 | 5378 | 0 | 0 |
| `data_sources` | 477 | 177 | 153861 | 95788 | 32855 | 25218 | 716 | 615 |
| `diagnostic_tools` | 33 | 20 | 16801 | 11145 | 3323 | 2333 | 686 | 448 |
| `display` | 51 | 10 | 22294 | 15665 | 3723 | 2906 | 0 | 0 |
| `eew` | 79 | 33 | 22455 | 14619 | 4867 | 2969 | 0 | 0 |
| `grab_bag` | 41 | 15 | 25628 | 10752 | 9800 | 5076 | 0 | 0 |
| `html` | 1 | 0 | 846 | 604 | 115 | 127 | 0 | 0 |
| `include` | 185 | 129 | 40137 | 14133 | 18878 | 7126 | 0 | 0 |
| `libsrc` | 182 | 123 | 96678 | 63317 | 22644 | 10717 | 293 | 203 |
| `reporting` | 53 | 39 | 32786 | 23546 | 5046 | 4194 | 37 | 27 |
| `seismic_processing` | 401 | 332 | 142726 | 93234 | 32249 | 17243 | 252 | 168 |
| `system_control` | 24 | 8 | 4445 | 2310 | 1578 | 557 | 0 | 0 |
| `third-party/b64` | 4 | 3 | 353 | 217 | 77 | 59 | 0 | 0 |
| `third-party/filterpicker` | 20 | 10 | 2746 | 1751 | 519 | 476 | 352 | 244 |
| `third-party/lib330` | 60 | 60 | 28617 | 25027 | 1890 | 1700 | 0 | 0 |
| `third-party/lib660` | 42 | 42 | 15245 | 11009 | 1484 | 2752 | 0 | 0 |
| `third-party/libcrypto` | 7 | 7 | 811 | 375 | 300 | 136 | 0 | 0 |
| `third-party/libdali` | 14 | 13 | 6219 | 3625 | 1785 | 809 | 0 | 0 |
| `third-party/libgd` | 23 | 22 | 12348 | 10563 | 800 | 985 | 0 | 0 |
| `third-party/libmseed` | 60 | 27 | 40272 | 25303 | 9846 | 5123 | 5104 | 3726 |
| `third-party/libslink` | 280 | 271 | 215925 | 125709 | 65281 | 24935 | 0 | 0 |
| `third-party/mysql-connector` | 509 | 0 | 250814 | 192971 | 29849 | 27994 | 23106 | 16654 |
| `third-party/q660util` | 16 | 16 | 5851 | 4216 | 669 | 966 | 0 | 0 |
| `third-party/qlib2` | 35 | 33 | 15677 | 9618 | 4261 | 1798 | 0 | 0 |
| `third-party/sqlite` | 2 | 2 | 221118 | 130545 | 75795 | 14778 | 0 | 0 |

## Cyclomatic complexity

| Module | Functions | Mean | Median | p90 | Max | Over 10 | Over 20 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `archiving` | 357 | 12.50 | 7.0 | 31 | 115 | 122 | 61 |
| `data_exchange` | 354 | 12.79 | 6.0 | 37 | 152 | 116 | 65 |
| `data_sources` | 906 | 6.90 | 3.0 | 16 | 190 | 158 | 57 |
| `diagnostic_tools` | 70 | 18.07 | 7.5 | 44 | 181 | 29 | 18 |
| `display` | 232 | 9.14 | 5.0 | 19 | 133 | 60 | 21 |
| `eew` | 81 | 12.36 | 5.0 | 27 | 133 | 21 | 11 |
| `grab_bag` | 66 | 12.88 | 8.0 | 26 | 89 | 27 | 10 |
| `html` | 0 | 0.00 | 0.0 | 0 | 0 | 0 | 0 |
| `include` | 0 | 0.00 | 0.0 | 0 | 0 | 0 | 0 |
| `libsrc` | 1164 | 9.56 | 4.0 | 22 | 711 | 275 | 135 |
| `reporting` | 298 | 11.13 | 5.0 | 26 | 157 | 73 | 35 |
| `seismic_processing` | 1156 | 12.05 | 5.0 | 32 | 193 | 324 | 181 |
| `system_control` | 21 | 7.05 | 8.0 | 13 | 19 | 4 | 0 |
| `third-party/b64` | 3 | 10.67 | 9.0 | 22 | 22 | 1 | 1 |
| `third-party/filterpicker` | 12 | 9.33 | 4.5 | 15 | 52 | 3 | 1 |
| `third-party/lib330` | 482 | 6.21 | 3.0 | 12 | 166 | 58 | 29 |
| `third-party/lib660` | 251 | 6.59 | 3.0 | 15 | 99 | 40 | 15 |
| `third-party/libcrypto` | 9 | 2.67 | 2.0 | 9 | 9 | 0 | 0 |
| `third-party/libdali` | 73 | 8.73 | 8.0 | 17 | 45 | 26 | 5 |
| `third-party/libgd` | 62 | 6.82 | 4.0 | 20 | 25 | 13 | 6 |
| `third-party/libmseed` | 700 | 6.92 | 2.0 | 14 | 196 | 97 | 49 |
| `third-party/libslink` | 2477 | 5.27 | 3.0 | 11 | 155 | 282 | 82 |
| `third-party/mysql-connector` | 0 | 0.00 | 0.0 | 0 | 0 | 0 | 0 |
| `third-party/q660util` | 79 | 5.75 | 3.0 | 14 | 66 | 10 | 5 |
| `third-party/qlib2` | 198 | 7.47 | 3.0 | 19 | 79 | 41 | 18 |
| `third-party/sqlite` | 1978 | 7.02 | 3.0 | 13 | 702 | 272 | 121 |

### Most complex functions

The 20 most complex of 11029 production functions; all of them are in `functions.csv`.

| Complexity | Function | Where | Module |
|---:|---|---|---|
| 711 | `GetRegion(double, double)` | `src/libsrc/earlybird/geotools.c:549` | `libsrc` |
| 702 | `sqlite3VdbeExec(Vdbe *)` | `src/libsrc/util/sqlite3.c:81811` | `third-party/sqlite` |
| 330 | `yy_reduce(yyParser *, unsigned int, int, Token)` | `src/libsrc/util/sqlite3.c:142196` | `third-party/sqlite` |
| 244 | `sqlite3Pragma(Parse *, Token *, Token *, Token *, int)` | `src/libsrc/util/sqlite3.c:117206` | `third-party/sqlite` |
| 196 | `msr3_pack_header2(const MS3Record *, char *, uint32_t, int8_t)` | `src/libsrc/util/libmseed/pack.c:858` | `third-party/libmseed` |
| 193 | `ReadConfig(LMPARAMS *, char *, int *)` | `src/seismic_processing/localmag/lm_config.c:554` | `seismic_processing` |
| 192 | `WarnOrAdvisory(double, double)` | `src/libsrc/earlybird/geotools.c:2003` | `libsrc` |
| 191 | `ms_parse_raw2(const char *, int, int8_t, int8_t)` | `src/libsrc/util/libmseed/parseutils.c:527` | `third-party/libmseed` |
| 190 | `main(int, char **)` | `src/data_sources/k2ew/k2ewmain.c:472` | `data_sources` |
| 181 | `main(int, char **)` | `src/diagnostic_tools/latency/latency.c:206` | `diagnostic_tools` |
| 181 | `sqlite3VXPrintf(StrAccum *, const char *, struct __va_list_tag *)` | `src/libsrc/util/sqlite3.c:26419` | `third-party/sqlite` |
| 166 | `proc_insequence(pq330, int32_t)` | `src/libsrc/lib330/libslider.c:274` | `third-party/lib330` |
| 163 | `ReadConfig(GMPARAMS *, char *)` | `src/seismic_processing/gmew/gm_config.c:315` | `seismic_processing` |
| 157 | `process_message(HypoArc *, MAG_INFO *, MAG_INFO *, MSG_LOGO)` | `src/reporting/ewhtmlemail/ewhtmlemail.c:2045` | `reporting` |
| 156 | `rd_strongmotionII(char **, SM_INFO *, int)` | `src/libsrc/util/rw_strongmotionII.c:102` | `libsrc` |
| 156 | `sqlite3WhereCodeOneLoopStart(WhereInfo *, int, Bitmask)` | `src/libsrc/util/sqlite3.c:131966` | `third-party/sqlite` |
| 155 | `mbedtls_high_level_strerr(int)` | `src/data_exchange/slink2ew/libslink/mbedtls/library/error.c:174` | `third-party/libslink` |
| 153 | `config(char *)` | `src/reporting/ewhtmlemail/ewhtmlemail.c:857` | `reporting` |
| 152 | `exportfilter_com()` | `src/data_exchange/export/scnfilter.c:102` | `data_exchange` |
| 147 | `main(int, char **)` | `src/data_sources/nmxptool/src/nmxptool.c:147` | `data_sources` |

## Halstead

Volume and effort add up across functions; difficulty does not, and is per function in `functions.csv`.

| Module | Volume | Effort |
|---|---:|---:|
| `archiving` | 718621 | 38252419 |
| `data_exchange` | 695626 | 32510008 |
| `data_sources` | 906382 | 37831242 |
| `diagnostic_tools` | 220786 | 14013304 |
| `display` | 427043 | 27343213 |
| `eew` | 188711 | 13116804 |
| `grab_bag` | 130345 | 4768226 |
| `html` | 0 | 0 |
| `include` | 0 | 0 |
| `libsrc` | 1858415 | 131365699 |
| `reporting` | 604303 | 34713175 |
| `seismic_processing` | 2471256 | 155778026 |
| `system_control` | 19451 | 364898 |
| `third-party/b64` | 5353 | 429935 |
| `third-party/filterpicker` | 28998 | 2166704 |
| `third-party/lib330` | 676301 | 26346476 |
| `third-party/lib660` | 341130 | 15480170 |
| `third-party/libcrypto` | 17506 | 1238518 |
| `third-party/libdali` | 83895 | 3542710 |
| `third-party/libgd` | 61015 | 2893789 |
| `third-party/libmseed` | 646504 | 38508001 |
| `third-party/libslink` | 1926345 | 78302849 |
| `third-party/mysql-connector` | 0 | 0 |
| `third-party/q660util` | 73714 | 3350940 |
| `third-party/qlib2` | 203152 | 10616745 |
| `third-party/sqlite` | 2411609 | 206009192 |

### Highest effort

| Effort | Volume | Difficulty | Function | Where |
|---:|---:|---:|---|---|
| 71178472 | 221641 | 321.1 | `sqlite3VdbeExec(Vdbe *)` | `src/libsrc/util/sqlite3.c:81811` |
| 15816232 | 99574 | 158.8 | `yy_reduce(yyParser *, unsigned int, int, Token)` | `src/libsrc/util/sqlite3.c:142196` |
| 14354637 | 30342 | 473.1 | `vpassm(double *, double *, double *, double *, double *, long, long, long, long, long, long, long, long)` | `src/libsrc/util/fft99.c:614` |
| 7769325 | 59974 | 129.5 | `sqlite3Pragma(Parse *, Token *, Token *, Token *, int)` | `src/libsrc/util/sqlite3.c:117206` |
| 6794274 | 36451 | 186.4 | `grid_joint(long)` | `src/seismic_processing/binder_max/grid.c:682` |
| 6658781 | 45873 | 145.2 | `sqlite3WhereCodeOneLoopStart(WhereInfo *, int, Bitmask)` | `src/libsrc/util/sqlite3.c:131966` |
| 5456122 | 26530 | 205.7 | `balance_nonroot(MemPage *, int, u8 *, int, int)` | `src/libsrc/util/sqlite3.c:68809` |
| 4967729 | 45646 | 108.8 | `main(int, char **)` | `src/data_sources/k2ew/k2ewmain.c:472` |
| 4784211 | 47193 | 101.4 | `ms_parse_raw2(const char *, int, int8_t, int8_t)` | `src/libsrc/util/libmseed/parseutils.c:527` |
| 4605671 | 25872 | 178.0 | `sqlite3VXPrintf(StrAccum *, const char *, struct __va_list_tag *)` | `src/libsrc/util/sqlite3.c:26419` |

## Documentation

Public API declared in production headers with a comment attached. Plain comments count; the doxygen column says how many use ///, //!, /** or /*!.

| Module | Declarations | Documented | Share | Doxygen |
|---|---:|---:|---:|---:|
| `archiving` | 824 | 322 | 39.1% | 19 |
| `data_exchange` | 145 | 30 | 20.7% | 8 |
| `data_sources` | 2982 | 1153 | 38.7% | 139 |
| `diagnostic_tools` | 33 | 30 | 90.9% | 6 |
| `display` | 328 | 277 | 84.5% | 8 |
| `eew` | 260 | 104 | 40.0% | 4 |
| `grab_bag` | 54 | 11 | 20.4% | 0 |
| `html` | 0 | 0 | n/a | 0 |
| `include` | 4256 | 2486 | 58.4% | 312 |
| `libsrc` | 0 | 0 | n/a | 0 |
| `reporting` | 358 | 276 | 77.1% | 8 |
| `seismic_processing` | 2432 | 1672 | 68.8% | 20 |
| `system_control` | 0 | 0 | n/a | 0 |
| `third-party/b64` | 3 | 3 | 100.0% | 3 |
| `third-party/filterpicker` | 147 | 20 | 13.6% | 1 |
| `third-party/lib330` | 2205 | 1213 | 55.0% | 0 |
| `third-party/lib660` | 1040 | 577 | 55.5% | 0 |
| `third-party/libcrypto` | 28 | 20 | 71.4% | 0 |
| `third-party/libdali` | 102 | 39 | 38.2% | 30 |
| `third-party/libgd` | 142 | 38 | 26.8% | 0 |
| `third-party/libmseed` | 716 | 478 | 66.8% | 455 |
| `third-party/libslink` | 2697 | 2161 | 80.1% | 2015 |
| `third-party/mysql-connector` | 0 | 0 | n/a | 0 |
| `third-party/q660util` | 591 | 310 | 52.5% | 0 |
| `third-party/qlib2` | 1089 | 544 | 50.0% | 4 |
| `third-party/sqlite` | 481 | 219 | 45.5% | 3 |

### Undocumented declarations

The first 50 of 8930, in file order; all of them are in `api.csv`.

| Declaration | Kind | Where |
|---|---|---|
| `vector::x` | field | `include/ahhead.h:47` |
| `vector::y` | field | `include/ahhead.h:48` |
| `vector` | type_alias | `include/ahhead.h:49` |
| `complex::r` | field | `include/ahhead.h:52` |
| `complex::i` | field | `include/ahhead.h:53` |
| `complex` | type_alias | `include/ahhead.h:54` |
| `d_complex::r` | field | `include/ahhead.h:57` |
| `d_complex::i` | field | `include/ahhead.h:58` |
| `d_complex` | type_alias | `include/ahhead.h:59` |
| `tensor::xx` | field | `include/ahhead.h:62` |
| `tensor::yy` | field | `include/ahhead.h:63` |
| `tensor::xy` | field | `include/ahhead.h:64` |
| `tensor` | type_alias | `include/ahhead.h:65` |
| `ah_time` | type | `include/ahhead.h:67` |
| `calib` | type | `include/ahhead.h:76` |
| `station_info` | type | `include/ahhead.h:81` |
| `event_info` | type | `include/ahhead.h:93` |
| `record_info` | type | `include/ahhead.h:101` |
| `ahhed` | type_alias | `include/ahhead.h:117` |
| `_ARCEVENTINFO::mag` | field | `include/arcdb.h:13` |
| `_ARCEVENTINFO::eventID` | field | `include/arcdb.h:14` |
| `_ARCEVENTINFO::rowid` | field | `include/arcdb.h:15` |
| `_ARCEVENTINFO::update_count` | field | `include/arcdb.h:16` |
| `_ARCEVENTINFO::version` | field | `include/arcdb.h:17` |
| `_ARCEVENTINFO::flags` | field | `include/arcdb.h:18` |
| `ARCEVENTINFO` | type_alias | `include/arcdb.h:19` |
| `Base64encode_len(int)` | function | `include/base64.h:97` |
| `Base64encode(char *, const char *, int)` | function | `include/base64.h:98` |
| `Base64decode_len(const char *)` | function | `include/base64.h:100` |
| `Base64decode(char *, const char *)` | function | `include/base64.h:101` |
| `make_butterworth_filter(const unsigned int, Complex *, double *, const double)` | function | `include/butterworth.h:36` |
| `Greg` | type | `include/chron3.h:39` |
| `Greg::year` | field | `include/chron3.h:40` |
| `Greg::month` | field | `include/chron3.h:41` |
| `Greg::day` | field | `include/chron3.h:42` |
| `Greg::hour` | field | `include/chron3.h:43` |
| `Greg::minute` | field | `include/chron3.h:44` |
| `Greg::second` | field | `include/chron3.h:45` |
| `date17(double, char *)` | function | `include/chron3.h:51` |
| `date18(double, char *)` | function | `include/chron3.h:52` |
| `datime(double, struct Greg *)` | function | `include/chron3.h:54` |
| `gregor(long, struct Greg *)` | function | `include/chron3.h:55` |
| `grg(long, struct Greg *)` | function | `include/chron3.h:56` |
| `julian(struct Greg *)` | function | `include/chron3.h:58` |
| `julmin(struct Greg *)` | function | `include/chron3.h:59` |
| `julsec17(char *)` | function | `include/chron3.h:61` |
| `julsec18(char *)` | function | `include/chron3.h:62` |
| `epochsec17(double *, char *)` | function | `include/chron3.h:64` |
| `epochsec18(double *, char *)` | function | `include/chron3.h:65` |
| `timegm(struct tm *)` | function | `include/chron3.h:66` |

## Test coverage

No coverage data was imported, so there are no coverage figures: not 0%.

## Include dependencies

1584 files, 4966 include edges between them. A cycle is a set of files (or modules) that each reach every other through includes.

No file cycles.

No module cycles.

### Most included files

| Included by | File |
|---:|---|
| 460 | `include/earthworm.h` |
| 240 | `include/transport.h` |
| 181 | `include/kom.h` |
| 180 | `include/trace_buf.h` |
| 124 | `src/data_exchange/slink2ew/libslink/mbedtls/library/common.h` |
| 84 | `src/data_exchange/slink2ew/libslink/mbedtls/include/mbedtls/error.h` |
| 82 | `include/chron3.h` |
| 81 | `src/data_exchange/slink2ew/libslink/mbedtls/include/mbedtls/platform.h` |
| 79 | `include/time_ew.h` |
| 78 | `src/data_exchange/slink2ew/libslink/mbedtls/include/mbedtls/build_info.h` |

### Files that include the most

| Includes | File |
|---:|---|
| 47 | `src/data_exchange/slink2ew/libslink/mbedtls/library/psa_crypto.c` |
| 38 | `src/data_exchange/slink2ew/libslink/mbedtls/library/error.c` |
| 25 | `src/libsrc/lib330/libclient.c` |
| 21 | `src/libsrc/lib660/libclient.c` |
| 20 | `src/reporting/ewhtmlemail/ewhtmlemail.c` |
| 20 | `src/reporting/gmewhtmlemail/gmewhtmlemail.c` |
| 17 | `src/data_exchange/slink2ew/libslink/mbedtls/library/ssl_misc.h` |
| 17 | `src/data_sources/k2ew/k2ewmain.c` |
| 16 | `src/display/sgram/sgram.c` |
| 16 | `src/libsrc/lib330/libcmds.c` |

### Module coupling

The 20 heaviest module-to-module include counts; all of them are in `include-coupling.csv`.

| Edges | From | To |
|---:|---|---|
| 1228 | `third-party/libslink` | `third-party/libslink` |
| 683 | `seismic_processing` | `include` |
| 398 | `seismic_processing` | `seismic_processing` |
| 356 | `libsrc` | `include` |
| 284 | `data_sources` | `data_sources` |
| 273 | `third-party/lib330` | `third-party/lib330` |
| 222 | `archiving` | `include` |
| 211 | `data_exchange` | `include` |
| 210 | `third-party/lib660` | `third-party/lib660` |
| 126 | `include` | `include` |
| 117 | `third-party/qlib2` | `third-party/qlib2` |
| 91 | `data_sources` | `include` |
| 83 | `archiving` | `archiving` |
| 83 | `reporting` | `include` |
| 72 | `data_exchange` | `data_exchange` |
| 68 | `diagnostic_tools` | `include` |
| 62 | `eew` | `include` |
| 47 | `grab_bag` | `include` |
| 44 | `eew` | `eew` |
| 37 | `third-party/libmseed` | `third-party/libmseed` |

