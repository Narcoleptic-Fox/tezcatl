# Code metrics baseline: libmseed

Measured by Tezcatl 0.1.0 (Ubuntu clang version 22.1.8 (++20260714014902+ca7933e47d3a-1~exp1~20260714135019.80)) from the compilation database in `libmseed/build`: 38 translation units parsed, 0 with errors. 53 of the 64 source files under the root were parsed, as a unit or through an include. The other 11 (`parsed` is `no` in `files.csv`: code for other platforms, parts the build skips) count toward lines of code and nothing else. Every figure is defined in Tezcatl's `docs/metrics.md`; the same data, complete, is in `report.json` and the CSV files next to this report.

Complexity thresholds: flagged over 10, high over 20. Test code is recognised by `**/test/**`, `**/tests/**`, `**/*_test.*`, `**/test_*.*`; it counts toward test lines only.

## Summary

| Module | Files (parsed) | Code lines | Test code lines | Functions | Mean complexity | Max | Over threshold | Documented | Line coverage | In a cycle |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `examples` | 11 (0) | 1484 | 0 | 0 | 0.00 | 0 | 0 | n/a | n/a | no |
| `libmseed` | 25 (25) | 14269 | 0 | 229 | 13.84 | 223 | 76 | 59.3% | 57.1% | no |
| `tests` | 26 (26) | 0 | 6658 | 0 | 0.00 | 0 | 0 | n/a | n/a | no |
| `third-party/yyjson` | 2 (2) | 12827 | 0 | 572 | 4.63 | 131 | 46 | 93.3% | 31.1% | no |
| **Total** | 64 (53) | 28580 | 6658 | 801 | 7.26 | 223 | 122 | 68.9% | 46.5% |  |

## Lines of code

Production: 28580 code lines of 44968 physical (10795 comment, 5593 blank). Test: 6658 code lines of 9427.

| Module | Files | Parsed | Physical | Code | Comment | Blank | Test physical | Test code |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `examples` | 11 | 0 | 2172 | 1484 | 387 | 301 | 0 | 0 |
| `libmseed` | 25 | 25 | 23501 | 14269 | 6278 | 2954 | 0 | 0 |
| `tests` | 26 | 26 | 0 | 0 | 0 | 0 | 9427 | 6658 |
| `third-party/yyjson` | 2 | 2 | 19295 | 12827 | 4130 | 2338 | 0 | 0 |

## Cyclomatic complexity

| Module | Functions | Mean | Median | p90 | Max | Over 10 | Over 20 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `examples` | 0 | 0.00 | 0.0 | 0 | 0 | 0 | 0 |
| `libmseed` | 229 | 13.84 | 7.0 | 29 | 223 | 76 | 40 |
| `tests` | 0 | 0.00 | 0.0 | 0 | 0 | 0 | 0 |
| `third-party/yyjson` | 572 | 4.63 | 2.0 | 8 | 131 | 46 | 17 |

### Most complex functions

The 20 most complex of 801 production functions; all of them are in `functions.csv`.

