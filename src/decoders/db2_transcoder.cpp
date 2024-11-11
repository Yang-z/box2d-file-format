#include "db2_transcoder.h"

auto db2Transcoder::Transcode(dotBox2d &db2) -> void
{
    if (db2.p_db2ContactListener || db2.p_db2OffstepListener)
        return;

    db2.p_db2ContactListener = new db2ContactListener();
    db2.p_db2OffstepListener = new db2OffstepListener();

    auto &dicts = db2.chunks.get<CKDict>();

    for (auto d = 0; d < dicts.size(); ++d)
    {
        auto &dict = dicts[d];
        char *target_type = nullptr;
        for (auto e = 0; e < dict.size(); ++e)
        {
            auto &elem = dict[e];
            switch (elem.key)
            {
            case db2Key::Base:
            {
                target_type = &elem.type0;
            }
            break;

            case db2Key::BreakForce:
            {
                if (db2Reflector::IsRefOf<CKBody>(target_type))
                    db2Transcoder::Transcode_BreakForce_Body(db2, reinterpret_cast<b2Body *&>(dict.runtime), reinterpret_cast<float32_t &>(elem.value));
                else if (db2Reflector::IsRefOf<CKJoint>(target_type))
                    db2Transcoder::Transcode_BreakForce_Joint(db2, reinterpret_cast<b2Joint *&>(dict.runtime), reinterpret_cast<float32_t &>(elem.value));
            }
            break;

            default:
                break;
            }
        }
    }
}

auto db2Transcoder::Transcode_BreakForce_Body(dotBox2d &db2, b2Body *&b2_body, float32_t &break_force) -> void
{
    db2.p_db2ContactListener->onPostSolve.emplace_back(
        [&db2, b2_body, break_force](b2Contact *contact, const b2ContactImpulse *impulse) mutable -> void
        {
            if (b2_body != contact->GetFixtureA()->GetBody() && b2_body != contact->GetFixtureB()->GetBody())
                return;

            float32_t impulse_sum = 0.0f;
            for (int i = 0; i < impulse->count; ++i)
                impulse_sum += impulse->normalImpulses[i];

            if (impulse_sum * db2.inv_dt > break_force)
            {
                db2.p_b2w->DestroyBody(b2_body);
                b2_body = nullptr;
            }
        } //
    );
}

auto db2Transcoder::Transcode_BreakForce_Joint(dotBox2d &db2, b2Joint *&b2_joint, float32_t &break_force) -> void
{
    db2.p_db2OffstepListener->onPostStep.emplace_back(
        [&db2, b2_joint, break_force]() mutable -> bool
        {
            if (b2_joint && b2_joint->GetReactionForce(db2.inv_dt).Length() > break_force)
            {
                db2.p_b2w->DestroyJoint(b2_joint);
                b2_joint == nullptr;
                return true;
            }

            return false;
        } //
    );
}