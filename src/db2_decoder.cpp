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
    auto &db2js = this->db2->chunk<dotB2Joint>(db2ChunkType::JOIN);
    auto &db2bs = this->db2->chunk<dotB2Body>(db2ChunkType::BODY);
    auto &db2fs = this->db2->chunk<dotB2Fixture>(db2ChunkType::FXTR);
    auto &db2vs = this->db2->chunk<float32_t>(db2ChunkType::VECT);

    /*world*/
    auto &db2w = db2ws[0];
    b2Vec2 gravity{db2w.gravity_x, db2w.gravity_y};
    this->b2w = new b2World{gravity};

    /*body*/
    for (auto i = db2w.bodyList; i <= db2w.bodyList + db2w.bodyCount - 1; ++i)
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

            auto p0 = db2f.shape_vecList;

            // auto shape_vecCount = db2vs[p0++]<int32_t>;
            auto shape_vecCount = db2vs.operator[]<int32_t>(p0++); // what the fork?!

            switch ((b2Shape::Type)db2f.shape_type)
            {

            case b2Shape::Type::e_circle:
            {
                assert(shape_vecCount == 2);
                auto shape = b2CircleShape();
                shape.m_radius = db2f.shape_radius;

                shape.m_p = {db2vs[p0++], db2vs[p0++]};

                b2fdef.shape = &shape; // The shape will be cloned
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_edge:
            {
                assert(shape_vecCount == 9);

                auto shape = b2EdgeShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                shape.m_vertex0 = {db2vs[p0++], db2vs[p0++]};
                shape.m_vertex1 = {db2vs[p0++], db2vs[p0++]};
                shape.m_vertex2 = {db2vs[p0++], db2vs[p0++]};
                shape.m_vertex3 = {db2vs[p0++], db2vs[p0++]};
                shape.m_oneSided = (bool)db2vs[p0++];

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_polygon:
            {
                assert(shape_vecCount >= 6);

                auto shape = b2PolygonShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                auto points = (b2Vec2 *)(&(db2vs[p0]));
                auto count = shape_vecCount / 2;
                shape.Set(points, count);

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)j;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_chain:
            {
                assert(shape_vecCount >= 8);

                auto shape = b2ChainShape();
                shape.m_radius = db2f.shape_radius;

                /*
                CreateChain or CreateLoop?
                Since we record the "fusion point", m_prevVertex and m_nextVertex,
                they are the same.
                */
                auto points = (b2Vec2 *)(&(db2vs[p0]));
                auto count = shape_vecCount / 2 - 2;
                shape.CreateChain(
                    points,
                    count,
                    {db2vs[p0 + count * 2 + 0], db2vs[p0 + count * 2 + 1]},
                    {db2vs[p0 + count * 2 + 2], db2vs[p0 + count * 2 + 3]});

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
        auto &db2j = db2js[i];

        auto p = db2j.para;
        /*paraCount*/ const auto paraCount = db2vs.operator[]<int32_t>(p++);

        switch (b2JointType(db2j.type))
        {
        case b2JointType::e_revoluteJoint:
        {
            b2RevoluteJointDef b2jdef;
            b2jdef.bodyA = (b2Body *)db2bs[db2j.bodyA].userData;
            b2jdef.bodyB = (b2Body *)db2bs[db2j.bodyB].userData;
            b2jdef.collideConnected = db2j.collideConnected;

            assert(paraCount == 11);

            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.referenceAngle = db2vs[p++];
            b2jdef.enableLimit = (bool)db2vs[p++];
            b2jdef.lowerAngle = db2vs[p++];
            b2jdef.upperAngle = db2vs[p++];
            b2jdef.enableMotor = (bool)db2vs[p++];
            b2jdef.motorSpeed = db2vs[p++];
            b2jdef.maxMotorTorque = db2vs[p++];

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

            assert(paraCount == 13);

            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.localAxisA = {db2vs[p++], db2vs[p++]};
            b2jdef.referenceAngle = db2vs[p++];
            b2jdef.enableLimit = (bool)db2vs[p++];
            b2jdef.lowerTranslation = db2vs[p++];
            b2jdef.upperTranslation = db2vs[p++];
            b2jdef.enableMotor = (bool)db2vs[p++];
            b2jdef.maxMotorForce = db2vs[p++];
            b2jdef.motorSpeed = db2vs[p++];

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

            assert(paraCount == 9);

            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.length = db2vs[p++];
            b2jdef.minLength = db2vs[p++];
            b2jdef.maxLength = db2vs[p++];
            b2jdef.stiffness = db2vs[p++];
            b2jdef.damping = db2vs[p++];

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

            assert(paraCount == 11);

            b2jdef.groundAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.groundAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.lengthA = db2vs[p++];
            b2jdef.lengthB = db2vs[p++];
            b2jdef.ratio = db2vs[p++];

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

            assert(paraCount == 5);

            b2jdef.target = {db2vs[p++], db2vs[p++]};
            b2jdef.maxForce = db2vs[p++];
            b2jdef.stiffness = db2vs[p++];
            b2jdef.damping = db2vs[p++];

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

            assert(paraCount == 3);

            b2jdef.joint1 = (b2Joint *)db2js[(int)db2vs[p++]].userData;
            b2jdef.joint2 = (b2Joint *)db2js[(int)db2vs[p++]].userData;
            b2jdef.ratio = db2vs[p++];

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

            assert(paraCount == 14);

            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.localAxisA = {db2vs[p++], db2vs[p++]};
            b2jdef.enableLimit = (bool)db2vs[p++];
            b2jdef.lowerTranslation = db2vs[p++];
            b2jdef.upperTranslation = db2vs[p++];
            b2jdef.enableMotor = (bool)db2vs[p++];
            b2jdef.maxMotorTorque = db2vs[p++];
            b2jdef.motorSpeed = db2vs[p++];
            b2jdef.stiffness = db2vs[p++];
            b2jdef.damping = db2vs[p++];

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

            assert(paraCount == 7);

            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.referenceAngle = db2vs[p++];
            b2jdef.stiffness = db2vs[p++];
            b2jdef.damping = db2vs[p++];

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

            assert(paraCount == 6);

            b2jdef.localAnchorA = {db2vs[p++], db2vs[p++]};
            b2jdef.localAnchorB = {db2vs[p++], db2vs[p++]};
            b2jdef.maxForce = db2vs[p++];
            b2jdef.maxTorque = db2vs[p++];

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

            assert(paraCount == 6);

            b2jdef.linearOffset = {db2vs[p++], db2vs[p++]};
            b2jdef.angularOffset = db2vs[p++];
            b2jdef.maxForce = db2vs[p++];
            b2jdef.maxTorque = db2vs[p++];
            b2jdef.correctionFactor = db2vs[p++];

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
    auto &db2js = _db2->chunk<dotB2Joint>(db2ChunkType::JOIN);
    auto &db2bs = _db2->chunk<dotB2Body>(db2ChunkType::BODY);
    auto &db2fs = _db2->chunk<dotB2Fixture>(db2ChunkType::FXTR);
    auto &db2vs = _db2->chunk<float32_t>(db2ChunkType::VECT);

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

            (uint64_t)b2b);
        b2b->GetUserData().pointer = (uintptr_t)db2bs.size() - 1;

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

                db2vs.size(),
                // 0, //

                (uint64_t)b2f);
            b2f->GetUserData().pointer = (uintptr_t)db2fs.size() - 1;

            /*shape_vecCount*/ db2vs.emplace_back(0);

            /*shape*/
            auto b2s = b2f->GetShape();
            switch (b2s->GetType())
            {
            case b2Shape::e_circle:
            {
                auto b2s_c = (b2CircleShape *)b2s;

                db2vs.reserve(db2vs.size() + 1 * 2);

                db2vs.emplace_back(b2s_c->m_p.x);
                db2vs.emplace_back(b2s_c->m_p.y);
            }
            break;

            case b2Shape::e_edge:
            {
                auto b2s_e = (b2EdgeShape *)b2s;

                db2vs.reserve(db2vs.size() + 4 * 2 + 1);

                db2vs.emplace_back(b2s_e->m_vertex0.x);
                db2vs.emplace_back(b2s_e->m_vertex0.y);
                db2vs.emplace_back(b2s_e->m_vertex1.x);
                db2vs.emplace_back(b2s_e->m_vertex1.y);
                db2vs.emplace_back(b2s_e->m_vertex2.x);
                db2vs.emplace_back(b2s_e->m_vertex2.y);
                db2vs.emplace_back(b2s_e->m_vertex3.x);
                db2vs.emplace_back(b2s_e->m_vertex3.y);

                db2vs.emplace_back((float32_t)b2s_e->m_oneSided);
            }
            break;

            case b2Shape::e_polygon:
            {
                auto b2s_p = (b2PolygonShape *)b2s;

                db2vs.reserve(db2vs.size() + b2s_p->m_count * 2);

                for (int i = 0; i < b2s_p->m_count; i++)
                {
                    db2vs.emplace_back(b2s_p->m_vertices[i].x);
                    db2vs.emplace_back(b2s_p->m_vertices[i].y);
                }
            }
            break;

            case b2Shape::e_chain:
            {
                auto b2s_chain = (b2ChainShape *)b2s;

                db2vs.reserve(db2vs.size() + b2s_chain->m_count * 2 + 2 * 2);

                for (int i = 0; i < b2s_chain->m_count; i++)
                {
                    db2vs.emplace_back(b2s_chain->m_vertices[i].x);
                    db2vs.emplace_back(b2s_chain->m_vertices[i].y);
                }

                db2vs.emplace_back(b2s_chain->m_prevVertex.x);
                db2vs.emplace_back(b2s_chain->m_prevVertex.y);
                db2vs.emplace_back(b2s_chain->m_nextVertex.x);
                db2vs.emplace_back(b2s_chain->m_nextVertex.y);
            }
            break;
            }

            auto &shape_vecCount = db2vs.operator[]<int32_t>(db2fs[-1].shape_vecList);
            shape_vecCount = db2vs.size() - db2fs[-1].shape_vecList - 1;
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

            db2vs.size(),

            (uint64_t)b2j);
        b2j->GetUserData().pointer = (uintptr_t)db2js.size() - 1;

        /*paraCount*/ db2vs.emplace_back(0);

        switch (b2j->GetType())
        {
        case b2JointType::e_revoluteJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 11;

            auto b2j_r = (b2RevoluteJoint *)b2j;
            db2vs.emplace_back(b2j_r->GetLocalAnchorA().x);
            db2vs.emplace_back(b2j_r->GetLocalAnchorA().y);
            db2vs.emplace_back(b2j_r->GetLocalAnchorB().x);
            db2vs.emplace_back(b2j_r->GetLocalAnchorB().y);
            db2vs.emplace_back(b2j_r->GetReferenceAngle());
            db2vs.emplace_back((float32_t)b2j_r->IsLimitEnabled());
            db2vs.emplace_back(b2j_r->GetLowerLimit());
            db2vs.emplace_back(b2j_r->GetUpperLimit());
            db2vs.emplace_back((float32_t)b2j_r->IsMotorEnabled());
            db2vs.emplace_back(b2j_r->GetMotorSpeed());
            db2vs.emplace_back(b2j_r->GetMaxMotorTorque());
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 13;

            auto b2j_p = (b2PrismaticJoint *)b2j;
            db2vs.emplace_back(b2j_p->GetLocalAnchorA().x);
            db2vs.emplace_back(b2j_p->GetLocalAnchorA().y);
            db2vs.emplace_back(b2j_p->GetLocalAnchorB().x);
            db2vs.emplace_back(b2j_p->GetLocalAnchorB().y);
            db2vs.emplace_back(b2j_p->GetLocalAxisA().x);
            db2vs.emplace_back(b2j_p->GetLocalAxisA().y);
            db2vs.emplace_back(b2j_p->GetReferenceAngle());
            db2vs.emplace_back((float32_t)b2j_p->IsLimitEnabled());
            db2vs.emplace_back(b2j_p->GetLowerLimit());
            db2vs.emplace_back(b2j_p->GetUpperLimit());
            db2vs.emplace_back((float32_t)b2j_p->IsMotorEnabled());
            db2vs.emplace_back(b2j_p->GetMaxMotorForce());
            db2vs.emplace_back(b2j_p->GetMotorSpeed());
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 9;

            auto b2j_d = (b2DistanceJoint *)b2j;
            db2vs.emplace_back(b2j_d->GetLocalAnchorA().x);
            db2vs.emplace_back(b2j_d->GetLocalAnchorA().y);
            db2vs.emplace_back(b2j_d->GetLocalAnchorB().x);
            db2vs.emplace_back(b2j_d->GetLocalAnchorB().y);
            db2vs.emplace_back(b2j_d->GetLength());
            db2vs.emplace_back(b2j_d->GetMinLength());
            db2vs.emplace_back(b2j_d->GetMaxLength());
            db2vs.emplace_back(b2j_d->GetStiffness());
            db2vs.emplace_back(b2j_d->GetDamping());
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 11;

            auto b2j_p = (b2PulleyJoint *)b2j;
            db2vs.emplace_back(b2j_p->GetGroundAnchorA().x);
            db2vs.emplace_back(b2j_p->GetGroundAnchorA().y);
            db2vs.emplace_back(b2j_p->GetGroundAnchorB().x);
            db2vs.emplace_back(b2j_p->GetGroundAnchorB().y);

            auto localAnchorA = b2j_p->GetBodyA()->GetLocalPoint(b2j_p->GetAnchorA());
            db2vs.emplace_back(localAnchorA.x);
            db2vs.emplace_back(localAnchorA.y);
            auto localAnchorB = b2j_p->GetBodyB()->GetLocalPoint(b2j_p->GetAnchorB());
            db2vs.emplace_back(localAnchorB.x);
            db2vs.emplace_back(localAnchorB.y);

            db2vs.emplace_back(b2j_p->GetLengthA());
            db2vs.emplace_back(b2j_p->GetLengthB());
            db2vs.emplace_back(b2j_p->GetRatio());
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 5;

            auto b2j_m = (b2MouseJoint *)b2j;
            db2vs.emplace_back(b2j_m->GetTarget().x);
            db2vs.emplace_back(b2j_m->GetTarget().y);
            db2vs.emplace_back(b2j_m->GetMaxForce());
            db2vs.emplace_back(b2j_m->GetStiffness());
            db2vs.emplace_back(b2j_m->GetDamping());
        }
        break;

        case b2JointType::e_gearJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 3;

            auto b2j_g = (b2GearJoint *)b2j;
            db2vs.emplace_back((float32_t)b2j_g->GetJoint1()->GetUserData().pointer);
            db2vs.emplace_back((float32_t)b2j_g->GetJoint2()->GetUserData().pointer);
            db2vs.emplace_back(b2j_g->GetRatio());
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 14;

            auto b2j_w = (b2WheelJoint *)b2j;
            db2vs.emplace_back(b2j_w->GetLocalAnchorA().x);
            db2vs.emplace_back(b2j_w->GetLocalAnchorA().y);
            db2vs.emplace_back(b2j_w->GetLocalAnchorB().x);
            db2vs.emplace_back(b2j_w->GetLocalAnchorB().y);
            db2vs.emplace_back(b2j_w->GetLocalAxisA().x);
            db2vs.emplace_back(b2j_w->GetLocalAxisA().y);
            db2vs.emplace_back((float32_t)b2j_w->IsLimitEnabled());
            db2vs.emplace_back(b2j_w->GetLowerLimit());
            db2vs.emplace_back(b2j_w->GetUpperLimit());
            db2vs.emplace_back((float32_t)b2j_w->IsMotorEnabled());
            db2vs.emplace_back(b2j_w->GetMaxMotorTorque());
            db2vs.emplace_back(b2j_w->GetMotorSpeed());
            db2vs.emplace_back(b2j_w->GetStiffness());
            db2vs.emplace_back(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_weldJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 7;

            auto b2j_w = (b2WeldJoint *)b2j;
            db2vs.emplace_back(b2j_w->GetLocalAnchorA().x);
            db2vs.emplace_back(b2j_w->GetLocalAnchorA().y);
            db2vs.emplace_back(b2j_w->GetLocalAnchorB().x);
            db2vs.emplace_back(b2j_w->GetLocalAnchorB().y);
            db2vs.emplace_back(b2j_w->GetReferenceAngle());
            db2vs.emplace_back(b2j_w->GetStiffness());
            db2vs.emplace_back(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 6;

            auto b2j_f = (b2FrictionJoint *)b2j;
            db2vs.emplace_back(b2j_f->GetLocalAnchorA().x);
            db2vs.emplace_back(b2j_f->GetLocalAnchorA().y);
            db2vs.emplace_back(b2j_f->GetLocalAnchorB().x);
            db2vs.emplace_back(b2j_f->GetLocalAnchorB().y);
            db2vs.emplace_back(b2j_f->GetMaxForce());
            db2vs.emplace_back(b2j_f->GetMaxTorque());
        }
        break;

        case b2JointType::e_ropeJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 0;

            // auto b2j_r = (b2RopeJoint *)b2j;
        }
        break;

        case b2JointType::e_motorJoint:
        {
            /*paraCount*/ db2vs.operator[]<int32_t>(db2js[-1].para) = 6;

            auto b2j_m = (b2MotorJoint *)b2j;
            db2vs.emplace_back(b2j_m->GetLinearOffset().x);
            db2vs.emplace_back(b2j_m->GetLinearOffset().y);
            db2vs.emplace_back(b2j_m->GetAngularOffset());
            db2vs.emplace_back(b2j_m->GetMaxForce());
            db2vs.emplace_back(b2j_m->GetMaxTorque());
            db2vs.emplace_back(b2j_m->GetCorrectionFactor());
        }
        break;

        default:
            break;
        }
    }

    delete this->db2;
    this->db2 = _db2;
}