| Complexity | Function | Where | Module |
|---:|---|---|---|
| 223 | `msr3_pack_header2_offsets(const MS3Record *, char *, uint32_t, uint16_t *, uint16_t *, int8_t)` | `pack.c:1136` | `libmseed` |
| 198 | `ms_parse_raw2(const char *, int, int8_t, int8_t)` | `parseutils.c:535` | `libmseed` |
| 134 | `msr3_unpack_mseed2(const char *, int, MS3Record **, uint32_t, int8_t)` | `unpack.c:300` | `libmseed` |
| 131 | `read_root_pretty(u8 *, u8 *, u8 *, yyjson_alc, yyjson_read_flag, yyjson_read_err *)` | `yyjson.c:5718` | `third-party/yyjson` |
| 129 | `_mstl3_addmsr_impl(MS3TraceList *, const MS3Record *, MS3RecordPtr **, int8_t, int8_t, uint32_t, const MS3Tolerance *)` | `tracelist.c:646` | `libmseed` |
| 118 | `read_root_minify(u8 *, u8 *, u8 *, yyjson_alc, yyjson_read_flag, yyjson_read_err *)` | `yyjson.c:5314` | `third-party/yyjson` |
| 97 | `yyjson_incr_read(yyjson_incr_state *, size_t, yyjson_read_err *)` | `yyjson.c:6532` | `third-party/yyjson` |
| 95 | `ms_nstime2timestr_n(nstime_t, char *, size_t, ms_timeformat_t, ms_subseconds_t)` | `genutils.c:1053` | `libmseed` |
| 90 | `read_num(u8 **, u8 **, yyjson_read_flag, yyjson_val *, const char **)` | `yyjson.c:3816` | `third-party/yyjson` |
| 73 | `ms_decode_data(const void *, uint64_t, uint8_t, uint64_t, void *, uint64_t, char *, int8_t, const char *, int8_t)` | `unpack.c:1419` | `libmseed` |
| 68 | `_ms3_readmsr_impl(MS3FileParam **, MS3Record **, const char *, uint32_t, const MS3Selections *, int8_t)` | `fileutils.c:234` | `libmseed` |
| 67 | `read_str_opt(u8, u8 **, u8 *, yyjson_read_flag, yyjson_val *, const char **, u8 **)` | `yyjson.c:4724` | `third-party/yyjson` |
| 65 | `ms3_readselectionsfile(MS3Selections **, const char *)` | `selection.c:442` | `libmseed` |
| 56 | `msr_encode_steim2(int32_t *, uint64_t, int32_t *, uint64_t, int32_t, uint32_t *, const char *, int)` | `packdata.c:426` | `libmseed` |
| 53 | `is_truncated_end(u8 *, u8 *, u8 *, yyjson_read_code, yyjson_read_flag)` | `yyjson.c:3466` | `third-party/yyjson` |
| 49 | `ms_timestr2nstime(const char *)` | `genutils.c:1468` | `libmseed` |
| 44 | `mseh_set_ptr_r(MS3Record *, const char *, void *, char, LM_PARSED_JSON **)` | `extraheaders.c:467` | `libmseed` |
| 44 | `mstl3_pack_next(MS3TraceListPacker *, uint32_t, char **, int32_t *)` | `tracelist.c:3077` | `libmseed` |
| 44 | `unsafe_yyjson_mut_ptr_putx(yyjson_mut_val *, const char *, size_t, yyjson_mut_val *, yyjson_mut_doc *, bool, bool, yyjson_ptr_ctx *, yyjson_ptr_err *)` | `yyjson.c:10449` | `third-party/yyjson` |
| 44 | `yyjson_patch(yyjson_mut_doc *, yyjson_val *, yyjson_val *, yyjson_patch_err *)` | `yyjson.c:10703` | `third-party/yyjson` |

## Halstead

Volume and effort add up across functions; difficulty does not, and is per function in `functions.csv`.

| Module | Volume | Effort |
|---|---:|---:|
| `examples` | 0 | 0 |
| `libmseed` | 433372 | 31850090 |
| `tests` | 0 | 0 |
| `third-party/yyjson` | 330247 | 15835630 |

### Highest effort

| Effort | Volume | Difficulty | Function | Where |
|---:|---:|---:|---|---|
| 4895652 | 48135 | 101.7 | `ms_parse_raw2(const char *, int, int8_t, int8_t)` | `parseutils.c:535` |
| 4354348 | 38305 | 113.7 | `msr3_pack_header2_offsets(const MS3Record *, char *, uint32_t, uint16_t *, uint16_t *, int8_t)` | `pack.c:1136` |
| 4154138 | 36343 | 114.3 | `msr3_unpack_mseed2(const char *, int, MS3Record **, uint32_t, int8_t)` | `unpack.c:300` |
| 2309777 | 14396 | 160.4 | `_mstl3_addmsr_impl(MS3TraceList *, const MS3Record *, MS3RecordPtr **, int8_t, int8_t, uint32_t, const MS3Tolerance *)` | `tracelist.c:646` |
| 1803249 | 16525 | 109.1 | `read_root_pretty(u8 *, u8 *, u8 *, yyjson_alc, yyjson_read_flag, yyjson_read_err *)` | `yyjson.c:5718` |
| 1793497 | 15491 | 115.8 | `read_num(u8 **, u8 **, yyjson_read_flag, yyjson_val *, const char **)` | `yyjson.c:3816` |
| 1697608 | 16446 | 103.2 | `yyjson_incr_read(yyjson_incr_state *, size_t, yyjson_read_err *)` | `yyjson.c:6532` |
| 1602533 | 15316 | 104.6 | `read_root_minify(u8 *, u8 *, u8 *, yyjson_alc, yyjson_read_flag, yyjson_read_err *)` | `yyjson.c:5314` |
| 1131326 | 8154 | 138.8 | `msr_encode_steim2(int32_t *, uint64_t, int32_t *, uint64_t, int32_t, uint32_t *, const char *, int)` | `packdata.c:426` |
| 911523 | 9522 | 95.7 | `_ms3_readmsr_impl(MS3FileParam **, MS3Record **, const char *, uint32_t, const MS3Selections *, int8_t)` | `fileutils.c:234` |

