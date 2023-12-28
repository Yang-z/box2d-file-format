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

    auto &db2ws = this->db2->chunk<dotB2Wrold>(db2ChunkType::WRLD);
    auto &db2js = this->db2->chunk<dotB2Joint>(db2ChunkType::JINT);
    auto &db2bs = this->db2->chunk<dotB2Body>(db2ChunkType::BODY);
    auto &db2fs = this->db2->chunk<dotB2Fixture>(db2ChunkType::FXTR);

    auto &db2sxs = this->db2->chunk<float32_t>(db2ChunkType::SHpX);
    auto &db2jxs = this->db2->chunk<float32_t>(db2ChunkType::JInX);

    /*world*/
    auto &db2w = db2ws[0];
    b2Vec2 gravity{db2w.gravity_x, db2w.gravity_y};
    this->b2w = new b2World{gravity};

    /*body*/
    for (auto i = db2w.bodyList; i < db2w.bodyList + db2w.bodyCount; ++i)
    {
        auto &db2b = db2bs[i];

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
            dotB2Fixture &db2f = db2fs[j];
            b2FixtureDef b2fdef{};

            b2fdef.friction = db2f.friction;
            b2fdef.restitution = db2f.restitution;
            b2fdef.restitutionThreshold = db2f.restitutionThreshold;
            b2fdef.density = db2f.density;
            b2fdef.isSensor = db2f.isSensor;

            b2fdef.filter.categoryBits = db2f.filter_categoryBits;
            b2fdef.filter.maskBits = db2f.filter_maskBits;
            b2fdef.filter.groupIndex = db2f.filter_groupIndex;

            auto p = db2f.shape_extend;

            // auto shape_extCount = db2sxs[p++]<int32_t>;
            auto shape_extCount = db2sxs.operator[]<int32_t>(p++); // what the fork?!

            switch ((b2Shape::Type)db2f.shape_type)
            {

            case b2Shape::Type::e_circle:
            {
                assert(shape_extCount == 2);
                auto shape = b2CircleShape();
                shape.m_radius = db2f.shape_radius;

                shape.m_p = {db2sxs[p++], db2sxs[p++]};

                b2fdef.shape = &shape; // The shape will be cloned
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_edge:
            {
                assert(shape_extCount == 9);

                auto shape = b2EdgeShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                shape.m_vertex0 = {db2sxs[p++], db2sxs[p++]};
                shape.m_vertex1 = {db2sxs[p++], db2sxs[p++]};
                shape.m_vertex2 = {db2sxs[p++], db2sxs[p++]};
                shape.m_vertex3 = {db2sxs[p++], db2sxs[p++]};
                shape.m_oneSided = (bool)db2sxs[p++];

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_polygon:
            {
                assert(shape_extCount >= 6);

                auto shape = b2PolygonShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                auto points = (b2Vec2 *)(&(db2sxs[p]));
                auto count = shape_extCount / 2;
                shape.Set(points, count);

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_chain:
            {
                assert(shape_extCount >= 8);

                auto shape = b2ChainShape();
                shape.m_radius = db2f.shape_radius;

                /*
                CreateChain or CreateLoop?
                Since we record the "fusion point", m_prevVertex and m_nextVertex,
                they are the same.
                */
                auto points = (b2Vec2 *)(&(db2sxs[p]));
                auto count = shape_extCount / 2 - 2;
                shape.CreateChain(
                    points,
                    count,
                    {db2sxs[p + count * 2 + 0], db2sxs[p + count * 2 + 1]},
                    {db2sxs[p + count * 2 + 2], db2sxs[p + count * 2 + 3]});

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;
            }
        }
    }

    /*joint*/
    for (auto i = db2w.jointList; i < db2w.jointList + db2w.jointCount; ++i)
    {
        auto &db2j = db2js[i];

        auto p = db2j.extend;
        /*joint_extCount*/ const auto joint_extCount = db2jxs.operator[]<int32_t>(p++);

        switch (b2JointType(db2j.type))
        {
        case b2JointType::e_revoluteJoint:
        {
            b2RevoluteJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 11);

            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.referenceAngle = db2jxs[p++];
            b2jdef.enableLimit = (bool)db2jxs[p++];
            b2jdef.lowerAngle = db2jxs[p++];
            b2jdef.upperAngle = db2jxs[p++];
            b2jdef.enableMotor = (bool)db2jxs[p++];
            b2jdef.motorSpeed = db2jxs[p++];
            b2jdef.maxMotorTorque = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            b2PrismaticJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 13);

            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAxisA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.referenceAngle = db2jxs[p++];
            b2jdef.enableLimit = (bool)db2jxs[p++];
            b2jdef.lowerTranslation = db2jxs[p++];
            b2jdef.upperTranslation = db2jxs[p++];
            b2jdef.enableMotor = (bool)db2jxs[p++];
            b2jdef.maxMotorForce = db2jxs[p++];
            b2jdef.motorSpeed = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            b2DistanceJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 9);

            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.length = db2jxs[p++];
            b2jdef.minLength = db2jxs[p++];
            b2jdef.maxLength = db2jxs[p++];
            b2jdef.stiffness = db2jxs[p++];
            b2jdef.damping = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            b2PulleyJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 11);

            b2jdef.groundAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.groundAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.lengthA = db2jxs[p++];
            b2jdef.lengthB = db2jxs[p++];
            b2jdef.ratio = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            b2MouseJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 5);

            b2jdef.target = {db2jxs[p++], db2jxs[p++]};
            b2jdef.maxForce = db2jxs[p++];
            b2jdef.stiffness = db2jxs[p++];
            b2jdef.damping = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_gearJoint:
        {
            b2GearJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 3);

            b2jdef.joint1 = (b2Joint *)db2js[(int)db2jxs[p++]].userData;
            b2jdef.joint2 = (b2Joint *)db2js[(int)db2jxs[p++]].userData;
            b2jdef.ratio = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            b2WheelJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 14);

            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAxisA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.enableLimit = (bool)db2jxs[p++];
            b2jdef.lowerTranslation = db2jxs[p++];
            b2jdef.upperTranslation = db2jxs[p++];
            b2jdef.enableMotor = (bool)db2jxs[p++];
            b2jdef.maxMotorTorque = db2jxs[p++];
            b2jdef.motorSpeed = db2jxs[p++];
            b2jdef.stiffness = db2jxs[p++];
            b2jdef.damping = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_weldJoint:
        {
            b2WeldJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 7);

            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.referenceAngle = db2jxs[p++];
            b2jdef.stiffness = db2jxs[p++];
            b2jdef.damping = db2jxs[p++];

            b2jdef.userData.pointer = (uintptr_t)i;

            db2j.userData = (uint64_t)b2w->CreateJoint(&b2jdef);
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            b2FrictionJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 6);

            b2jdef.localAnchorA = {db2jxs[p++], db2jxs[p++]};
            b2jdef.localAnchorB = {db2jxs[p++], db2jxs[p++]};
            b2jdef.maxForce = db2jxs[p++];
            b2jdef.maxTorque = db2jxs[p++];

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
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(joint_extCount == 6);

            b2jdef.linearOffset = {db2jxs[p++], db2jxs[p++]};
            b2jdef.angularOffset = db2jxs[p++];
            b2jdef.maxForce = db2jxs[p++];
            b2jdef.maxTorque = db2jxs[p++];
            b2jdef.correctionFactor = db2jxs[p++];

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
    auto &db2is = _db2->chunk<dotB2Info>(db2ChunkType::INFO);
    auto &db2ws = _db2->chunk<dotB2Wrold>(db2ChunkType::WRLD);
    auto &db2js = _db2->chunk<dotB2Joint>(db2ChunkType::JINT);
    auto &db2bs = _db2->chunk<dotB2Body>(db2ChunkType::BODY);
    auto &db2fs = _db2->chunk<dotB2Fixture>(db2ChunkType::FXTR);

    auto &db2sxs = _db2->chunk<float32_t>(db2ChunkType::SHpX);
    auto &db2jxs = _db2->chunk<float32_t>(db2ChunkType::JInX);

    /*info*/
    // constructs an element in-place at the end
    db2is.emplace_back(); // default

    /*world*/
    db2ws.emplace_back(
        this->b2w->GetGravity().x,
        this->b2w->GetGravity().y,

        db2bs.size(),
        this->b2w->GetBodyCount(),

        db2js.size(),
        this->b2w->GetJointCount());

    /*body*/
    db2DynArray<b2Body *> bodies{};
    bodies.reserve(this->b2w->GetBodyCount(), false);
    for (auto b2b = this->b2w->GetBodyList(); b2b; b2b = b2b->GetNext())
        bodies.push(b2b);

    for (int i = bodies.size() - 1; i >= 0; --i) // reverse order
    {
        auto b2b = bodies[i];

        db2bs.emplace_back(
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

            db2fs.size(),
            0, //

            /*extraDict*/ -1,

            (uint64_t)b2b);
        b2b->GetUserData().pointer = (uintptr_t)i; // db2bs.size() - 1;

        /*fixture*/
        db2DynArray<b2Fixture *> fixtures{};
        // fixtures.reserve(b2b->GetFixtureCount(), false); // no way to get fixture count directly
        for (auto b2f = b2b->GetFixtureList(); b2f; b2f = b2f->GetNext())
            fixtures.push(b2f);

        db2bs[-1].fixtureCount = fixtures.size();

        for (int i = fixtures.size() - 1; i >= 0; --i) // reverse order
        {
            auto b2f = fixtures[i];

            db2fs.emplace_back(
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

                db2sxs.size(),
                // 0, //

                (uint64_t)b2f);
            b2f->GetUserData().pointer = (uintptr_t)db2fs.size() - 1;

            /*shape_extCount*/ db2sxs.emplace_back(0);

            /*shape*/
            auto b2s = b2f->GetShape();
            switch (b2s->GetType())
            {
            case b2Shape::e_circle:
            {
                auto b2s_c = (b2CircleShape *)b2s;

                db2sxs.reserve(db2sxs.size() + 1 * 2);

                db2sxs.emplace_back(b2s_c->m_p.x);
                db2sxs.emplace_back(b2s_c->m_p.y);
            }
            break;

            case b2Shape::e_edge:
            {
                auto b2s_e = (b2EdgeShape *)b2s;

                db2sxs.reserve(db2sxs.size() + 4 * 2 + 1);

                db2sxs.emplace_back(b2s_e->m_vertex0.x);
                db2sxs.emplace_back(b2s_e->m_vertex0.y);
                db2sxs.emplace_back(b2s_e->m_vertex1.x);
                db2sxs.emplace_back(b2s_e->m_vertex1.y);
                db2sxs.emplace_back(b2s_e->m_vertex2.x);
                db2sxs.emplace_back(b2s_e->m_vertex2.y);
                db2sxs.emplace_back(b2s_e->m_vertex3.x);
                db2sxs.emplace_back(b2s_e->m_vertex3.y);

                db2sxs.emplace_back((float32_t)b2s_e->m_oneSided);
            }
            break;

            case b2Shape::e_polygon:
            {
                auto b2s_p = (b2PolygonShape *)b2s;

                db2sxs.reserve(db2sxs.size() + b2s_p->m_count * 2);

                for (int i = 0; i < b2s_p->m_count; i++)
                {
                    db2sxs.emplace_back(b2s_p->m_vertices[i].x);
                    db2sxs.emplace_back(b2s_p->m_vertices[i].y);
                }
            }
            break;

            case b2Shape::e_chain:
            {
                auto b2s_chain = (b2ChainShape *)b2s;

                db2sxs.reserve(db2sxs.size() + b2s_chain->m_count * 2 + 2 * 2);

                for (int i = 0; i < b2s_chain->m_count; i++)
                {
                    db2sxs.emplace_back(b2s_chain->m_vertices[i].x);
                    db2sxs.emplace_back(b2s_chain->m_vertices[i].y);
                }

                db2sxs.emplace_back(b2s_chain->m_prevVertex.x);
                db2sxs.emplace_back(b2s_chain->m_prevVertex.y);
                db2sxs.emplace_back(b2s_chain->m_nextVertex.x);
                db2sxs.emplace_back(b2s_chain->m_nextVertex.y);
            }
            break;
            }

            auto &shape_extCount = db2sxs.operator[]<int32_t>(db2fs[-1].shape_extend);
            shape_extCount = db2sxs.size() - db2fs[-1].shape_extend - 1;
        }
    }

    /*joint*/
    db2DynArray<b2Joint *> joints{};
    joints.reserve(this->b2w->GetJointCount(), false);
    for (auto b2j = this->b2w->GetJointList(); b2j; b2j = b2j->GetNext())
        joints.push(b2j);

    for (int i = joints.size() - 1; i >= 0; --i) // reverse order
    {
        auto &b2j = joints[i];

        db2js.emplace_back(
            b2j->GetType(),
            int32_t(b2j->GetBodyA()->GetUserData().pointer),
            int32_t(b2j->GetBodyB()->GetUserData().pointer),
            b2j->GetCollideConnected(),

            db2jxs.size(),

            (uint64_t)b2j);
        b2j->GetUserData().pointer = (uintptr_t)i; // db2js.size() - 1;

        /*joint_extCount*/ db2jxs.emplace_back(0);

        switch (b2j->GetType())
        {
        case b2JointType::e_revoluteJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 11;

            auto b2j_r = (b2RevoluteJoint *)b2j;
            db2jxs.emplace_back(b2j_r->GetLocalAnchorA().x);
            db2jxs.emplace_back(b2j_r->GetLocalAnchorA().y);
            db2jxs.emplace_back(b2j_r->GetLocalAnchorB().x);
            db2jxs.emplace_back(b2j_r->GetLocalAnchorB().y);
            db2jxs.emplace_back(b2j_r->GetReferenceAngle());
            db2jxs.emplace_back((float32_t)b2j_r->IsLimitEnabled());
            db2jxs.emplace_back(b2j_r->GetLowerLimit());
            db2jxs.emplace_back(b2j_r->GetUpperLimit());
            db2jxs.emplace_back((float32_t)b2j_r->IsMotorEnabled());
            db2jxs.emplace_back(b2j_r->GetMotorSpeed());
            db2jxs.emplace_back(b2j_r->GetMaxMotorTorque());
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 13;

            auto b2j_p = (b2PrismaticJoint *)b2j;
            db2jxs.emplace_back(b2j_p->GetLocalAnchorA().x);
            db2jxs.emplace_back(b2j_p->GetLocalAnchorA().y);
            db2jxs.emplace_back(b2j_p->GetLocalAnchorB().x);
            db2jxs.emplace_back(b2j_p->GetLocalAnchorB().y);
            db2jxs.emplace_back(b2j_p->GetLocalAxisA().x);
            db2jxs.emplace_back(b2j_p->GetLocalAxisA().y);
            db2jxs.emplace_back(b2j_p->GetReferenceAngle());
            db2jxs.emplace_back((float32_t)b2j_p->IsLimitEnabled());
            db2jxs.emplace_back(b2j_p->GetLowerLimit());
            db2jxs.emplace_back(b2j_p->GetUpperLimit());
            db2jxs.emplace_back((float32_t)b2j_p->IsMotorEnabled());
            db2jxs.emplace_back(b2j_p->GetMaxMotorForce());
            db2jxs.emplace_back(b2j_p->GetMotorSpeed());
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 9;

            auto b2j_d = (b2DistanceJoint *)b2j;
            db2jxs.emplace_back(b2j_d->GetLocalAnchorA().x);
            db2jxs.emplace_back(b2j_d->GetLocalAnchorA().y);
            db2jxs.emplace_back(b2j_d->GetLocalAnchorB().x);
            db2jxs.emplace_back(b2j_d->GetLocalAnchorB().y);
            db2jxs.emplace_back(b2j_d->GetLength());
            db2jxs.emplace_back(b2j_d->GetMinLength());
            db2jxs.emplace_back(b2j_d->GetMaxLength());
            db2jxs.emplace_back(b2j_d->GetStiffness());
            db2jxs.emplace_back(b2j_d->GetDamping());
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 11;

            auto b2j_p = (b2PulleyJoint *)b2j;
            db2jxs.emplace_back(b2j_p->GetGroundAnchorA().x);
            db2jxs.emplace_back(b2j_p->GetGroundAnchorA().y);
            db2jxs.emplace_back(b2j_p->GetGroundAnchorB().x);
            db2jxs.emplace_back(b2j_p->GetGroundAnchorB().y);

            auto localAnchorA = b2j_p->GetBodyA()->GetLocalPoint(b2j_p->GetAnchorA());
            db2jxs.emplace_back(localAnchorA.x);
            db2jxs.emplace_back(localAnchorA.y);
            auto localAnchorB = b2j_p->GetBodyB()->GetLocalPoint(b2j_p->GetAnchorB());
            db2jxs.emplace_back(localAnchorB.x);
            db2jxs.emplace_back(localAnchorB.y);

            db2jxs.emplace_back(b2j_p->GetLengthA());
            db2jxs.emplace_back(b2j_p->GetLengthB());
            db2jxs.emplace_back(b2j_p->GetRatio());
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 5;

            auto b2j_m = (b2MouseJoint *)b2j;
            db2jxs.emplace_back(b2j_m->GetTarget().x);
            db2jxs.emplace_back(b2j_m->GetTarget().y);
            db2jxs.emplace_back(b2j_m->GetMaxForce());
            db2jxs.emplace_back(b2j_m->GetStiffness());
            db2jxs.emplace_back(b2j_m->GetDamping());
        }
        break;

        case b2JointType::e_gearJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 3;

            auto b2j_g = (b2GearJoint *)b2j;
            db2jxs.emplace_back((float32_t)b2j_g->GetJoint1()->GetUserData().pointer);
            db2jxs.emplace_back((float32_t)b2j_g->GetJoint2()->GetUserData().pointer);
            db2jxs.emplace_back(b2j_g->GetRatio());
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 14;

            auto b2j_w = (b2WheelJoint *)b2j;
            db2jxs.emplace_back(b2j_w->GetLocalAnchorA().x);
            db2jxs.emplace_back(b2j_w->GetLocalAnchorA().y);
            db2jxs.emplace_back(b2j_w->GetLocalAnchorB().x);
            db2jxs.emplace_back(b2j_w->GetLocalAnchorB().y);
            db2jxs.emplace_back(b2j_w->GetLocalAxisA().x);
            db2jxs.emplace_back(b2j_w->GetLocalAxisA().y);
            db2jxs.emplace_back((float32_t)b2j_w->IsLimitEnabled());
            db2jxs.emplace_back(b2j_w->GetLowerLimit());
            db2jxs.emplace_back(b2j_w->GetUpperLimit());
            db2jxs.emplace_back((float32_t)b2j_w->IsMotorEnabled());
            db2jxs.emplace_back(b2j_w->GetMaxMotorTorque());
            db2jxs.emplace_back(b2j_w->GetMotorSpeed());
            db2jxs.emplace_back(b2j_w->GetStiffness());
            db2jxs.emplace_back(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_weldJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 7;

            auto b2j_w = (b2WeldJoint *)b2j;
            db2jxs.emplace_back(b2j_w->GetLocalAnchorA().x);
            db2jxs.emplace_back(b2j_w->GetLocalAnchorA().y);
            db2jxs.emplace_back(b2j_w->GetLocalAnchorB().x);
            db2jxs.emplace_back(b2j_w->GetLocalAnchorB().y);
            db2jxs.emplace_back(b2j_w->GetReferenceAngle());
            db2jxs.emplace_back(b2j_w->GetStiffness());
            db2jxs.emplace_back(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 6;

            auto b2j_f = (b2FrictionJoint *)b2j;
            db2jxs.emplace_back(b2j_f->GetLocalAnchorA().x);
            db2jxs.emplace_back(b2j_f->GetLocalAnchorA().y);
            db2jxs.emplace_back(b2j_f->GetLocalAnchorB().x);
            db2jxs.emplace_back(b2j_f->GetLocalAnchorB().y);
            db2jxs.emplace_back(b2j_f->GetMaxForce());
            db2jxs.emplace_back(b2j_f->GetMaxTorque());
        }
        break;

        case b2JointType::e_ropeJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 0;

            // auto b2j_r = (b2RopeJoint *)b2j;
        }
        break;

        case b2JointType::e_motorJoint:
        {
            /*joint_extCount*/ db2jxs.operator[]<int32_t>(db2js[-1].extend) = 6;

            auto b2j_m = (b2MotorJoint *)b2j;
            db2jxs.emplace_back(b2j_m->GetLinearOffset().x);
            db2jxs.emplace_back(b2j_m->GetLinearOffset().y);
            db2jxs.emplace_back(b2j_m->GetAngularOffset());
            db2jxs.emplace_back(b2j_m->GetMaxForce());
            db2jxs.emplace_back(b2j_m->GetMaxTorque());
            db2jxs.emplace_back(b2j_m->GetCorrectionFactor());
        }
        break;

        default:
            break;
        }
    }

    delete this->db2;
    this->db2 = _db2;
}