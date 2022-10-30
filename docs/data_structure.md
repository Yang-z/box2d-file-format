# Data Structure
A BOX2D (or B2) file contains every piece of information needed to create a B2World. 

## Head
At the beginning of a BOX2D file, an 8-byte signature is presented, this design is borrowed form PNG file standard:

|value|purpose|
|----|----|
|B2|Has the high bit set to detect transmission systems that do not support 8-bit data and to reduce the chance that a text file is mistakenly interpreted as a B2 file, or vice versa.|
|42 32 00|In ASCII, the first two letters B2, allowing a person to identify the format easily if it is viewed in a text editor.|
|0D 0A|A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.|
|1A|A byte that stops display of the file under DOS when the command type has been used—the end-of-file character.|
|0A|A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.|

## Chunks

|Length     |Chunk type     |Chunk data     |CRC        |
|----       |----           |----           |----       |
|4 bytes    |4 bytes        |Length bytes	|4 bytes    |

### Chunk Types

|Chunk type|Date stored|
|----|----|
|INFO|dotBox2dInfo|
|WRLD|dotB2Wrold[]|
|BODY|dotB2Body[]|
|FXTR|dotB2Fixture[]|
|VECT|dotB2Vec2[]|

### Chunk data
#### INFO
INFO is short for dotBox2d information, and it stores the data defined by dotBox2dInfo. See:
|Data|Length|C++ type|default value|
|----|----|----|----|
|version.dotBox2d|1 byte *3|char * 3||
|version.box2d|1 byte *3|char * 3||
|count.world|4 bytes|int||
|count.body|4 bytes|int||
|count.fixture|4 bytes|int||
|count.joint|4 bytes|int||
|count.vec2|4 bytes|int||

#### WRLD
WRLD, short for World, it's data unit is dotB2Wrold::raw.
|Data|Length|C++ type|default value|
|----|----|----|----|
|gravity.x|4 bytes|float||
|gravity.y|4 bytes|float||
|_bodies.start|4 byte|int||
|_bodies.start|4 byte|int||

#### BODY
BODY, the chunk data of which is an array of it's data unit dotB2Body::raw.
|Data|Length|C++ type|default value|
|----|----|----|----|
|type|4 bytes|int|0|
|position.x|4 bytes|float|0.0f|
|position.y|4 bytes|float|0.0f|
|angle|4 bytes|float|0.0f|
|linearVelocity.x|4 bytes|float|0.0f|
|linearVelocity.y|4 bytes|float|0.0f|
|angularVelocity|4 bytes|float|0.0f|
|linearDamping|4 bytes|float|0.0f|
|angularDamping|4 bytes|float|0.0f|
|allowSleep|1/8 bytes|bool|true|
|awake|1/8 bytes|bool|true|
|fixedRotation|1/8 bytes|bool|false|
|bullet|1/8 bytes|bool|false|
|enabled|1/8 bytes|bool|true|
|gravityScale|4 bytes|float|1.0f|
|_fixtures.start|4 bytes|int||
|_fixtures.end|4 bytes|int||

#### FXTR
FXTR is short for Fixture, and it's data unit is dotB2Fixture::raw.
|Data|Length|C++ type|default value|
|----|----|----|----|
|friction|4 bytes|float|0.2f|
|restitution|4 bytes|float|0.0f|
|restitutionThreshold|4 bytes|float|1.0f|
|density|4 bytes|float|0.0f|
|isSensor|1 bytes|bool|0|
|filter.categoryBits|2 bytes|unsigned short|0x0001|
|filter.maskBits|2 bytes|unsigned short|0xFFFF|
|filter.groupIndex|2 bytes|signed short|0|
|shape.type|4 bytes|int||
|shape.radius|4 bytes|float||
|_shape.vec2s.start|4 bytes|int||
|_shape.vec2s.end|4 bytes|int||

#### VECT
VECT, short for Vector, stores an array of it's data unit, dotB2Vec2.
|Data|Length|C++ type|default value|
|----|----|----|----|
|x|4 bytes|float||
|y|4 bytes|float||


