# Data Structure
A BOX2D (or B2) file contains every piece of information needed to create a b2World. 

## Head
At the beginning of a BOX2D file, an 8-byte signature is presented, this design is borrowed form PNG file standard:

|value|purpose|
|----|----|
|B2|Has the high bit set to detect transmission systems that do not support 8-bit data and to reduce the chance that a text file is mistakenly interpreted as a B2 file, or vice versa.|
|42 32 44(or 64)|In ASCII, the letters B2D (or B2d for little endian coded), allowing a person to identify the format easily if it is viewed in a text editor.|
|0D 0A|A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.|
|1A|A byte that stops display of the file under DOS when the command type has been used—the end-of-file character.|
|0A|A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.|

## Chunks

|Length     |Chunk type     |Chunk data     |CRC        |
|----       |----           |----           |----       |
|4 bytes    |4 bytes        |Length bytes	|4 bytes    |
* Endian of length, CRC and chunk data is indicated by the 4th byte of the head. Big-endian or network-byte-order is recommended for file storage.
* The CRC is computed over the chunk type and chunk data, but not the length.

### Chunk Types/Names

|Chunk type|Date stored|
|----|----|
|INFO|db2Info[1]|
|WRLD|db2Wrold[1]|
|JInT|db2Joint[]|
|BODY|db2Body[]|
|FXTR|db2Fixture[]|
|SHpE|db2Shape[]|
* The case of the third letter indicates whether the chunk contains fixed-length sub-structure. Lowercase means it stores variable-length sub-chunks. Like b2shape or b2joint, data structure with variants(extended structures) normally require different lengthes to store its variants, so adopting variable-length sub-chunk is nessary.
* The case of the fourth letter indicates whether the chunk is safe to copy. Lowercase means it is safe to to copy without addintional modification. Upcase means it may contains links to other chunks, and those links might require relocating if linked chunks are touched. (However, sub-chunks do not require copy safety check independently. Actually, the fourth letter of a sub-chunk is normally set to '\0' or other int8_t values, to represent the type of extended date types.)

### Chunk Data
#### INFO
INFO is short for dotBox2d information, and it stores the data defined by db2Info. See:
|Data|Length|C++ type|default value|
|----|----|----|----|
|packSize|1 byte|char|8|
|(not_used)|1 byte|char||
|ver_dotBox2d_0|1 byte|uint8_t||
|ver_dotBox2d_1|1 byte|uint8_t||
|ver_dotBox2d_2|1 byte|uint8_t||
|ver_box2d_0|1 byte|uint8_t||
|ver_box2d_1|1 byte|uint8_t||
|ver_box2d_2|1 byte|uint8_t||

#### WRLD
WRLD, short for world, it's data unit is db2Wrold.
|Data|Length|C++ type|default value|
|----|----|----|----|
|gravity_x|4 bytes|float32_t||
|gravity_y|4 bytes|float32_t||
|bodyList|4 byte|int32_t||
|bodyCount|4 byte|int32_t||
|jointList|4 byte|int32_t||
|jointCount|4 byte|int32_t||

#### JInT
JInT is short for joint. Data unit of this chunk is db2Joint, which is not a fixed-length structure. Actually, the data unit db2Joint is an sub-chunk, which is extendable. The whole chunk can be considered as a two-dimensional variable-length array.
|Data|Length|C++ type|default value|
|----|----|----|----|
|sub_chunk_length|4 bytes|int32_t|
|sub_chunk_type|4 bytes|char[4]|['J', 'I', 'N' ,0(e_unknownJoint)]|
|bodyA|4 bytes|int32_t||
|bodyB|4 bytes|int32_t||
|collideConnected|1(4) bytes|bool|false|
|extend|4 bytes * n|int32_t or float32_t or bool||
* n = sub_chunk_length/4 - 3

#### BODY
BODY, the chunk data of which is an array of it's data unit db2Body.
|Data|Length|C++ type|default value|
|----|----|----|----|
|type|4 bytes|int32_t|0|
|position_x|4 bytes|float32_t|0.0f|
|position_y|4 bytes|float32_t|0.0f|
|angle|4 bytes|float32_t|0.0f|
|linearVelocity_x|4 bytes|float32_t|0.0f|
|linearVelocity_y|4 bytes|float32_t|0.0f|
|angularVelocity|4 bytes|float32_t|0.0f|
|linearDamping|4 bytes|float32_t|0.0f|
|angularDamping|4 bytes|float32_t|0.0f|
|allowSleep|1 byte|bool|true|
|awake|1 byte|bool|true|
|fixedRotation|1 byte|bool|false|
|bullet|1 byte|bool|false|
|enabled|1 byte|bool|true|
|gravityScale|4 bytes|float32_t|1.0f|
|fixtureList|4 bytes|int32_t||
|fixtureCount|4 bytes|int32_t||

#### FXTR
FXTR is short for fixture, and it's data unit is db2Fixture.
|Data|Length|C++ type|default value|
|----|----|----|----|
|friction|4 bytes|float32_t|0.2f|
|restitution|4 bytes|float32_t|0.0f|
|restitutionThreshold|4 bytes|float32_t|1.0f|
|density|4 bytes|float32_t|0.0f|
|isSensor|1 bytes|bool|false|
|filter_categoryBits|2 bytes|uint16_t|0x0001|
|filter_maskBits|2 bytes|uint16_t|0xFFFF|
|filter_groupIndex|2 bytes|int16_t|0|
|shape|4 bytes|int32_t||

#### SHpE
SHpE, short for shap. It's data unit is db2Shape, which acts as variable-length sub-chunk. The whole chunk stores a two-dimensional variable-length array. 
|Data|Length|C++ type|default value|
|----|----|----|----|
|sub_chunk_length|4 bytes|int32_t|
|sub_chunk_type|4 bytes|char[4]|['S', 'H', 'P' ,0(e_circle)]|
|shape_radius|4 bytes|float32_t|0|
|extend|4 bytes * n|int32_t or float32_t or bool||
* n = sub_chunk_length/4 - 1