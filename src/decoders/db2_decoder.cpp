#include "db2_decoder.h"

auto db2Decoder::Decode(dotBox2d &db2) -> void
{
    if (db2.p_b2w)
        return;

    /*world*/
    auto &world_dict = db2.world_dict();
    if (world_dict == nullval)
        return;
    auto &db2w = world_dict.at<CKWorld>(db2Key::Base);

    b2Vec2 gravity;
    db2Decoder::Decode_World(db2w, gravity);

    db2.p_b2w = new b2World{gravity};
    db2.dt = 1.0f / db2w.inv_dt;
    db2.inv_dt = db2w.inv_dt;
    db2.velocityIterations = db2w.velocityIterations;
    db2.positionIterations = db2w.positionIterations;

    /*body*/
    auto &world_body_list = world_dict.at<CKList>(db2Key::BODY);

    if (world_body_list != nullval)
    {
        for (auto b = 0; b < world_body_list.size(); ++b)
        {
            auto &body_dict = world_body_list.at<CKDict>(b);
            auto &db2b = body_dict.at<CKBody>(db2Key::Base);

            b2BodyDef b2bdef{};
            db2Decoder::Decode_Body(db2b, b2bdef);

            /*userData*/ b2bdef.userData.pointer = (uintptr_t)world_body_list.ref<CKDict>(b);
            auto p_b2b = db2.p_b2w->CreateBody(&b2bdef);
            /*.userData*/ body_dict.runtime = p_b2b;

            /*fixture*/
            auto &body_fixture_list = body_dict.at<CKList>(db2Key::FIXTURE);
            if (body_fixture_list != nullval)
                for (auto f = 0; f < body_fixture_list.size(); ++f)
                {
                    auto &fixture_dict = body_fixture_list.at<CKDict>(f);
                    auto &db2f = fixture_dict.at<CKFixture>(db2Key::Base);

                    b2FixtureDef b2fdef{};
                    db2Decoder::Decode_Fixture(db2f, b2fdef);

                    /*shape*/
                    b2Shape *p_b2s = nullptr;
                    {
                        auto &db2s = fixture_dict.at<CKShape>(db2Key::SHAPE);
                        db2Decoder::Decode_Shpae(db2s, p_b2s);
                    }

                    b2fdef.shape = p_b2s; // The shape will be cloned

                    /*userData*/ b2fdef.userData.pointer = (uintptr_t)body_fixture_list.ref<CKDict>(f);
                    auto p_b2f = p_b2b->CreateFixture(&b2fdef);
                    /*.userData*/ fixture_dict.runtime = p_b2f;

                    if (p_b2s)
                        delete p_b2s; // virtual ~b2Shape()
                }
        }
    }

    // /*test return*/ return;

    /*joint*/
    auto &world_joint_list = world_dict.at<CKList>(db2Key::JOINT);
    if (world_joint_list != nullval)
    {
        for (auto j = 0; j < world_joint_list.size(); ++j)
        {
            auto &joint_dict = world_joint_list.at<CKDict>(j);
            auto &db2j = joint_dict.at<CKJoint>(db2Key::Base);

            b2JointDef *p_b2jdef{nullptr};
            db2Decoder::Decode_Joint(db2j, p_b2jdef);

            if (db2j.type3() != b2JointType::e_gearJoint)
            {
                auto &joint_body_list = joint_dict.at<CKList>(db2Key::BODY);
                p_b2jdef->bodyA = (b2Body *)joint_body_list.at<CKDict>(0).runtime;
                p_b2jdef->bodyB = (b2Body *)joint_body_list.at<CKDict>(1).runtime;
            }
            else
            {
                auto &joint_joint_list = joint_dict.at<CKList>(db2Key::JOINT);
                ((b2GearJointDef *)p_b2jdef)->joint1 = (b2Joint *)joint_joint_list.at<CKDict>(0).runtime;
                ((b2GearJointDef *)p_b2jdef)->joint2 = (b2Joint *)joint_joint_list.at<CKDict>(1).runtime;
            }

            /*userData*/ p_b2jdef->userData.pointer = (uintptr_t)world_joint_list.ref<CKDict>(j);
            /*.userData*/ joint_dict.runtime = db2.p_b2w->CreateJoint(p_b2jdef);
            if (p_b2jdef)
                delete p_b2jdef;
        }
    }
}

