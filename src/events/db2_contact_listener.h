#pragma once

#include <functional>

#include "box2d/box2d.h"

#include "containers/db2_dynarray.h"

class db2ContactListener : public b2ContactListener
{
public:
    db2DynArray<std::function<void(b2Contact *)>> onBeginContact;
    db2DynArray<std::function<void(b2Contact *)>> onEndContact;
    db2DynArray<std::function<void(b2Contact *, const b2Manifold *)>> onPreSolve;
    db2DynArray<std::function<void(b2Contact *, const b2ContactImpulse *)>> onPostSolve;

public:
    auto BeginContact(b2Contact *contact) -> void override;
    auto EndContact(b2Contact *contact) -> void override;
    auto PreSolve(b2Contact *contact, const b2Manifold *oldManifold) -> void override;
    auto PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) -> void override;
};
