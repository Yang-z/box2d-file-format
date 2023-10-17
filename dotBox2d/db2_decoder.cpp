#include "db2_decoder.h"

#include <assert.h>

dotB2Decoder::~dotB2Decoder()
{
    delete this->db2;
    delete this->b2w;
}

auto dotB2Decoder::decode() -> void
{
    if (!this->db2)
        return;

    auto &db2v = db2->chunks.vector;

    auto &db2w = db2->chunks.world[0];
    b2Vec2 gravity{db2w.gravity_x, db2w.gravity_y};
    this->b2w = new b2World{gravity};

    /*body*/
    for (auto i = db2w.bodyList; i <= db2w.bodyList + db2w.bodyCount - 1; ++i)
    {
        auto &db2b = db2->chunks.body[i];

        b2BodyDef b2bdef{};
        /*
        We can't control the packsize of libbox2d,
        so we cna't use ::memcpy safely.
        Instead we set each field manually.
        */
        b2bdef.type = b2BodyType(db2b.type);
        b2bdef.position = {db2b.position_x, db2b.position_y};
        b2bdef.angle = db2b.angle;
        b2bdef.linearVelocity = {db2b.linearVelocity_x, db2b.linearVelocity_y};
        b2bdef.angularVelocity = db2b.angularVelocity;
        b2bdef.linearDamping = db2b.linearDamping;
        b2bdef.angularDamping = db2b.angularDamping;
        b2bdef.allowSleep = db2b.allowSleep;
        b2bdef.awake = db2b.awake;
        b2bdef.fixedRotation = db2b.fixedRotation;
        b2bdef.bullet = db2b.bullet;
        b2bdef.enabled = db2b.enabled;
        b2bdef.gravityScale = db2b.gravityScale;

        b2bdef.userData.pointer = (uintptr_t)i;

        auto *b2b = b2w->CreateBody(&b2bdef);
        db2b.userData = (uint64_t)b2b;

        /*fixture*/
        for (auto j = db2b.fixtureList; j <= db2b.fixtureList + db2b.fixtureCount - 1; ++j)
        {
            dotB2Fixture &db2f = db2->chunks.fixture[j];
            b2FixtureDef b2fdef{};

            b2fdef.friction = db2f.friction;
            b2fdef.restitution = db2f.restitution;
            b2fdef.restitutionThreshold = db2f.restitutionThreshold;
            b2fdef.density = db2f.density;
            b2fdef.isSensor = db2f.isSensor;

            b2fdef.filter.categoryBits = db2f.filter_categoryBits;
            b2fdef.filter.maskBits = db2f.filter_maskBits;
            b2fdef.filter.groupIndex = db2f.filter_groupIndex;

            auto p0 = db2f.shape_vecList;

            switch ((b2Shape::Type)db2f.shape_type)
            {

            case b2Shape::Type::e_circle:
            {
                assert(db2f.shape_vecCount >= 2);
                auto shape = b2CircleShape();
                shape.m_radius = db2f.shape_radius;

                shape.m_p = {db2v[p0++], db2v[p0++]};

                b2fdef.shape = &shape; // The shape will be cloned
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_edge:
            {
                assert(db2f.shape_vecCount >= 9);

                auto shape = b2EdgeShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                shape.m_vertex0 = {db2v[p0++], db2v[p0++]};
                shape.m_vertex1 = {db2v[p0++], db2v[p0++]};
                shape.m_vertex2 = {db2v[p0++], db2v[p0++]};
                shape.m_vertex3 = {db2v[p0++], db2v[p0++]};
                shape.m_oneSided = (bool)db2v[p0++];

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_polygon:
            {
                auto shape = b2PolygonShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                auto points = (b2Vec2 *)(&(db2v[p0]));
                auto count = db2f.shape_vecCount / 2 - 1;
                shape.Set(points, count);

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_chain:
            {
                assert(db2f.shape_vecCount >= 8);

                auto shape = b2ChainShape();
                shape.m_radius = db2f.shape_radius;

                /*
                CreateChain or CreateLoop?
                Since we record the "fusion point", m_prevVertex and m_nextVertex,
                they are the same.
                */
                auto points = (b2Vec2 *)(&(db2v[p0]));
                auto count = db2f.shape_vecCount / 2 - 2;
                shape.CreateChain(
                    points,
                    count,
                    {db2v[p0 + count * 2 + 0], db2v[p0 + count * 2 + 1]},
                    {db2v[p0 + count * 2 + 2], db2v[p0 + count * 2 + 3]});

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;
            }
        }
    }

    /*joint*/
    for (auto i = db2w.jointList; i <= db2w.jointList + db2w.jointCount - 1; ++i)
    {
        auto &db2j = db2->chunks.joint[i];

        switch (b2JointType(db2j.type))
        {
        case b2JointType::e_revoluteJoint:
        {
            b2RevoluteJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 11;

            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.referenceAngle = db2v[p++];
            b2jdef.enableLimit = (bool)db2v[p++];
            b2jdef.lowerAngle = db2v[p++];
            b2jdef.upperAngle = db2v[p++];
            b2jdef.enableMotor = (bool)db2v[p++];
            b2jdef.motorSpeed = db2v[p++];
            b2jdef.maxMotorTorque = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            b2PrismaticJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 13;

            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.localAxisA = {db2v[p++], db2v[p++]};
            b2jdef.referenceAngle = db2v[p++];
            b2jdef.enableLimit = (bool)db2v[p++];
            b2jdef.lowerTranslation = db2v[p++];
            b2jdef.upperTranslation = db2v[p++];
            b2jdef.enableMotor = (bool)db2v[p++];
            b2jdef.maxMotorForce = db2v[p++];
            b2jdef.motorSpeed = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            b2DistanceJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 9;

            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.length = db2v[p++];
            b2jdef.minLength = db2v[p++];
            b2jdef.maxLength = db2v[p++];
            b2jdef.stiffness = db2v[p++];
            b2jdef.damping = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            b2PulleyJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 11;

            b2jdef.groundAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.groundAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.lengthA = db2v[p++];
            b2jdef.lengthB = db2v[p++];
            b2jdef.ratio = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            b2MouseJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 5;

            b2jdef.target = {db2v[p++], db2v[p++]};
            b2jdef.maxForce = db2v[p++];
            b2jdef.stiffness = db2v[p++];
            b2jdef.damping = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_gearJoint:
        {
            b2GearJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 3;

            b2jdef.joint1 = (b2Joint *)this->db2->chunks.joint[(int)db2v[p++]].userData;
            b2jdef.joint2 = (b2Joint *)this->db2->chunks.joint[(int)db2v[p++]].userData;
            b2jdef.ratio = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            b2WheelJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 14;

            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.localAxisA = {db2v[p++], db2v[p++]};
            b2jdef.enableLimit = (bool)db2v[p++];
            b2jdef.lowerTranslation = db2v[p++];
            b2jdef.upperTranslation = db2v[p++];
            b2jdef.enableMotor = (bool)db2v[p++];
            b2jdef.maxMotorTorque = db2v[p++];
            b2jdef.motorSpeed = db2v[p++];
            b2jdef.stiffness = db2v[p++];
            b2jdef.damping = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_weldJoint:
        {
            b2WeldJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 7;

            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.referenceAngle = db2v[p++];
            b2jdef.stiffness = db2v[p++];
            b2jdef.damping = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            b2FrictionJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 6;

            b2jdef.localAnchorA = {db2v[p++], db2v[p++]};
            b2jdef.localAnchorB = {db2v[p++], db2v[p++]};
            b2jdef.maxForce = db2v[p++];
            b2jdef.maxTorque = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_ropeJoint:
        {
            // b2RopeJointDef b2jdef;
        }
        break;

        case b2JointType::e_motorJoint:
        {
            b2MotorJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)this->db2->chunks.body[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)this->db2->chunks.body[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            auto p = db2j.para;
            const auto paraCount = 6;

            b2jdef.linearOffset = {db2v[p++], db2v[p++]};
            b2jdef.angularOffset = db2v[p++];
            b2jdef.maxForce = db2v[p++];
            b2jdef.maxTorque = db2v[p++];
            b2jdef.correctionFactor = db2v[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        default:
            break;
        }
    }
}

auto dotB2Decoder::encode() -> void
{
    if (!this->b2w)
        return;

    auto _db2 = new dotBox2d();

    /*info*/
    // constructs an element in-place at the end
    _db2->chunks.info.emplace_back(); // default

    /*world*/
    _db2->chunks.world.emplace_back(
        this->b2w->GetGravity().x,
        this->b2w->GetGravity().y,

        _db2->chunks.body.size,
        this->b2w->GetBodyCount(),

        _db2->chunks.joint.size,
        this->b2w->GetJointCount());

    /*body*/
    db2Container<b2Body *> bodies{};
    bodies.reserve(this->b2w->GetBodyCount(), false);
    for (auto b2b = this->b2w->GetBodyList(); b2b; b2b = b2b->GetNext())
        bodies.push(b2b);

    for (int i = bodies.size - 1; i >= 0; --i) // reverse order
    {
        auto b2b = bodies[i];

        _db2->chunks.body.emplace_back(
            b2b->GetType(),
            b2b->GetPosition().x,
            b2b->GetPosition().y,
            b2b->GetAngle(),
            b2b->GetLinearVelocity().x,
            b2b->GetLinearVelocity().y,
            b2b->GetAngularVelocity(),
            b2b->GetLinearDamping(),
            b2b->GetAngularDamping(),
            b2b->IsSleepingAllowed(),
            b2b->IsAwake(),
            b2b->IsFixedRotation(),
            b2b->IsBullet(),
            b2b->IsEnabled(),

            b2b->GetGravityScale(),

            _db2->chunks.fixture.size,
            0, //

            (uint64_t)b2b);
        b2b->GetUserData().pointer = (uintptr_t)_db2->chunks.body.size - 1;

        /*fixture*/
        db2Container<b2Fixture *> fixtures{};
        // fixtures.reserve(b2b->GetFixtureCount(), false); // no way to get fixture count directly
        for (auto b2f = b2b->GetFixtureList(); b2f; b2f = b2f->GetNext())
            fixtures.push(b2f);

        _db2->chunks.body[-1].fixtureCount = fixtures.size;

        for (int i = fixtures.size - 1; i >= 0; --i) // reverse order
        {
            auto b2f = fixtures[i];

            _db2->chunks.fixture.emplace_back(
                b2f->GetFriction(),
                b2f->GetRestitution(),
                b2f->GetRestitutionThreshold(),
                b2f->GetDensity(),
                b2f->IsSensor(),

                /*filter*/
                b2f->GetFilterData().categoryBits,
                b2f->GetFilterData().maskBits,
                b2f->GetFilterData().groupIndex,

                /*shape*/
                b2f->GetShape()->GetType(),
                b2f->GetShape()->m_radius,

                _db2->chunks.vector.size,
                0, //

                (uint64_t)b2f);
            b2f->GetUserData().pointer = (uintptr_t)_db2->chunks.fixture.size - 1;

            /*shape*/

            auto b2s = b2f->GetShape();
            switch (b2s->GetType())
            {
            case b2Shape::e_circle:
            {
                auto b2s_c = (b2CircleShape *)b2s;

                _db2->chunks.vector.reserve(_db2->chunks.vector.size + 1 * 2);

                _db2->chunks.vector.emplace_back(b2s_c->m_p.x);
                _db2->chunks.vector.emplace_back(b2s_c->m_p.y);
            }
            break;

            case b2Shape::e_edge:
            {
                auto b2s_e = (b2EdgeShape *)b2s;

                _db2->chunks.vector.reserve(_db2->chunks.vector.size + 4 * 2 + 1);

                _db2->chunks.vector.emplace_back(b2s_e->m_vertex0.x);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex0.y);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex1.x);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex1.y);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex2.x);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex2.y);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex3.x);
                _db2->chunks.vector.emplace_back(b2s_e->m_vertex3.y);

                _db2->chunks.vector.emplace_back((float32_t)b2s_e->m_oneSided);
            }
            break;

            case b2Shape::e_polygon:
            {
                auto b2s_p = (b2PolygonShape *)b2s;

                _db2->chunks.vector.reserve(_db2->chunks.vector.size + b2s_p->m_count * 2);

                for (int i = 0; i < b2s_p->m_count; i++)
                {
                    _db2->chunks.vector.emplace_back(b2s_p->m_vertices[i].x);
                    _db2->chunks.vector.emplace_back(b2s_p->m_vertices[i].y);
                }
            }
            break;

            case b2Shape::e_chain:
            {
                auto b2s_chain = (b2ChainShape *)b2s;

                _db2->chunks.vector.reserve(_db2->chunks.vector.size + b2s_chain->m_count * 2 + 2 * 2);

                for (int i = 0; i < b2s_chain->m_count; i++)
                {
                    _db2->chunks.vector.emplace_back(b2s_chain->m_vertices[i].x);
                    _db2->chunks.vector.emplace_back(b2s_chain->m_vertices[i].y);
                }

                _db2->chunks.vector.emplace_back(b2s_chain->m_prevVertex.x);
                _db2->chunks.vector.emplace_back(b2s_chain->m_prevVertex.y);
                _db2->chunks.vector.emplace_back(b2s_chain->m_nextVertex.x);
                _db2->chunks.vector.emplace_back(b2s_chain->m_nextVertex.y);
            }
            break;
            }
            _db2->chunks.fixture[-1].shape_vecCount = _db2->chunks.vector.size - _db2->chunks.fixture[-1].shape_vecList;
        }
    }

    /*joint*/
    db2Container<b2Joint *> joints{};
    joints.reserve(this->b2w->GetJointCount(), false);
    for (auto b2j = this->b2w->GetJointList(); b2j; b2j = b2j->GetNext())
        joints.push(b2j);

    for (int i = joints.size - 1; i >= 0; --i) // reverse order
    {
        auto &b2j = joints[i];

        _db2->chunks.joint.emplace_back(
            b2j->GetType(),
            int32_t(b2j->GetBodyA()->GetUserData().pointer),
            int32_t(b2j->GetBodyB()->GetUserData().pointer),
            b2j->GetCollideConnected(),

            _db2->chunks.vector.size,

            (uint64_t)b2j);
        b2j->GetUserData().pointer = (uintptr_t)_db2->chunks.joint.size - 1;

        switch (b2j->GetType())
        {
        case b2JointType::e_revoluteJoint:
        {
            auto b2j_r = (b2RevoluteJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_r->GetLocalAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_r->GetLocalAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_r->GetLocalAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_r->GetLocalAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_r->GetReferenceAngle());
            _db2->chunks.vector.emplace_back((float32_t)b2j_r->IsLimitEnabled());
            _db2->chunks.vector.emplace_back(b2j_r->GetLowerLimit());
            _db2->chunks.vector.emplace_back(b2j_r->GetUpperLimit());
            _db2->chunks.vector.emplace_back((float32_t)b2j_r->IsMotorEnabled());
            _db2->chunks.vector.emplace_back(b2j_r->GetMotorSpeed());
            _db2->chunks.vector.emplace_back(b2j_r->GetMaxMotorTorque());
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            auto b2j_p = (b2PrismaticJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_p->GetLocalAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_p->GetLocalAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_p->GetLocalAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_p->GetLocalAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_p->GetLocalAxisA().x);
            _db2->chunks.vector.emplace_back(b2j_p->GetLocalAxisA().y);
            _db2->chunks.vector.emplace_back(b2j_p->GetReferenceAngle());
            _db2->chunks.vector.emplace_back((float32_t)b2j_p->IsLimitEnabled());
            _db2->chunks.vector.emplace_back(b2j_p->GetLowerLimit());
            _db2->chunks.vector.emplace_back(b2j_p->GetUpperLimit());
            _db2->chunks.vector.emplace_back((float32_t)b2j_p->IsMotorEnabled());
            _db2->chunks.vector.emplace_back(b2j_p->GetMaxMotorForce());
            _db2->chunks.vector.emplace_back(b2j_p->GetMotorSpeed());
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            auto b2j_d = (b2DistanceJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_d->GetLocalAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_d->GetLocalAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_d->GetLocalAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_d->GetLocalAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_d->GetLength());
            _db2->chunks.vector.emplace_back(b2j_d->GetMinLength());
            _db2->chunks.vector.emplace_back(b2j_d->GetMaxLength());
            _db2->chunks.vector.emplace_back(b2j_d->GetStiffness());
            _db2->chunks.vector.emplace_back(b2j_d->GetDamping());
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            auto b2j_p = (b2PulleyJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_p->GetGroundAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_p->GetGroundAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_p->GetGroundAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_p->GetGroundAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_p->GetLengthA());
            _db2->chunks.vector.emplace_back(b2j_p->GetLengthB());
            _db2->chunks.vector.emplace_back(b2j_p->GetRatio());
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            auto b2j_m = (b2MouseJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_m->GetTarget().x);
            _db2->chunks.vector.emplace_back(b2j_m->GetTarget().y);
            _db2->chunks.vector.emplace_back(b2j_m->GetMaxForce());
            _db2->chunks.vector.emplace_back(b2j_m->GetStiffness());
            _db2->chunks.vector.emplace_back(b2j_m->GetDamping());
        }
        break;

        case b2JointType::e_gearJoint:
        {
            auto b2j_g = (b2GearJoint *)b2j;
            _db2->chunks.vector.emplace_back((float32_t)b2j_g->GetJoint1()->GetUserData().pointer);
            _db2->chunks.vector.emplace_back((float32_t)b2j_g->GetJoint2()->GetUserData().pointer);
            _db2->chunks.vector.emplace_back(b2j_g->GetRatio());
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            auto b2j_w = (b2WheelJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAxisA().x);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAxisA().y);
            _db2->chunks.vector.emplace_back((float32_t)b2j_w->IsLimitEnabled());
            _db2->chunks.vector.emplace_back(b2j_w->GetLowerLimit());
            _db2->chunks.vector.emplace_back(b2j_w->GetUpperLimit());
            _db2->chunks.vector.emplace_back((float32_t)b2j_w->IsMotorEnabled());
            _db2->chunks.vector.emplace_back(b2j_w->GetMaxMotorTorque());
            _db2->chunks.vector.emplace_back(b2j_w->GetMotorSpeed());
            _db2->chunks.vector.emplace_back(b2j_w->GetStiffness());
            _db2->chunks.vector.emplace_back(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_weldJoint:
        {
            auto b2j_w = (b2WeldJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_w->GetLocalAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_w->GetReferenceAngle());
            _db2->chunks.vector.emplace_back(b2j_w->GetStiffness());
            _db2->chunks.vector.emplace_back(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            auto b2j_f = (b2FrictionJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_f->GetLocalAnchorA().x);
            _db2->chunks.vector.emplace_back(b2j_f->GetLocalAnchorA().y);
            _db2->chunks.vector.emplace_back(b2j_f->GetLocalAnchorB().x);
            _db2->chunks.vector.emplace_back(b2j_f->GetLocalAnchorB().y);
            _db2->chunks.vector.emplace_back(b2j_f->GetMaxForce());
            _db2->chunks.vector.emplace_back(b2j_f->GetMaxTorque());
        }
        break;

        case b2JointType::e_ropeJoint:
        {
            // auto b2j_r = (b2RopeJoint *)b2j;
        }
        break;

        case b2JointType::e_motorJoint:
        {
            auto b2j_m = (b2MotorJoint *)b2j;
            _db2->chunks.vector.emplace_back(b2j_m->GetLinearOffset().x);
            _db2->chunks.vector.emplace_back(b2j_m->GetLinearOffset().y);
            _db2->chunks.vector.emplace_back(b2j_m->GetAngularOffset());
            _db2->chunks.vector.emplace_back(b2j_m->GetMaxForce());
            _db2->chunks.vector.emplace_back(b2j_m->GetMaxTorque());
            _db2->chunks.vector.emplace_back(b2j_m->GetCorrectionFactor());
        }
        break;

        default:
            break;
        }
    }

    delete this->db2;
    this->db2 = _db2;
}