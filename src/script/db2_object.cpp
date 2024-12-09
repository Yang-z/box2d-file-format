#include "db2_object.h"
#include "data/db2_key.h"
#include "box2d/box2d.h"

auto db2Object::Get(void *ptr, int32_t type, int32_t property, db2Tensor<float32_t> &result) -> std::function<void()>
{

    switch (type)
    {
    case int32_t(db2Key::BODY):
    {
        switch (property)
        {
        case int32_t(db2Key::Position):
        {
            return [ptr, &result]() -> void
            {
                auto pos = static_cast<b2Body *>(ptr)->GetPosition();
                result = {pos.x, pos.y};
            };
        }
        break;
        case int32_t(db2Key::Angle):
        {
            return [ptr, &result]() -> void
            {
                auto angle = static_cast<b2Body *>(ptr)->GetAngle();
                result = angle;
            };
        }
        break;
        }
    }

    break;

    default:
        return []() {};
        break;
    }
}