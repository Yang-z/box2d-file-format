
struct dotBox2D
{
    char box2d_version;

    dotB2Wrold *world;
};

struct dotB2Wrold
{
    float gravity_x{0.0f};
    float gravity_y{0.0f};

    int count_body;
    dotB2Body *bodys;

    int count_joint;
    dotB2Joint *joints;
};

struct dotB2Body
{
    int type{0};
    float position_x{0.0f};
    float position_y{0.0f};
    float angle{0.0f};
    float linearVelocity_x{0.0f};
    float linearVelocity_y{0.0f};
    float angularVelocity{0.0f};
    float linearDamping{0.0f};
    float angularDamping{0.0f};
    bool allowSleep{true};
    bool awake{true};
    bool fixedRotation{false};
    bool bullet{false};
    bool enabled{true};
    float gravityScale{1.0f};

    int count;
    dotB2Fixture *fixtures;
};

struct dotB2Fixture
{

    float friction{0.2f};
    float restitution{0.0f};
    float restitutionThreshold{1.0f};
    float density{0.0f};
    bool isSensor{false};
    short filter_categoryBits{0x0001};
    short filter_maskBits{0xFFFF};
    short filter_groupIndex{0};

    void *userData{0};

    dotB2Shape *shape;
};

struct dotB2Shape
{
    int type;
    int radius;
    bool oneSided;

    int count;
    float *points;
};

struct dotB2Joint
{
    int type;
    /* data */
};
