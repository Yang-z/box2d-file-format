# Data Structure
A BOX2D (or B2) file contains every piece of information needed to create a B2World. 

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

### Chunk Types/Names

|Chunk type|Date stored|
|----|----|
|INFO|dotBox2dInfo[1]|
|WRLD|dotB2Wrold[1]|
|JOIN|dotB2Joint[]|
|BODY|dotB2Body[]|
|FXTR|dotB2Fixture[]|
|VECT|float32_t[]|

### Chunk Data
#### INFO
INFO is short for dotBox2d information, and it stores the data defined by dotB2Info. See:
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
WRLD, short for World, it's data unit is dotB2Wrold.
|Data|Length|C++ type|default value|
|----|----|----|----|
|gravity_x|4 bytes|float32_t||
|gravity_y|4 bytes|float32_t||
|bodyList|4 byte|int32_t||
|bodyCount|4 byte|int32_t||
|jointList|4 byte|int32_t||
|jointCount|4 byte|int32_t||

#### JOIN
JOIN is short for joint, and it's data unit is dotB2Joint.
|Data|Length|C++ type|default value|
|----|----|----|----|
|type|4 bytes|int32_t|0(e_unknownJoint)|
|bodyA|4 bytes|int32_t||
|bodyB|4 bytes|int32_t||
|collideConnected|1 bytes|bool|false|
|para|4 bytes|int32_t||
|userData|8 bytes|uint64_t||

#### BODY
BODY, the chunk data of which is an array of it's data unit dotB2Body.
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
|userData|8 bytes|uint64_t||

#### FXTR
FXTR is short for Fixture, and it's data unit is dotB2Fixture.
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
|shape_type|4 bytes|int32_t||
|shape_radius|4 bytes|float32_t||
|shape_vecList|4 bytes|int32_t||
|shape_vecCount|4 bytes|int32_t||
|userData|8 bytes|uint64_t||

#### VECT
VECT, short for Vector, stores an array of it's data unit, float32_t.
|Data|Length|C++ type|default value|
|----|----|----|----|
|[shape_vec, joint_para, any]|4 bytes|float32_t||