auto db2Decoder::Decode_World(db2World &db2w, b2Vec2 &gravity) -> void
{
    gravity.x = db2w.gravity_x;
    gravity.y = db2w.gravity_y;
}

auto db2Decoder::Decode_Body(db2Body &db2b, b2BodyDef &b2bdef) -> void
{
    /*
    We can't control the packsize of libbox2d,
    so we cna't use std::memcpy safely.
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
}

auto db2Decoder::Decode_Fixture(db2Fixture &db2f, b2FixtureDef &b2fdef) -> void
{
    b2fdef.friction = db2f.friction;
    b2fdef.restitution = db2f.restitution;
    b2fdef.restitutionThreshold = db2f.restitutionThreshold;
    b2fdef.density = db2f.density;
    b2fdef.isSensor = db2f.isSensor;

    b2fdef.filter.categoryBits = db2f.filter_categoryBits;
    b2fdef.filter.maskBits = db2f.filter_maskBits;
    b2fdef.filter.groupIndex = db2f.filter_groupIndex;
}

auto db2Decoder::Decode_Shpae(db2Shape &db2s, b2Shape *&p_b2s) -> void
{
    static_assert(sizeof(b2Vec2) == 8);

    switch ((b2Shape::Type)db2s.type3())
    {

    case b2Shape::Type::e_circle:
    {
        assert(db2s.size() == 1 + 2);

        p_b2s = new b2CircleShape();
        auto &b2s_t = *(b2CircleShape *)p_b2s;

        int32_t i = 0;
        b2s_t.m_radius = db2s[i++]; // default: b2_polygonRadius

        b2s_t.m_p = {db2s[i++], db2s[i++]};
    }
    break;

    case b2Shape::e_edge:
    {
        assert(db2s.size() == 1 + 9);

        p_b2s = new b2EdgeShape();
        auto &b2s_t = *(b2EdgeShape *)p_b2s;

        int32_t i = 0;
        b2s_t.m_radius = db2s[i++]; // default: b2_polygonRadius

        b2s_t.m_vertex0 = {db2s[i++], db2s[i++]};
        b2s_t.m_vertex1 = {db2s[i++], db2s[i++]};
        b2s_t.m_vertex2 = {db2s[i++], db2s[i++]};
        b2s_t.m_vertex3 = {db2s[i++], db2s[i++]};
        b2s_t.m_oneSided = (bool)db2s[i++];
    }
    break;

    case b2Shape::e_polygon:
    {
        assert(db2s.size() >= 1 + 6);

        p_b2s = new b2PolygonShape();
        auto &b2s_t = *(b2PolygonShape *)p_b2s;

        int32_t i = 0;
        b2s_t.m_radius = db2s[i++]; // default: b2_polygonRadius

        auto points = (b2Vec2 *)(&(db2s[i])); // !
        auto count = (db2s.size() - 1) / 2;
        b2s_t.Set(points, count);
    }
    break;

    case b2Shape::e_chain:
    {
        assert(db2s.size() >= 1 + 8);

        p_b2s = new b2ChainShape();
        auto &b2s_t = *(b2ChainShape *)p_b2s;

        int32_t i = 0;
        b2s_t.m_radius = db2s[i++]; // default: b2_polygonRadius

        /*
        CreateChain or CreateLoop?
        Since we record the "fusion point", m_prevVertex and m_nextVertex,
        they are the same.
        */
        auto points = (b2Vec2 *)(&(db2s[i])); // !
        auto count = (db2s.size() - 1 - 2 * 2) / 2;
        b2s_t.CreateChain(
            points,
            count,
            {db2s[i + count * 2 + 0], db2s[i + count * 2 + 1]},
            {db2s[i + count * 2 + 2], db2s[i + count * 2 + 3]});
    }
    break;
    }
}