## Documentation

Public API declared in production headers with a comment attached. Plain comments count; the doxygen column says how many use ///, //!, /** or /*!.

| Module | Declarations | Documented | Share | Doxygen |
|---|---:|---:|---:|---:|
| `examples` | 0 | 0 | n/a | 0 |
| `libmseed` | 383 | 227 | 59.3% | 175 |
| `tests` | 0 | 0 | n/a | 0 |
| `third-party/yyjson` | 150 | 140 | 93.3% | 119 |

### Undocumented declarations

The first 50 of 166, in file order; all of them are in `api.csv`.

| Declaration | Kind | Where |
|---|---|---|
| `_priv_realloc(void *, void *, size_t, size_t)` | function | `extraheaders.h:36` |
| `_priv_free(void *, void *)` | function | `extraheaders.h:37` |
| `LM_PARSED_JSON_s::doc` | field | `extraheaders.h:42` |
| `LM_PARSED_JSON_s::mut_doc` | field | `extraheaders.h:43` |
| `ms_gmtime64_r(const int64_t *, struct tm *)` | function | `gmtime64.h:16` |
| `LMTraceListNode::mstl` | field | `internalstate.h:120` |
| `LMTraceIDNode::id` | field | `internalstate.h:138` |
| `LMTraceIDNode::recentseg` | field | `internalstate.h:139` |
| `LMTraceIDNode::nonrecentendbound` | field | `internalstate.h:140` |
| `ms_nstime2time(nstime_t, uint16_t *, uint16_t *, uint8_t *, uint8_t *, uint8_t *, uint32_t *)` | function | `libmseed.h:328` |
| `ms_nstime2timestr_n(nstime_t, char *, size_t, ms_timeformat_t, ms_subseconds_t)` | function | `libmseed.h:330` |
| `ms_nstime2timestr(nstime_t, char *, ms_timeformat_t, ms_subseconds_t)` | function | `libmseed.h:332` |
| `ms_nstime2timestrz(nstime_t, char *, ms_timeformat_t, ms_subseconds_t)` | function | `libmseed.h:334` |
| `ms_time2nstime(int, int, int, int, int, uint32_t)` | function | `libmseed.h:336` |
| `ms_timestr2nstime(const char *)` | function | `libmseed.h:337` |
| `ms_mdtimestr2nstime(const char *)` | function | `libmseed.h:338` |
| `ms_seedtimestr2nstime(const char *)` | function | `libmseed.h:339` |
| `ms_doy2md(int, int, int *, int *)` | function | `libmseed.h:340` |
| `ms_md2doy(int, int, int, int *)` | function | `libmseed.h:341` |
| `msr3_parse(const char *, uint64_t, MS3Record **, uint32_t, int8_t)` | function | `libmseed.h:421` |
| `msr3_pack(const MS3Record *, void (*)(char *, int, void *), void *, int64_t *, uint32_t, int8_t)` | function | `libmseed.h:424` |
| `msr3_pack_init(const MS3Record *, uint32_t, int8_t)` | function | `libmseed.h:430` |
| `msr3_pack_next(MS3RecordPacker *, char **, int32_t *)` | function | `libmseed.h:431` |
| `msr3_pack_free(MS3RecordPacker **, int64_t *)` | function | `libmseed.h:432` |
| `msr3_repack_mseed3(const MS3Record *, char *, uint32_t, int8_t)` | function | `libmseed.h:434` |
| `msr3_repack_mseed2(const MS3Record *, char *, uint32_t, int8_t)` | function | `libmseed.h:437` |
| `msr3_pack_header3(const MS3Record *, char *, uint32_t, int8_t)` | function | `libmseed.h:440` |
| `msr3_pack_header2(const MS3Record *, char *, uint32_t, int8_t)` | function | `libmseed.h:443` |
| `msr3_unpack_data(MS3Record *, int8_t)` | function | `libmseed.h:446` |
| `msr3_data_bounds(const MS3Record *, uint32_t *, uint32_t *)` | function | `libmseed.h:448` |
| `ms_decode_data(const void *, uint64_t, uint8_t, uint64_t, void *, uint64_t, char *, int8_t, const char *, int8_t)` | function | `libmseed.h:450` |
| `msr3_init(MS3Record *)` | function | `libmseed.h:454` |
| `msr3_free(MS3Record **)` | function | `libmseed.h:455` |
| `msr3_duplicate(const MS3Record *, int8_t)` | function | `libmseed.h:456` |
| `msr3_duplicate_extra(const MS3Record *, int8_t, int8_t)` | function | `libmseed.h:457` |
| `msr3_endtime(const MS3Record *)` | function | `libmseed.h:458` |
| `msr3_print(const MS3Record *, int8_t)` | function | `libmseed.h:459` |
| `msr3_resize_buffer(MS3Record *)` | function | `libmseed.h:460` |
| `msr3_sampratehz(const MS3Record *)` | function | `libmseed.h:461` |
| `msr3_nsperiod(const MS3Record *)` | function | `libmseed.h:462` |
| `msr3_host_latency(const MS3Record *)` | function | `libmseed.h:463` |
| `ms3_detect(const char *, uint64_t, uint8_t *)` | function | `libmseed.h:465` |
| `ms_parse_raw3(const char *, int, int8_t)` | function | `libmseed.h:466` |
| `ms_parse_raw2(const char *, int, int8_t, int8_t)` | function | `libmseed.h:467` |
| `ms3_matchselect(const MS3Selections *, const char *, nstime_t, nstime_t, int, const MS3SelectTime **)` | function | `libmseed.h:503` |
| `msr3_matchselect(const MS3Selections *, const MS3Record *, const MS3SelectTime **)` | function | `libmseed.h:506` |
| `ms3_addselect(MS3Selections **, const char *, nstime_t, nstime_t, uint8_t)` | function | `libmseed.h:508` |
| `ms3_addselect_comp(MS3Selections **, char *, char *, char *, char *, nstime_t, nstime_t, uint8_t)` | function | `libmseed.h:510` |
| `ms3_readselectionsfile(MS3Selections **, const char *)` | function | `libmseed.h:513` |
| `ms3_freeselections(MS3Selections *)` | function | `libmseed.h:514` |

