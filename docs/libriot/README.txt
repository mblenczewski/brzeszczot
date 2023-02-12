libriot
===============================================================================
libriot is the support library to brzeszczot. It currently implements reading
and writing of Riot's WAD and INIBIN formats, documented below.

libriot: Primitive types
===============================================================================
In the below format descriptions, the following primitive types are assumed:
+-----------+---------------+-------------------------------------------------+
| Type Tag  | Size (bytes)  | Description                                     |
+-----------+---------------+-------------------------------------------------+
| chr8      | 1 byte        | ASCII character                                 |
| b8        | 1 byte        | Boolean value                                   |
| f32       | 4 bytes       | 32-bit floating point value                     |
| s8        | 1 byte        | Signed 8-bit integer                            |
| s16       | 2 bytes       | Signed 16-bit integer                           |
| s32       | 4 bytes       | Signed 32-bit integer                           |
| s64       | 8 bytes       | Signed 64-bit integer                           |
| u8        | 1 byte        | Unsigned 8-bit integer                          |
| u16       | 2 bytes       | Unsigned 16-bit integer                         |
| u32       | 4 bytes       | Unsigned 32-bit integer                         |
| u64       | 8 bytes       | Unsigned 64-bit integer                         |
| fnv1a_u32 | 4 bytes       | FNV1a hash value                                |
| xxh64_u64 | 8 bytes       | XXH64 hash value                                |
+-----------+---------------+-------------------------------------------------+

Arrays (fixed length contiguous sequences of elements) are represented by the
following convention:
+-----------+-----------------------------------------------------------------+
| Type Tag  | Description                                                     |
+-----------+-----------------------------------------------------------------+
| T[const]  | Homogenous array of type T, with constant length `const`        |
| T[Var]    | Homogenous array of type T, with indirect length given by `Var` |
+-----------+-----------------------------------------------------------------+

Bitfields may also be declared by formats, and are represented by the following
connvention:
+-----------+-----------------------------------------------------------------+
| Type Tag  | Description                                                     |
+-----------+-----------------------------------------------------------------+
| T[a:b]    | Bitfield across T's backing bits, spanning from bit a to bit b  |
+-----------+-----------------------------------------------------------------+

Note that in addition to the above primitives, formats might define additional
primitive or compound types unique to said format.

