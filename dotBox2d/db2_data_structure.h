#ifndef DB2_DATA_STRUCTURE_H
#define DB2_DATA_STRUCTURE_H

struct dotB2Vec2
{
    float x;
    float y;
};

struct dotRange
{
    int start;
    int end;
};

struct dotB2Fixture
{
    float friction{0.2f};
    float restitution{0.0f};
    float restitutionThreshold{1.0f};
    float density{0.0f};
    bool isSensor{false};

    struct
    {
        unsigned short categoryBits{0x0001};
        unsigned short maskBits{0xFFFF};
        signed short groupIndex{0};
    } filter;

    struct
    {
        int type;
        float radius;

        dotRange vec2s;

        // bool oneSided;?

    } shape;

    unsigned long long userData{0};
};

struct dotB2Body
{
    int type{0};
    dotB2Vec2 position{0.0f, 0.0f};
    float angle{0.0f};
    dotB2Vec2 linearVelocity{0.0f, 0.0f};
    float angularVelocity{0.0f};
    float linearDamping{0.0f};
    float angularDamping{0.0f};
    bool allowSleep{true};
    bool awake{true};
    bool fixedRotation{false};
    bool bullet{false};
    bool enabled{true};
    unsigned long long userData{0};
    float gravityScale{1.0f};

    dotRange fixtures;

};

struct dotB2Joint
{
    int type;
    /* data */
};

struct dotB2Wrold
{
    dotB2Vec2 gravity{0.0f, 0.0f};

    dotRange bodies;

};

struct dotBox2d
{
    const int head[8]{0xB2, 0x42, 0x32, 0x00, 0x0D, 0x0A, 0x1A, 0x0A};

    struct
    {
        const char dotBox2d[3];
        const char box2d[3];


    } version;

    int countWorld{0};
    int countbody{0};
    int countFixture{0};
    int countVec2{0};

    //int locB2World;
    //int locB2Body;
    //int locB2Fixture;
    //int locB2Vec2;

    dotB2Wrold *worlds{nullptr};
    dotB2Body *bodies{nullptr};
    dotB2Fixture *fixtures{nullptr};
    dotB2Vec2 *vec2s{nullptr};

    ~dotBox2d();

    auto load(char *filePath) -> void;
    auto save(char *filePath) -> void;
};

#endif //DB2_DATA_STRUCTURE_H