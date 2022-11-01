#pragma once

#define DB2_NOTE(note)

#define ENDIAN_SENSITIVE

#define DB2_PACK_SIZE 8
#define DB2_PRAGMA(p) _Pragma(#p)
#define DB2_PRAGMA_PACK(s) DB2_PRAGMA(pack(s))
#define DB2_PRAGMA_PACK_ON DB2_PRAGMA_PACK(DB2_PACK_SIZE) // _Pragma("pack(8)")
#define DB2_PRAGMA_PACK_OFF DB2_PRAGMA_PACK()             // _Pragma("pack()")
