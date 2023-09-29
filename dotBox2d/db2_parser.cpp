#include "db2_parser.h"

#include <assert.h>

auto dotB2Parser::parse() -> void
{
    auto &db2v = db2->chunks.vector;

    auto &db2w = db2->chunks.world[0];
    b2Vec2 gravity{db2w.gravity_x, db2w.gravity_y};
    this->b2w = new b2World{gravity};

    for (auto i = db2w.body_begin; i <= db2w.body_end; ++i)
    {
        auto &db2b = db2->chunks.body[i];
        b2BodyDef b2bdef{};

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

                b2fdef.shape = &shape;
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

                b2fdef.shape = &shape;
                b2fdef.userData.pointer = (uintptr_t)&db2f;
                db2f.userData = (uint64_t)b2b->CreateFixture(&b2fdef);
            }
            break;
            }
        }
    };
}