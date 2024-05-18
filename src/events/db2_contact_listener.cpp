#include "db2_contact_listener.h"

auto db2ContactListener::BeginContact(b2Contact *contact) -> void
{
    for (auto i = 0; i < onBeginContact.size(); ++i)
        this->onBeginContact[i](contact);
}

auto db2ContactListener::EndContact(b2Contact *contact) -> void
{
    for (auto i = 0; i < onEndContact.size(); ++i)
        this->onEndContact[i](contact);
}

auto db2ContactListener::PreSolve(b2Contact *contact, const b2Manifold *oldManifold) -> void
{
    for (auto i = 0; i < onPreSolve.size(); ++i)
        this->onPreSolve[i](contact, oldManifold);
}

auto db2ContactListener::PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) -> void
{
    for (auto i = 0; i < onPostSolve.size(); ++i)
        this->onPostSolve[i](contact, impulse);
}