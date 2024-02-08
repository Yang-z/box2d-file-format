#include "db2_decoder.h"

#include <assert.h>

db2Decoder::~db2Decoder()
{
    delete this->db2;
    delete this->b2w;
}

auto db2Decoder::decode() -> void
{
    if (!this->db2)
        return;

    auto &db2ws = this->db2->get<db2Chunk<db2Wrold>>();
    auto &db2js = this->db2->get<db2Chunk<db2Joint>>();
    auto &db2bs = this->db2->get<db2Chunk<db2Body>>();
    auto &db2fs = this->db2->get<db2Chunk<db2Fixture>>();
    auto &db2ss = this->db2->get<db2Chunk<db2Shape>>();

    /*world*/
    auto &db2w = db2ws[0];
    b2Vec2 gravity{db2w.gravity_x, db2w.gravity_y};
    this->b2w = new b2World{gravity};

    /*body*/
    for (auto b = db2w.bodyList; b < db2w.bodyList + db2w.bodyCount; ++b)
    {
        auto &db2b = db2bs[b];

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

        /*userData*/ b2bdef.userData.pointer = (uintptr_t)b;

        auto *b2b = b2w->CreateBody(&b2bdef);

        /*.userData*/ db2bs.userData.push(b2b);

        /*fixture*/
        for (auto f = db2b.fixtureList; f <= db2b.fixtureList + db2b.fixtureCount - 1; ++f)
        {
            db2Fixture &db2f = db2fs[f];
            b2FixtureDef b2fdef{};

            b2fdef.friction = db2f.friction;
            b2fdef.restitution = db2f.restitution;
            b2fdef.restitutionThreshold = db2f.restitutionThreshold;
            b2fdef.density = db2f.density;
            b2fdef.isSensor = db2f.isSensor;

            b2fdef.filter.categoryBits = db2f.filter_categoryBits;
            b2fdef.filter.maskBits = db2f.filter_maskBits;
            b2fdef.filter.groupIndex = db2f.filter_groupIndex;

            /*shape*/
            auto &db2s = db2ss[db2f.shape];
            b2Shape *b2s = nullptr;

            switch ((b2Shape::Type)db2s.type3())
            {

            case b2Shape::Type::e_circle:
            {
                assert(db2s.size() == 1 + 2);

                b2s = new b2CircleShape();
                auto &b2s_t = *(b2CircleShape *)b2s;

                b2s_t.m_radius = db2s.shape_radius();

                auto db2s_ptr = db2s.shape_extend(); // 1
                b2s_t.m_p = {db2s[db2s_ptr++], db2s[db2s_ptr++]};
            }
            break;

            case b2Shape::e_edge:
            {
                assert(db2s.size() == 1 + 9);

                b2s = new b2EdgeShape();
                auto &b2s_t = *(b2EdgeShape *)b2s;

                b2s_t.m_radius = db2s.shape_radius(); // default: b2_polygonRadius

                auto db2s_ptr = db2s.shape_extend(); // 1
                b2s_t.m_vertex0 = {db2s[db2s_ptr++], db2s[db2s_ptr++]};
                b2s_t.m_vertex1 = {db2s[db2s_ptr++], db2s[db2s_ptr++]};
                b2s_t.m_vertex2 = {db2s[db2s_ptr++], db2s[db2s_ptr++]};
                b2s_t.m_vertex3 = {db2s[db2s_ptr++], db2s[db2s_ptr++]};
                b2s_t.m_oneSided = (bool)db2s[db2s_ptr++];
            }
            break;

            case b2Shape::e_polygon:
            {
                assert(db2s.size() >= 1 + 6);

                b2s = new b2PolygonShape();
                auto &b2s_t = *(b2PolygonShape *)b2s;

                b2s_t.m_radius = db2s.shape_radius(); // default: b2_polygonRadius

                auto db2s_ptr = db2s.shape_extend(); // 1
                auto points = (b2Vec2 *)(&(db2s[db2s_ptr]));
                auto count = (db2s.size() - 1) / 2;
                b2s_t.Set(points, count);
            }
            break;

            case b2Shape::e_chain:
            {
                assert(db2s.size() >= 1 + 8);

                b2s = new b2ChainShape();
                auto &b2s_t = *(b2ChainShape *)b2s;

                b2s_t.m_radius = db2s.shape_radius();

                /*
                CreateChain or CreateLoop?
                Since we record the "fusion point", m_prevVertex and m_nextVertex,
                they are the same.
                */
                auto db2s_ptr = db2s.shape_extend(); // 1
                auto points = (b2Vec2 *)(&(db2s[db2s_ptr]));
                auto count = (db2s.size() - 1 - 2 * 2) / 2;
                b2s_t.CreateChain(
                    points,
                    count,
                    {db2s[db2s_ptr + count * 2 + 0], db2s[db2s_ptr + count * 2 + 1]},
                    {db2s[db2s_ptr + count * 2 + 2], db2s[db2s_ptr + count * 2 + 3]});
            }
            break;
            }

            b2fdef.shape = b2s; // The shape will be cloned
            /*userData*/ b2fdef.userData.pointer = (uintptr_t)f;
            /*.userData*/ db2fs.userData.push(b2b->CreateFixture(&b2fdef));
            if (b2s)
                delete b2s; // virtual ~b2Shape()
        }
    }

    /*joint*/
    for (auto j = db2w.jointList; j < db2w.jointList + db2w.jointCount; ++j)
    {
        auto &db2j = db2js[j];

        auto p = db2j.extend();

        b2JointDef *b2jdef{nullptr};

        switch (b2JointType(db2j.type3()))
        {
        case b2JointType::e_revoluteJoint:
        {
            b2jdef = new b2RevoluteJointDef();
            auto &b2jdef_t = *(b2RevoluteJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 11);

            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.referenceAngle = db2j[p++];
            b2jdef_t.enableLimit = (bool)db2j[p++];
            b2jdef_t.lowerAngle = db2j[p++];
            b2jdef_t.upperAngle = db2j[p++];
            b2jdef_t.enableMotor = (bool)db2j[p++];
            b2jdef_t.motorSpeed = db2j[p++];
            b2jdef_t.maxMotorTorque = db2j[p++];
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            b2jdef = new b2PrismaticJointDef();
            auto &b2jdef_t = *(b2PrismaticJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 13);

            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.localAxisA = {db2j[p++], db2j[p++]};
            b2jdef_t.referenceAngle = db2j[p++];
            b2jdef_t.enableLimit = (bool)db2j[p++];
            b2jdef_t.lowerTranslation = db2j[p++];
            b2jdef_t.upperTranslation = db2j[p++];
            b2jdef_t.enableMotor = (bool)db2j[p++];
            b2jdef_t.maxMotorForce = db2j[p++];
            b2jdef_t.motorSpeed = db2j[p++];
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            b2jdef = new b2DistanceJointDef();
            auto &b2jdef_t = *(b2DistanceJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 9);

            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.length = db2j[p++];
            b2jdef_t.minLength = db2j[p++];
            b2jdef_t.maxLength = db2j[p++];
            b2jdef_t.stiffness = db2j[p++];
            b2jdef_t.damping = db2j[p++];
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            b2jdef = new b2PulleyJointDef();
            auto &b2jdef_t = *(b2PulleyJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 11);

            b2jdef_t.groundAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.groundAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.lengthA = db2j[p++];
            b2jdef_t.lengthB = db2j[p++];
            b2jdef_t.ratio = db2j[p++];
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            b2jdef = new b2MouseJointDef();
            auto &b2jdef_t = *(b2MouseJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 5);

            b2jdef_t.target = {db2j[p++], db2j[p++]};
            b2jdef_t.maxForce = db2j[p++];
            b2jdef_t.stiffness = db2j[p++];
            b2jdef_t.damping = db2j[p++];
        }
        break;

        case b2JointType::e_gearJoint:
        {
            b2jdef = new b2GearJointDef();
            auto &b2jdef_t = *(b2GearJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 3);

            b2jdef_t.joint1 = (b2Joint *)db2js.userData[(int)db2j[p++]];
            b2jdef_t.joint2 = (b2Joint *)db2js.userData[(int)db2j[p++]];
            b2jdef_t.ratio = db2j[p++];
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            b2jdef = new b2WheelJointDef();
            auto &b2jdef_t = *(b2WheelJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 14);

            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.localAxisA = {db2j[p++], db2j[p++]};
            b2jdef_t.enableLimit = (bool)db2j[p++];
            b2jdef_t.lowerTranslation = db2j[p++];
            b2jdef_t.upperTranslation = db2j[p++];
            b2jdef_t.enableMotor = (bool)db2j[p++];
            b2jdef_t.maxMotorTorque = db2j[p++];
            b2jdef_t.motorSpeed = db2j[p++];
            b2jdef_t.stiffness = db2j[p++];
            b2jdef_t.damping = db2j[p++];
        }
        break;

        case b2JointType::e_weldJoint:
        {
            b2jdef = new b2WeldJointDef();
            auto &b2jdef_t = *(b2WeldJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 7);

            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.referenceAngle = db2j[p++];
            b2jdef_t.stiffness = db2j[p++];
            b2jdef_t.damping = db2j[p++];
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            b2jdef = new b2FrictionJointDef();
            auto &b2jdef_t = *(b2FrictionJointDef *)b2jdef;

            b2FrictionJointDef b2jdef;
            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 6);

            b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
            b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
            b2jdef_t.maxForce = db2j[p++];
            b2jdef_t.maxTorque = db2j[p++];
        }
        break;

        case b2JointType::e_ropeJoint:
        {
            // b2RopeJointDef b2jdef;
        }
        break;

        case b2JointType::e_motorJoint:
        {
            b2jdef = new b2MotorJointDef();
            auto &b2jdef_t = *(b2MotorJointDef *)b2jdef;

            b2jdef_t.bodyA = (b2Body *)db2bs.userData[db2j.bodyA()];
            b2jdef_t.bodyB = (b2Body *)db2bs.userData[db2j.bodyB()];
            b2jdef_t.collideConnected = db2j.collideConnected();

            assert(db2j.size() == 3 + 6);

            b2jdef_t.linearOffset = {db2j[p++], db2j[p++]};
            b2jdef_t.angularOffset = db2j[p++];
            b2jdef_t.maxForce = db2j[p++];
            b2jdef_t.maxTorque = db2j[p++];
            b2jdef_t.correctionFactor = db2j[p++];
        }
        break;

        default:
            break;
        }

        /*userData*/ b2jdef->userData.pointer = (uintptr_t)j;
        /*.userData*/ db2js.userData.push(b2w->CreateJoint(b2jdef));
        if (b2jdef)
            delete b2jdef;
    }
}

