#pragma once

#include "box2d/box2d.h"
#include "db2_structure.h"
#include "db2_cson.h"
#include "db2_key.h"

class db2Decoder
{
public:
    dotBox2d *db2{nullptr};
    b2World *b2w{nullptr};

    // db2Decoder();
    ~db2Decoder();

    auto decode() -> void;
    static auto Decode_World(db2World &db2w, b2Vec2 &gravity) -> void;
    static auto Decode_Body(db2Body &db2b, b2BodyDef &b2bdef) -> void;
    static auto Decode_Fixture(db2Fixture &db2f, b2FixtureDef &b2fdef) -> void;
    static auto Decode_Shpae(db2Shape &db2s, b2Shape *&p_b2s) -> void;
    static auto Decode_Joint(db2Joint &db2j, b2JointDef *&p_b2jdef) -> void;

    auto encode() -> void;
    static auto Encode_World(b2World &b2w, db2World &db2w) -> void;
    static auto Encode_Body(b2Body &b2b, db2Body &db2b) -> void;
    static auto Encode_Fixture(b2Fixture &b2f, db2Fixture &db2f) -> void;
    static auto Encode_Shpae(b2Shape &b2s, db2Shape &db2s) -> void;
    static auto Encode_Joint(b2Joint &b2j, db2Joint &db2j) -> void;
};