## Test coverage

Imported from 1 file(s); Tezcatl does not run tests. 1 recorded file(s) outside the project were left out.

| Module | Lines | Branches | Functions |
|---|---:|---:|---:|
| `examples` | n/a (0/0) | n/a (0/0) | n/a (0/0) |
| `libmseed` | 57.1% (4094/7168) | 46.9% (2930/6251) | 80.5% (182/226) |
| `tests` | n/a (0/0) | n/a (0/0) | n/a (0/0) |
| `third-party/yyjson` | 31.1% (1545/4960) | 9.5% (1893/19975) | 25.0% (20/80) |
| **Total** | 46.5% (5639/12128) | 18.4% (4823/26226) | 66.0% (202/306) |

## Include dependencies

53 files, 80 include edges between them. A cycle is a set of files (or modules) that each reach every other through includes.

No file cycles.

No module cycles.

### Most included files

| Included by | File |
|---:|---|
| 43 | `libmseed.h` |
| 12 | `test/tau/tau.h` |
| 4 | `mseedformat.h` |
| 3 | `yyjson.h` |
| 2 | `extraheaders.h` |
| 2 | `internalstate.h` |
| 2 | `msio.h` |
| 2 | `packdata.h` |
| 2 | `test/testdata.h` |
| 2 | `unpack.h` |

### Files that include the most

| Includes | File |
|---:|---|
| 5 | `pack.c` |
| 4 | `test/test-read.c` |
| 4 | `unpack.c` |
| 3 | `parseutils.c` |
| 3 | `test/test-extraheaders.c` |
| 3 | `test/test-write.c` |
| 2 | `extraheaders.c` |
| 2 | `extraheaders.h` |
| 2 | `fileutils.c` |
| 2 | `genutils.c` |

### Module coupling

Include edges from each module (rows) to each module (columns):

| | `libmseed` | `tests` | `third-party/yyjson` |
|---|---:|---:|---:|
| `libmseed` | 38 | 0 | 1 |
| `tests` | 22 | 17 | 1 |
| `third-party/yyjson` | 0 | 0 | 1 |