auto db2Decoder::encode() -> void
{
    if (!this->b2w)
        return;

    auto _db2 = new dotBox2d();

    auto &db2is = _db2->get<db2Chunk<db2Info>>();

    auto &db2ws = _db2->get<db2Chunk<db2Wrold>>();
    auto &db2js = _db2->get<db2Chunk<db2Joint>>();
    auto &db2bs = _db2->get<db2Chunk<db2Body>>();
    auto &db2fs = _db2->get<db2Chunk<db2Fixture>>();
    auto &db2ss = _db2->get<db2Chunk<db2Shape>>();

    /*info*/
    // constructs an element in-place at the end
    db2is.emplace(); // default

    /*world*/
    db2ws.emplace(
        this->b2w->GetGravity().x,
        this->b2w->GetGravity().y,

        db2bs.size(),
        this->b2w->GetBodyCount(),

        db2js.size(),
        this->b2w->GetJointCount()
        //
    );

    /*body*/
    db2DynArray<b2Body *> bodies{};
    bodies.reserve(this->b2w->GetBodyCount(), false);
    for (auto b2b = this->b2w->GetBodyList(); b2b; b2b = b2b->GetNext())
        bodies.push(b2b);

    for (int b = bodies.size() - 1; b >= 0; --b) // reverse order
    {
        auto b2b = bodies[b];

        db2bs.emplace(
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

            /*fixtureList*/ db2fs.size(),
            /*fixtureCount*/ 0

            // /*extraDict*/ -1
        );
        /*.userData*/ db2bs.userData.push(b2b);
        /*userData*/ b2b->GetUserData().pointer = (uintptr_t)b; // db2bs.size() - 1;

        /*fixture*/
        db2DynArray<b2Fixture *> fixtures{};
        // fixtures.reserve(b2b->GetFixtureCount(), false); // no way to get fixture count directly
        for (auto b2f = b2b->GetFixtureList(); b2f; b2f = b2f->GetNext())
            fixtures.push(b2f);

        db2bs[-1].fixtureCount = fixtures.size();

        for (int f = fixtures.size() - 1; f >= 0; --f) // reverse order
        {
            auto b2f = fixtures[f];

            db2fs.emplace(
                b2f->GetFriction(),
                b2f->GetRestitution(),
                b2f->GetRestitutionThreshold(),
                b2f->GetDensity(),
                b2f->IsSensor(),

                /*filter*/
                b2f->GetFilterData().categoryBits,
                b2f->GetFilterData().maskBits,
                b2f->GetFilterData().groupIndex,

                /*shape*/ db2ss.size()

                // /*extra*/ 0
            );
            /*.userData*/ db2fs.userData.push(b2f);
            /*userData*/ b2f->GetUserData().pointer = (uintptr_t)db2fs.size() - 1;

            /*shape*/
            auto b2s = b2f->GetShape();
            /*add sub-chunk*/ auto &db2s = db2ss.emplace();

            /*shape_type*/ db2s.type3() = b2s->m_type;
            /*shape_radius*/ db2s.emplace(b2s->m_radius);

            switch (b2s->m_type)
            {
            case b2Shape::e_circle:
            {
                auto b2s_c = (b2CircleShape *)b2s;

                db2s.reserve(db2s.size() + 1 * 2);

                db2s.emplace(b2s_c->m_p.x);
                db2s.emplace(b2s_c->m_p.y);
            }
            break;

            case b2Shape::e_edge:
            {
                auto b2s_e = (b2EdgeShape *)b2s;

                db2s.reserve(db2s.size() + 4 * 2 + 1);

                db2s.emplace(b2s_e->m_vertex0.x);
                db2s.emplace(b2s_e->m_vertex0.y);
                db2s.emplace(b2s_e->m_vertex1.x);
                db2s.emplace(b2s_e->m_vertex1.y);
                db2s.emplace(b2s_e->m_vertex2.x);
                db2s.emplace(b2s_e->m_vertex2.y);
                db2s.emplace(b2s_e->m_vertex3.x);
                db2s.emplace(b2s_e->m_vertex3.y);

                db2s.emplace((float32_t)b2s_e->m_oneSided);
            }
            break;

            case b2Shape::e_polygon:
            {
                auto b2s_p = (b2PolygonShape *)b2s;

                db2s.reserve(db2s.size() + b2s_p->m_count * 2);

                for (int i = 0; i < b2s_p->m_count; i++)
                {
                    db2s.emplace(b2s_p->m_vertices[i].x);
                    db2s.emplace(b2s_p->m_vertices[i].y);
                }
            }
            break;

            case b2Shape::e_chain:
            {
                auto b2s_chain = (b2ChainShape *)b2s;

                db2s.reserve(db2s.size() + b2s_chain->m_count * 2 + 2 * 2);

                for (int i = 0; i < b2s_chain->m_count; i++)
                {
                    db2s.emplace(b2s_chain->m_vertices[i].x);
                    db2s.emplace(b2s_chain->m_vertices[i].y);
                }

                db2s.emplace(b2s_chain->m_prevVertex.x);
                db2s.emplace(b2s_chain->m_prevVertex.y);
                db2s.emplace(b2s_chain->m_nextVertex.x);
                db2s.emplace(b2s_chain->m_nextVertex.y);
            }
            break;
            }
        }
    }

    /*joint*/
    db2DynArray<b2Joint *> joints{};
    joints.reserve(this->b2w->GetJointCount(), false);
    for (auto b2j = this->b2w->GetJointList(); b2j; b2j = b2j->GetNext())
        joints.push(b2j);

    for (int j = joints.size() - 1; j >= 0; --j) // reverse order
    {
        auto &b2j = joints[j];

        /*add sub-chunk*/ auto &db2j = db2js.emplace();
        /*type*/ db2j.type3() = b2j->GetType();
        /*bodyA*/ db2j.emplace(int32_t(b2j->GetBodyA()->GetUserData().pointer));
        /*bodyB*/ db2j.emplace(int32_t(b2j->GetBodyB()->GetUserData().pointer));
        /*collideConnected*/ db2j.emplace(b2j->GetCollideConnected());

        /*.userData*/ db2js.userData.push(b2j);
        /*userData*/ b2j->GetUserData().pointer = (uintptr_t)j; // db2js.size() - 1;

        switch (b2j->GetType())
        {
        case b2JointType::e_revoluteJoint:
        {
            auto b2j_r = (b2RevoluteJoint *)b2j;
            db2j.emplace(b2j_r->GetLocalAnchorA().x);
            db2j.emplace(b2j_r->GetLocalAnchorA().y);
            db2j.emplace(b2j_r->GetLocalAnchorB().x);
            db2j.emplace(b2j_r->GetLocalAnchorB().y);
            db2j.emplace(b2j_r->GetReferenceAngle());
            db2j.emplace((float32_t)b2j_r->IsLimitEnabled());
            db2j.emplace(b2j_r->GetLowerLimit());
            db2j.emplace(b2j_r->GetUpperLimit());
            db2j.emplace((float32_t)b2j_r->IsMotorEnabled());
            db2j.emplace(b2j_r->GetMotorSpeed());
            db2j.emplace(b2j_r->GetMaxMotorTorque());
        }
        break;

        case b2JointType::e_prismaticJoint:
        {
            auto b2j_p = (b2PrismaticJoint *)b2j;
            db2j.emplace(b2j_p->GetLocalAnchorA().x);
            db2j.emplace(b2j_p->GetLocalAnchorA().y);
            db2j.emplace(b2j_p->GetLocalAnchorB().x);
            db2j.emplace(b2j_p->GetLocalAnchorB().y);
            db2j.emplace(b2j_p->GetLocalAxisA().x);
            db2j.emplace(b2j_p->GetLocalAxisA().y);
            db2j.emplace(b2j_p->GetReferenceAngle());
            db2j.emplace((float32_t)b2j_p->IsLimitEnabled());
            db2j.emplace(b2j_p->GetLowerLimit());
            db2j.emplace(b2j_p->GetUpperLimit());
            db2j.emplace((float32_t)b2j_p->IsMotorEnabled());
            db2j.emplace(b2j_p->GetMaxMotorForce());
            db2j.emplace(b2j_p->GetMotorSpeed());
        }
        break;

        case b2JointType::e_distanceJoint:
        {
            auto b2j_d = (b2DistanceJoint *)b2j;
            db2j.emplace(b2j_d->GetLocalAnchorA().x);
            db2j.emplace(b2j_d->GetLocalAnchorA().y);
            db2j.emplace(b2j_d->GetLocalAnchorB().x);
            db2j.emplace(b2j_d->GetLocalAnchorB().y);
            db2j.emplace(b2j_d->GetLength());
            db2j.emplace(b2j_d->GetMinLength());
            db2j.emplace(b2j_d->GetMaxLength());
            db2j.emplace(b2j_d->GetStiffness());
            db2j.emplace(b2j_d->GetDamping());
        }
        break;

        case b2JointType::e_pulleyJoint:
        {
            auto b2j_p = (b2PulleyJoint *)b2j;
            db2j.emplace(b2j_p->GetGroundAnchorA().x);
            db2j.emplace(b2j_p->GetGroundAnchorA().y);
            db2j.emplace(b2j_p->GetGroundAnchorB().x);
            db2j.emplace(b2j_p->GetGroundAnchorB().y);

            auto localAnchorA = b2j_p->GetBodyA()->GetLocalPoint(b2j_p->GetAnchorA());
            db2j.emplace(localAnchorA.x);
            db2j.emplace(localAnchorA.y);
            auto localAnchorB = b2j_p->GetBodyB()->GetLocalPoint(b2j_p->GetAnchorB());
            db2j.emplace(localAnchorB.x);
            db2j.emplace(localAnchorB.y);

            db2j.emplace(b2j_p->GetLengthA());
            db2j.emplace(b2j_p->GetLengthB());
            db2j.emplace(b2j_p->GetRatio());
        }
        break;

        case b2JointType::e_mouseJoint:
        {
            auto b2j_m = (b2MouseJoint *)b2j;
            db2j.emplace(b2j_m->GetTarget().x);
            db2j.emplace(b2j_m->GetTarget().y);
            db2j.emplace(b2j_m->GetMaxForce());
            db2j.emplace(b2j_m->GetStiffness());
            db2j.emplace(b2j_m->GetDamping());
        }
        break;

        case b2JointType::e_gearJoint:
        {
            auto b2j_g = (b2GearJoint *)b2j;
            db2j.emplace((float32_t)b2j_g->GetJoint1()->GetUserData().pointer);
            db2j.emplace((float32_t)b2j_g->GetJoint2()->GetUserData().pointer);
            db2j.emplace(b2j_g->GetRatio());
        }
        break;

        case b2JointType::e_wheelJoint:
        {
            auto b2j_w = (b2WheelJoint *)b2j;
            db2j.emplace(b2j_w->GetLocalAnchorA().x);
            db2j.emplace(b2j_w->GetLocalAnchorA().y);
            db2j.emplace(b2j_w->GetLocalAnchorB().x);
            db2j.emplace(b2j_w->GetLocalAnchorB().y);
            db2j.emplace(b2j_w->GetLocalAxisA().x);
            db2j.emplace(b2j_w->GetLocalAxisA().y);
            db2j.emplace((float32_t)b2j_w->IsLimitEnabled());
            db2j.emplace(b2j_w->GetLowerLimit());
            db2j.emplace(b2j_w->GetUpperLimit());
            db2j.emplace((float32_t)b2j_w->IsMotorEnabled());
            db2j.emplace(b2j_w->GetMaxMotorTorque());
            db2j.emplace(b2j_w->GetMotorSpeed());
            db2j.emplace(b2j_w->GetStiffness());
            db2j.emplace(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_weldJoint:
        {
            auto b2j_w = (b2WeldJoint *)b2j;
            db2j.emplace(b2j_w->GetLocalAnchorA().x);
            db2j.emplace(b2j_w->GetLocalAnchorA().y);
            db2j.emplace(b2j_w->GetLocalAnchorB().x);
            db2j.emplace(b2j_w->GetLocalAnchorB().y);
            db2j.emplace(b2j_w->GetReferenceAngle());
            db2j.emplace(b2j_w->GetStiffness());
            db2j.emplace(b2j_w->GetDamping());
        }
        break;

        case b2JointType::e_frictionJoint:
        {
            auto b2j_f = (b2FrictionJoint *)b2j;
            db2j.emplace(b2j_f->GetLocalAnchorA().x);
            db2j.emplace(b2j_f->GetLocalAnchorA().y);
            db2j.emplace(b2j_f->GetLocalAnchorB().x);
            db2j.emplace(b2j_f->GetLocalAnchorB().y);
            db2j.emplace(b2j_f->GetMaxForce());
            db2j.emplace(b2j_f->GetMaxTorque());
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
            db2j.emplace(b2j_m->GetLinearOffset().x);
            db2j.emplace(b2j_m->GetLinearOffset().y);
            db2j.emplace(b2j_m->GetAngularOffset());
            db2j.emplace(b2j_m->GetMaxForce());
            db2j.emplace(b2j_m->GetMaxTorque());
            db2j.emplace(b2j_m->GetCorrectionFactor());
        }
        break;

        default:
            break;
        }
    }

    delete this->db2;
    this->db2 = _db2;
}