libriot: WAD intro
===============================================================================
The WAD format (no relation to Doom's WAD) is used as an archive of INIBINs.
My understanding of this format came from studying the excellent LeagueToolkit
tool (https://github.com/LeagueToolkit/LeagueToolkit), without which this
library wouldn't have been possible.

libriot: WAD format
===============================================================================
Riot's WAD format has multiple versions, and is structured as follows:
+-----------------------------------------------------------------------------+
| Header                                                                      |
| +-------------------------------------------------------------------------+ |
| | 'RW' Magic : chr[2]                                                     | |
| | Major Version : u8                                                      | |
| | Minor Version : u8                                                      | |
| |                                                                         | |
| | V2 Signature (if Major Version == 2)                                    | |
| | +---------------------------------------------------------------------+ | |
| | | ECDSA Length : u32                                                  | | |
| | | ECDSA Signature : u8[80]                                            | | |
| | | Checksum : u64                                                      | | |
| | +---------------------------------------------------------------------+ | |
| |                                                                         | |
| | V3 Signature (if Major Version == 3)                                    | |
| | +---------------------------------------------------------------------+ | |
| | | ECDSA Signature : u8[256]                                           | | |
| | | Checksum : u64                                                      | | |
| | +---------------------------------------------------------------------+ | |
| |                                                                         | |
| | Table Of Contents (if Major Version <= 2)                               | |
| | +---------------------------------------------------------------------+ | |
| | | Start Offset : u16                                                  | | |
| | | Entry Size : u16                                                    | | |
| | +---------------------------------------------------------------------+ | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| Entries                                                                     |
| +-------------------------------------------------------------------------+ |
| | Count : u32                                                             | |
| | Chunks[Count]                                                           | |
| | +---------------------------------------------------------------------+ | |
| | | Path Hash : xxh64_u64                                               | | |
| | | Data Offset : u32                                                   | | |
| | | Compressed Size : u32                                               | | |
| | | Decompressed Size : u32                                             | | |
| | | Sub-Chunk Count : b8[0:3]                                           | | |
| | | Compression : b8[4:7]                                               | | |
| | | Duplicated : b8                                                     | | |
| | | Start Sub-Chunk Index : u16                                         | | |
| | | Checksum : u64 (if Major Version >= 2)                              | | |
| | +---------------------------------------------------------------------+ | |
| +-------------------------------------------------------------------------+ |
+-----------------------------------------------------------------------------+

The compression used in a given WAD entry is one of the following enum values:
+-------+---------------------------------------------------------------------+
| Value | Description                                                         |
+-------+---------------------------------------------------------------------+
| 0     | No compression                                                      |
| 1     | GZip compression                                                    |
| 2     | Satellite chunk; the chunk data is a string file redirect           |
| 3     | Zstd compression                                                    |
| 4     | Zstd compression using sub-chunks                                   |
+-------+---------------------------------------------------------------------+

libriot: INIBIN into
===============================================================================
The INIBIN format is used to store character model and animation data. My
understanding of the format came from studying existing tools written by
yretenai (https://github.com/yretenai), and moonshadow565
(https://github.com/moonshadow565). Without them, I likely wouldn't have been
able to write this library.

libriot: INIBIN format
===============================================================================
Riot's INIBIN file format has multiple versions, and is structured as follows:
+-----------------------------------------------------------------------------+
| PTCH Header (v3+, optional)                                                 |
| +-------------------------------------------------------------------------+ |
| | 'PTCH' Magic : chr8[4]                                                  | |
| | Unknown bytes : u64                                                     | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| Header (v1+)                                                                |
| +-------------------------------------------------------------------------+ |
| | 'PROP' Magic : chr8[4]                                                  | |
| | Version : u32                                                           | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| Linked Files (v2+)                                                          |
| +-------------------------------------------------------------------------+ |
| | Count : u32                                                             | |
| | File Names : riot_bin_str[Count]                                        | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| Prop Entries (v1+)                                                          |
| +-------------------------------------------------------------------------+ |
| | Count : u32                                                             | |
| | Entry Name Hashes : fnv1a_u32[Count]                                    | |
| | Entries[Count]                                                          | |
| | +---------------------------------------------------------------------+ | |
| | | Length : u32                                                        | | |
| | | Name Hash : fnv1a_u32                                               | | |
| | | Count : u16                                                         | | |
| | | Items[Count]                                                        | | |
| | | +-----------------------------------------------------------------+ | | |
| | | | Name Hash : fnv1a_u32                                           | | | |
| | | | Type : u8                                                       | | | |
| | | | Value : riot_bin_node<Type>                                     | | | |
| | | +-----------------------------------------------------------------+ | | |
| | +---------------------------------------------------------------------+ | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| Patch Entries (v3+, if PTCH header is present)                              |
| +-------------------------------------------------------------------------+ |
| | Count : u32                                                             | |
| | Entries[Count]                                                          | |
| | +---------------------------------------------------------------------+ | |
| | | Name Hash : fnv1a_u32                                               | | |
| | | Length : u32                                                        | | |
| | | Type : u8                                                           | | |
| | | String : riot_bin_str                                               | | |
| | | Value : riot_bin_node<Type>                                         | | |
| | +---------------------------------------------------------------------+ | |
| +-------------------------------------------------------------------------+ |
+-----------------------------------------------------------------------------+

In addition to the default primitives, Riot's INIBIN format declares the
following primitive types:
+-----------+---------------+-------------------------------------------------+
| Type Tag  | Size (bytes)  | Description                                     |
+-----------+---------------+-------------------------------------------------+
| fvec2     | 8 bytes       | Vector containing 2 f32 values                  |
| fvec3     | 12 bytes      | Vector containing 3 f32 values                  |
| fvec4     | 16 bytes      | Vector containing 4 f32 values                  |
| fmat4x4   | 64 bytes      | Row-major 4x4 matrix containing 16 f32 values   |
| rgba      | 4 bytes       | RGBA32 colour value                             |
| flag8     | 1 byte        | 8-bit flag value                                |
+-----------+---------------+-------------------------------------------------+

The following type aliases are declared:
+-----------+---------------+-------------------------------------------------+
| Type Tag  | Aliases Type  | Description                                     |
+-----------+---------------+-------------------------------------------------+
| hash      | fnv1a_u32     | ? (TODO)                                        |
| link      | fnv1a_u32     | ? (TODO)                                        |
| file      | xxh64_u64     | ? (TODO)                                        |
+-----------+---------------+-------------------------------------------------+

All of the above primitives can be read directly from the INIBIN file. However,
the following compound types are also declared:
+-----------------+-----------------------------------------------------------+
| Type Tag        | Description                                               |
+-----------------+-----------------------------------------------------------+
| riot_bin_str    | 16-bit length prefixed ASCII string                       |
| riot_bin_ptr    | List of riot_bin_field                                    |
| riot_bin_embed  | List of riot_bin_field                                    |
| riot_bin_opt    | Optional riot_bin_node<T> value                           |
| riot_bin_list   | Homogenous list<T> of riot_bin_node<T>                    |
| riot_bin_map    | Homogenous map<K, V> of riot_bin_pair<K, V>               |
+-----------------+-----------------------------------------------------------+

The above compound types have the following structures:
+-----------------------------------------------------------------------------+
| riot_bin_str                                                                |
| +-------------------------------------------------------------------------+ |
| | Count : u16                                                             | |
| | Data : chr8[Count]                                                      | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| riot_bin_field                                                              |
| +-------------------------------------------------------------------------+ |
| | Name Hash : fnv1a_u32                                                   | |
| | Type : u8                                                               | |
| | Value : riot_bin_node<Type>                                             | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| riot_bin_ptr, riot_bin_embed                                                |
| +-------------------------------------------------------------------------+ |
| | Name Hash : fnv1a_u32                                                   | |
| | Size : u32                                                              | |
| | Count : u16                                                             | |
| | Items : riot_bin_field[Count]                                           | |
| + ------------------------------------------------------------------------+ |
|                                                                             |
| riot_bin_opt                                                                |
| +-------------------------------------------------------------------------+ |
| | Type : u8                                                               | |
| | Exists : b8                                                             | |
| | Value : riot_bin_node<Type> (if Exists == 1)                            | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| riot_bin_list                                                               |
| +-------------------------------------------------------------------------+ |
| | Type : u8                                                               | |
| | Size : u32                                                              | |
| | Count : u32                                                             | |
| | Items : riot_bin_node<Type>[Count]                                      | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| riot_bin_pair<Key Type, Val Type>                                           |
| +-------------------------------------------------------------------------+ |
| | Key : riot_bin_node<Key Type>                                           | |
| | Val : riot_bin_node<Val Type>                                           | |
| +-------------------------------------------------------------------------+ |
|                                                                             |
| riot_bin_map                                                                |
| +-------------------------------------------------------------------------+ |
| | Key Type : u8                                                           | |
| | Val Type : u8                                                           | |
| | Size : u32                                                              | |
| | Count : u32                                                             | |
| | Items : riot_bin_pair<Key Type, Val Type>[Count]                        | |
| +-------------------------------------------------------------------------+ |
+-----------------------------------------------------------------------------+

The INIBIN format conceptually collates all of the primitive and compound
types declared above into a single, tagged union (`riot_bin_node`). The prop
entries, patch entries, and (pseudo-)collections then store this union type.