auto db2Decoder::Decode_Joint(db2Joint &db2j, b2JointDef *&p_b2jdef) -> void
{
    int32_t p = 0;

    switch (b2JointType(db2j.type3()))
    {
    case b2JointType::e_revoluteJoint:
    {
        p_b2jdef = new b2RevoluteJointDef();
        auto &b2jdef_t = *(b2RevoluteJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 11);

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
        p_b2jdef = new b2PrismaticJointDef();
        auto &b2jdef_t = *(b2PrismaticJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 13);

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
        p_b2jdef = new b2DistanceJointDef();
        auto &b2jdef_t = *(b2DistanceJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 9);

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
        p_b2jdef = new b2PulleyJointDef();
        auto &b2jdef_t = *(b2PulleyJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 11);

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
        p_b2jdef = new b2MouseJointDef();
        auto &b2jdef_t = *(b2MouseJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 5);

        b2jdef_t.target = {db2j[p++], db2j[p++]};
        b2jdef_t.maxForce = db2j[p++];
        b2jdef_t.stiffness = db2j[p++];
        b2jdef_t.damping = db2j[p++];
    }
    break;

    case b2JointType::e_gearJoint:
    {
        p_b2jdef = new b2GearJointDef();
        auto &b2jdef_t = *(b2GearJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 1);

        // b2jdef_t.joint1;
        // b2jdef_t.joint2;
        b2jdef_t.ratio = db2j[p++];
    }
    break;

    case b2JointType::e_wheelJoint:
    {
        p_b2jdef = new b2WheelJointDef();
        auto &b2jdef_t = *(b2WheelJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 14);

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
        p_b2jdef = new b2WeldJointDef();
        auto &b2jdef_t = *(b2WeldJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 7);

        b2jdef_t.localAnchorA = {db2j[p++], db2j[p++]};
        b2jdef_t.localAnchorB = {db2j[p++], db2j[p++]};
        b2jdef_t.referenceAngle = db2j[p++];
        b2jdef_t.stiffness = db2j[p++];
        b2jdef_t.damping = db2j[p++];
    }
    break;

    case b2JointType::e_frictionJoint:
    {
        p_b2jdef = new b2FrictionJointDef();
        auto &b2jdef_t = *(b2FrictionJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 6);

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
        p_b2jdef = new b2MotorJointDef();
        auto &b2jdef_t = *(b2MotorJointDef *)p_b2jdef;

        b2jdef_t.collideConnected = (bool)db2j[p++];

        assert(db2j.size() == 1 + 6);

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
}

auto db2Decoder::Encode(dotBox2d &db2) -> void
{
    if (!db2.p_b2w)
        return;

    auto &dicts = db2.chunks.get<CKDict>();

    // info
    {
        // constructs an element in-place at the end
        auto &info = dicts.emplace_back();
        info.emplace<CKInfo>(db2Key::Base);
    }

    // world
    auto world_dict_i = dicts.size();
    {
        auto &world_dict = dicts.emplace_back();
        auto &db2w = world_dict.emplace<CKWorld>(db2Key::Base);
        db2Decoder::Encode_World(*db2.p_b2w, db2w);
    }

    /*body*/
    db2DynArray<b2Body *> bodies{};
    bodies.reserve(db2.p_b2w->GetBodyCount(), false);
    for (auto p_b2b = db2.p_b2w->GetBodyList(); p_b2b; p_b2b = p_b2b->GetNext())
        bodies.push_back(p_b2b);

    for (int b = bodies.size() - 1; b >= 0; --b) // reverse order
    {
        auto p_b2b = bodies[b];

        auto body_dict_i = dicts.size();
        {
            auto &world_body_list = dicts[world_dict_i].get<CKList>(db2Key::BODY);
            auto &body_dict = world_body_list.emplace_back<CKDict>();
            auto &db2b = body_dict.emplace<CKBody>(db2Key::Base);
            db2Decoder::Encode_Body(*p_b2b, db2b);

            /*.userData*/ body_dict.runtime = p_b2b;
            /*userData*/ p_b2b->GetUserData().pointer = (uintptr_t)body_dict_i;
        }

        /*fixture*/
        db2DynArray<b2Fixture *> fixtures{};
        // fixtures.reserve(b2b->GetFixtureCount(), false); // no way to get fixture count directly
        for (auto p_b2f = p_b2b->GetFixtureList(); p_b2f; p_b2f = p_b2f->GetNext())
            fixtures.push_back(p_b2f);

        for (int f = fixtures.size() - 1; f >= 0; --f) // reverse order
        {
            auto p_b2f = fixtures[f];

            auto fixture_dict_i = dicts.size();
            {
                auto &body_fixture_list = dicts[body_dict_i].get<CKList>(db2Key::FIXTURE);
                auto &fixture_dict = body_fixture_list.emplace_back<CKDict>();
                auto &db2f = fixture_dict.emplace<CKFixture>(db2Key::Base);

                db2Decoder::Encode_Fixture(*p_b2f, db2f);

                /*.userData*/ fixture_dict.runtime = p_b2f;
                /*userData*/ p_b2f->GetUserData().pointer = (uintptr_t)fixture_dict_i;
            }

            /*shape*/
            {
                auto p_b2s = p_b2f->GetShape();
                auto &fixture_shape = dicts[fixture_dict_i].emplace<CKShape>(db2Key::SHAPE);
                db2Decoder::Encode_Shpae(*p_b2s, fixture_shape);
            }
        }
    }

    /*joint*/
    db2DynArray<b2Joint *> joints{};
    joints.reserve(db2.p_b2w->GetJointCount(), false);
    for (auto p_b2j = db2.p_b2w->GetJointList(); p_b2j; p_b2j = p_b2j->GetNext())
        joints.push_back(p_b2j);

    for (int j = joints.size() - 1; j >= 0; --j) // reverse order
    {
        auto p_b2j = joints[j];

        auto joint_dict_i = dicts.size();
        {
            auto &world_joint_list = dicts[world_dict_i].get<CKList>(db2Key::JOINT);
            auto &joint_dict = world_joint_list.emplace_back<CKDict>();
            auto &db2j = joint_dict.emplace<CKJoint>(db2Key::Base);
            db2Decoder::Encode_Joint(*p_b2j, db2j);

            /*.userData*/ joint_dict.runtime = p_b2j;
            /*userData*/ p_b2j->GetUserData().pointer = (uintptr_t)joint_dict_i;
        }

        if (p_b2j->GetType() != b2JointType::e_gearJoint)
        {
            auto &joint_body_list = dicts[joint_dict_i].get<CKList>(db2Key::BODY);
            /*bodyA*/ joint_body_list.emplace_back_ref<CKBody>(p_b2j->GetBodyA()->GetUserData().pointer);
            /*bodyB*/ joint_body_list.emplace_back_ref<CKBody>(p_b2j->GetBodyB()->GetUserData().pointer);
        }
        else
        {
            auto &joint_joint_list = dicts[joint_dict_i].get<CKList>(db2Key::JOINT);
            /*joint1*/ joint_joint_list.emplace_back_ref<CKJoint>(((b2GearJoint *)p_b2j)->GetJoint1()->GetUserData().pointer);
            /*joint2*/ joint_joint_list.emplace_back_ref<CKJoint>(((b2GearJoint *)p_b2j)->GetJoint2()->GetUserData().pointer);
        }
    }
}

auto db2Decoder::Encode_World(b2World &b2w, db2World &db2w) -> void
{
    db2w.gravity_x = b2w.GetGravity().x;
    db2w.gravity_y = b2w.GetGravity().y;
}

auto db2Decoder::Encode_Body(b2Body &b2b, db2Body &db2b) -> void
{
    db2b.type = b2b.GetType();
    db2b.position_x = b2b.GetPosition().x;
    db2b.position_y = b2b.GetPosition().y;
    db2b.angle = b2b.GetAngle();
    db2b.linearVelocity_x = b2b.GetLinearVelocity().x;
    db2b.linearVelocity_y = b2b.GetLinearVelocity().y;
    db2b.angularVelocity = b2b.GetAngularVelocity();
    db2b.linearDamping = b2b.GetLinearDamping();
    db2b.angularDamping = b2b.GetAngularDamping();
    db2b.allowSleep = b2b.IsSleepingAllowed();
    db2b.awake = b2b.IsAwake();
    db2b.fixedRotation = b2b.IsFixedRotation();
    db2b.bullet = b2b.IsBullet();
    db2b.enabled = b2b.IsEnabled();

    db2b.gravityScale = b2b.GetGravityScale();
}

auto db2Decoder::Encode_Fixture(b2Fixture &b2f, db2Fixture &db2f) -> void
{
    db2f.friction = b2f.GetFriction();
    db2f.restitution = b2f.GetRestitution();
    db2f.restitutionThreshold = b2f.GetRestitutionThreshold();
    db2f.density = b2f.GetDensity();
    db2f.isSensor = b2f.IsSensor();

    /*filter*/
    db2f.filter_categoryBits = b2f.GetFilterData().categoryBits;
    db2f.filter_maskBits = b2f.GetFilterData().maskBits;
    db2f.filter_groupIndex = b2f.GetFilterData().groupIndex;
}

auto db2Decoder::Encode_Shpae(b2Shape &b2s, db2Shape &db2s) -> void
{
    /*shape_type*/ db2s.type3() = b2s.m_type;
    /*shape_radius*/ db2s.emplace_back(b2s.m_radius);

    switch (b2s.m_type)
    {
    case b2Shape::e_circle:
    {
        auto &b2s_c = reinterpret_cast<b2CircleShape &>(b2s);

        db2s.reserve(1 + 1 * 2);

        db2s.emplace_back(b2s_c.m_p.x);
        db2s.emplace_back(b2s_c.m_p.y);
    }
    break;

    case b2Shape::e_edge:
    {
        auto &b2s_e = reinterpret_cast<b2EdgeShape &>(b2s);

        db2s.reserve(1 + 4 * 2 + 1);

        db2s.emplace_back(b2s_e.m_vertex0.x);
        db2s.emplace_back(b2s_e.m_vertex0.y);
        db2s.emplace_back(b2s_e.m_vertex1.x);
        db2s.emplace_back(b2s_e.m_vertex1.y);
        db2s.emplace_back(b2s_e.m_vertex2.x);
        db2s.emplace_back(b2s_e.m_vertex2.y);
        db2s.emplace_back(b2s_e.m_vertex3.x);
        db2s.emplace_back(b2s_e.m_vertex3.y);

        db2s.emplace_back((float32_t)b2s_e.m_oneSided);
    }
    break;

    case b2Shape::e_polygon:
    {
        auto &b2s_p = reinterpret_cast<b2PolygonShape &>(b2s);

        db2s.reserve(1 + b2s_p.m_count * 2);

        for (int i = 0; i < b2s_p.m_count; ++i)
        {
            db2s.emplace_back(b2s_p.m_vertices[i].x);
            db2s.emplace_back(b2s_p.m_vertices[i].y);
        }
    }
    break;

    case b2Shape::e_chain:
    {
        auto &b2s_chain = reinterpret_cast<b2ChainShape &>(b2s);

        db2s.reserve(1 + b2s_chain.m_count * 2 + 2 * 2);

        for (int i = 0; i < b2s_chain.m_count; ++i)
        {
            db2s.emplace_back(b2s_chain.m_vertices[i].x);
            db2s.emplace_back(b2s_chain.m_vertices[i].y);
        }

        db2s.emplace_back(b2s_chain.m_prevVertex.x);
        db2s.emplace_back(b2s_chain.m_prevVertex.y);
        db2s.emplace_back(b2s_chain.m_nextVertex.x);
        db2s.emplace_back(b2s_chain.m_nextVertex.y);
    }
    break;
    }
}

auto db2Decoder::Encode_Joint(b2Joint &b2j, db2Joint &db2j) -> void
{
    /*type*/ db2j.type3() = b2j.GetType();
    /*collideConnected*/ db2j.emplace_back((float32_t)b2j.GetCollideConnected());

    switch (b2j.GetType())
    {
    case b2JointType::e_revoluteJoint:
    {
        auto &b2j_r = reinterpret_cast<b2RevoluteJoint &>(b2j);
        db2j.emplace_back(b2j_r.GetLocalAnchorA().x);
        db2j.emplace_back(b2j_r.GetLocalAnchorA().y);
        db2j.emplace_back(b2j_r.GetLocalAnchorB().x);
        db2j.emplace_back(b2j_r.GetLocalAnchorB().y);
        db2j.emplace_back(b2j_r.GetReferenceAngle());
        db2j.emplace_back((float32_t)b2j_r.IsLimitEnabled());
        db2j.emplace_back(b2j_r.GetLowerLimit());
        db2j.emplace_back(b2j_r.GetUpperLimit());
        db2j.emplace_back((float32_t)b2j_r.IsMotorEnabled());
        db2j.emplace_back(b2j_r.GetMotorSpeed());
        db2j.emplace_back(b2j_r.GetMaxMotorTorque());
    }
    break;

    case b2JointType::e_prismaticJoint:
    {
        auto &b2j_p = reinterpret_cast<b2PrismaticJoint &>(b2j);
        db2j.emplace_back(b2j_p.GetLocalAnchorA().x);
        db2j.emplace_back(b2j_p.GetLocalAnchorA().y);
        db2j.emplace_back(b2j_p.GetLocalAnchorB().x);
        db2j.emplace_back(b2j_p.GetLocalAnchorB().y);
        db2j.emplace_back(b2j_p.GetLocalAxisA().x);
        db2j.emplace_back(b2j_p.GetLocalAxisA().y);
        db2j.emplace_back(b2j_p.GetReferenceAngle());
        db2j.emplace_back((float32_t)b2j_p.IsLimitEnabled());
        db2j.emplace_back(b2j_p.GetLowerLimit());
        db2j.emplace_back(b2j_p.GetUpperLimit());
        db2j.emplace_back((float32_t)b2j_p.IsMotorEnabled());
        db2j.emplace_back(b2j_p.GetMaxMotorForce());
        db2j.emplace_back(b2j_p.GetMotorSpeed());
    }
    break;

    case b2JointType::e_distanceJoint:
    {
        auto &b2j_d = reinterpret_cast<b2DistanceJoint &>(b2j);
        db2j.emplace_back(b2j_d.GetLocalAnchorA().x);
        db2j.emplace_back(b2j_d.GetLocalAnchorA().y);
        db2j.emplace_back(b2j_d.GetLocalAnchorB().x);
        db2j.emplace_back(b2j_d.GetLocalAnchorB().y);
        db2j.emplace_back(b2j_d.GetLength());
        db2j.emplace_back(b2j_d.GetMinLength());
        db2j.emplace_back(b2j_d.GetMaxLength());
        db2j.emplace_back(b2j_d.GetStiffness());
        db2j.emplace_back(b2j_d.GetDamping());
    }
    break;

    case b2JointType::e_pulleyJoint:
    {
        auto &b2j_p = reinterpret_cast<b2PulleyJoint &>(b2j);
        db2j.emplace_back(b2j_p.GetGroundAnchorA().x);
        db2j.emplace_back(b2j_p.GetGroundAnchorA().y);
        db2j.emplace_back(b2j_p.GetGroundAnchorB().x);
        db2j.emplace_back(b2j_p.GetGroundAnchorB().y);

        auto localAnchorA = b2j_p.GetBodyA()->GetLocalPoint(b2j_p.GetAnchorA());
        db2j.emplace_back(localAnchorA.x);
        db2j.emplace_back(localAnchorA.y);
        auto localAnchorB = b2j_p.GetBodyB()->GetLocalPoint(b2j_p.GetAnchorB());
        db2j.emplace_back(localAnchorB.x);
        db2j.emplace_back(localAnchorB.y);

        db2j.emplace_back(b2j_p.GetLengthA());
        db2j.emplace_back(b2j_p.GetLengthB());
        db2j.emplace_back(b2j_p.GetRatio());
    }
    break;

    case b2JointType::e_mouseJoint:
    {
        auto &b2j_m = reinterpret_cast<b2MouseJoint &>(b2j);
        db2j.emplace_back(b2j_m.GetTarget().x);
        db2j.emplace_back(b2j_m.GetTarget().y);
        db2j.emplace_back(b2j_m.GetMaxForce());
        db2j.emplace_back(b2j_m.GetStiffness());
        db2j.emplace_back(b2j_m.GetDamping());
    }
    break;

    case b2JointType::e_gearJoint:
    {
        auto &b2j_g = reinterpret_cast<b2GearJoint &>(b2j);
        // db2j.emplace_back((float32_t)b2j_g.GetJoint1()->GetUserData().pointer);
        // db2j.emplace_back((float32_t)b2j_g.GetJoint2()->GetUserData().pointer);
        db2j.emplace_back(b2j_g.GetRatio());
    }
    break;

    case b2JointType::e_wheelJoint:
    {
        auto &b2j_w = reinterpret_cast<b2WheelJoint &>(b2j);
        db2j.emplace_back(b2j_w.GetLocalAnchorA().x);
        db2j.emplace_back(b2j_w.GetLocalAnchorA().y);
        db2j.emplace_back(b2j_w.GetLocalAnchorB().x);
        db2j.emplace_back(b2j_w.GetLocalAnchorB().y);
        db2j.emplace_back(b2j_w.GetLocalAxisA().x);
        db2j.emplace_back(b2j_w.GetLocalAxisA().y);
        db2j.emplace_back((float32_t)b2j_w.IsLimitEnabled());
        db2j.emplace_back(b2j_w.GetLowerLimit());
        db2j.emplace_back(b2j_w.GetUpperLimit());
        db2j.emplace_back((float32_t)b2j_w.IsMotorEnabled());
        db2j.emplace_back(b2j_w.GetMaxMotorTorque());
        db2j.emplace_back(b2j_w.GetMotorSpeed());
        db2j.emplace_back(b2j_w.GetStiffness());
        db2j.emplace_back(b2j_w.GetDamping());
    }
    break;

    case b2JointType::e_weldJoint:
    {
        auto &b2j_wd = reinterpret_cast<b2WeldJoint &>(b2j);
        db2j.emplace_back(b2j_wd.GetLocalAnchorA().x);
        db2j.emplace_back(b2j_wd.GetLocalAnchorA().y);
        db2j.emplace_back(b2j_wd.GetLocalAnchorB().x);
        db2j.emplace_back(b2j_wd.GetLocalAnchorB().y);
        db2j.emplace_back(b2j_wd.GetReferenceAngle());
        db2j.emplace_back(b2j_wd.GetStiffness());
        db2j.emplace_back(b2j_wd.GetDamping());
    }
    break;

    case b2JointType::e_frictionJoint:
    {
        auto &b2j_f = reinterpret_cast<b2FrictionJoint &>(b2j);
        db2j.emplace_back(b2j_f.GetLocalAnchorA().x);
        db2j.emplace_back(b2j_f.GetLocalAnchorA().y);
        db2j.emplace_back(b2j_f.GetLocalAnchorB().x);
        db2j.emplace_back(b2j_f.GetLocalAnchorB().y);
        db2j.emplace_back(b2j_f.GetMaxForce());
        db2j.emplace_back(b2j_f.GetMaxTorque());
    }
    break;

    case b2JointType::e_ropeJoint:
    {
        // auto b2j_r = (b2RopeJoint *)b2j;
    }
    break;

    case b2JointType::e_motorJoint:
    {
        auto &b2j_m = reinterpret_cast<b2MotorJoint &>(b2j);
        db2j.emplace_back(b2j_m.GetLinearOffset().x);
        db2j.emplace_back(b2j_m.GetLinearOffset().y);
        db2j.emplace_back(b2j_m.GetAngularOffset());
        db2j.emplace_back(b2j_m.GetMaxForce());
        db2j.emplace_back(b2j_m.GetMaxTorque());
        db2j.emplace_back(b2j_m.GetCorrectionFactor());
    }
    break;

    default:
        break;
    }
}
