
struct dotBox2d
{
    const char fileFormat[6]{'.', 'b', 'o', 'x', '2', 'd'};
    short dotBox2dVersion[3];
    short box2dVersion[3];

    // int countB2World{1};
    int countB2Body;
    int countB2Fixture;
    int countB2Vec2;

    int locB2World;
    int locB2Body;
    int locB2Fixture;
    int locB2Vec2;
};

struct dotB2Wrold
{
    dotB2Vec2 gravity{0.0f, 0.0f};


};

struct dotB2Body
{
    struct
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
        float gravityScale{1.0f};

        struct
        {
            int start;
            int end;
        } fixtures;

    } raw;
};

struct dotB2Fixture
{
    struct
    {
        float friction{0.2f};
        float restitution{0.0f};
        float restitutionThreshold{1.0f};
        float density{0.0f};
        bool isSensor{false};

        void *userData{0};

        struct
        {
            short categoryBits{0x0001};
            short maskBits{0xFFFF};
            short groupIndex{0};
        } filter;

        struct
        {
            int type;
            int radius;

            struct
            {
                int start;
                int end;
            } vec2s;

            // bool oneSided;

        } shape;

    } raw;
};

struct dotB2Vec2
{
    float x;
    float y;
};

struct dotB2Joint
{
    int type;
    /* data */
};
