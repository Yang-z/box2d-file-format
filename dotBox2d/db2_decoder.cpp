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

    for (auto i = db2w.body_begin; i <= db2w.body_end; ++i)
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

        b2bdef.userData.pointer = (uintptr_t)&db2b;

        auto *b2b = b2w->CreateBody(&b2bdef);
        db2b.userData = (uint64_t)b2b;

        for (auto j = db2b.fixture_begin; j <= db2b.fixture_end; j++)
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

            auto p0 = db2f.shape_vec_begin;
            auto pe = db2f.shape_vec_end;

            switch ((b2Shape::Type)db2f.shape_type)
            {

            case b2Shape::Type::e_circle:
            {
                assert(pe - p0 + 1 >= 2);
                auto shape = b2CircleShape();
                shape.m_radius = db2f.shape_radius;

                shape.m_p = {db2v[p0++], db2v[p0++]};

                b2fdef.shape = &shape; // The shape will be cloned
                b2fdef.userData.pointer = (uintptr_t)&db2f;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_edge:
            {
                assert(pe - p0 + 1 >= 9);

                auto shape = b2EdgeShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                shape.m_vertex0 = {db2v[p0++], db2v[p0++]};
                shape.m_vertex1 = {db2v[p0++], db2v[p0++]};
                shape.m_vertex2 = {db2v[p0++], db2v[p0++]};
                shape.m_vertex3 = {db2v[p0++], db2v[p0++]};
                shape.m_oneSided = (bool)db2v[p0++];

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)&db2f;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_polygon:
            {

                assert((pe - p0 + 1) % 2 == 0);
                auto count = (pe - p0 + 1) / 2 - 1;
                assert(3 <= count <= b2_maxPolygonVertices);

                auto shape = b2PolygonShape();
                shape.m_radius = db2f.shape_radius; // default: b2_polygonRadius

                shape.m_centroid = {db2v[p0++], db2v[p0++]};
                auto points = (b2Vec2 *)(&(db2v[p0]));
                shape.Set(points, count);

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)&db2f;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;

            case b2Shape::e_chain:
            {
                assert((pe - p0 + 1) % 2 == 0);

                auto shape = b2ChainShape();
                shape.m_radius = db2f.shape_radius;

                /*
                CreateChain or CreateLoop?
                Since we record the "fusion point", m_prevVertex and m_nextVertex,
                they are the same.
                */

                // auto count = (pe - p0 + 1) / 2 - 2;
                // auto points = (b2Vec2 *)(&(db2v[p0]));
                // shape.CreateChain(
                //     points,
                //     count,
                //     {db2v[p0 + count * 2 + 0], db2v[p0 + count * 2 + 1]},
                //     {db2v[p0 + count * 2 + 2], db2v[p0 + count * 2 + 3]});

                shape.m_vertices = (b2Vec2 *)(&(db2v[p0]));
                shape.m_count = (pe - p0 + 1) / 2 - 2;
                shape.m_prevVertex = {db2v[pe - 3], db2v[pe - 2]};
                shape.m_nextVertex = {db2v[pe - 1], db2v[pe - 0]};

                /*do some distance check here?*/

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)&db2f;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;
            }
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
        _db2->chunks.body.size + this->b2w->GetBodyCount() - 1,

        _db2->chunks.joint.size,
        _db2->chunks.joint.size + this->b2w->GetJointCount() - 1);

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
            -1, //

            (uint64_t)b2b);

        /*fixture*/
        db2Container<b2Fixture *> fixtures{};
        // fixtures.reserve(b2b->GetFixtureCount(), false); // no way to get fixture count directly
        for (auto b2f = b2b->GetFixtureList(); b2f; b2f = b2f->GetNext())
            fixtures.push(b2f);

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
                -1, //

                (uint64_t)b2f);

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

                _db2->chunks.vector.reserve(_db2->chunks.vector.size + 1 * 2 + b2s_p->m_count * 2);

                _db2->chunks.vector.emplace_back(b2s_p->m_centroid.x);
                _db2->chunks.vector.emplace_back(b2s_p->m_centroid.y);

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
            _db2->chunks.fixture[-1].shape_vec_end = _db2->chunks.vector.size - 1;
        }
        _db2->chunks.body[-1].fixture_end = _db2->chunks.fixture.size - 1;
    }
    delete this->db2;
    this->db2 = _db2;
}