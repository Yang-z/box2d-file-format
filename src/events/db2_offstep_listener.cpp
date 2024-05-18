#include "db2_offstep_listener.h"

auto db2OffstepListener::PreStep() -> void
{
    for (auto i = 0; i < this->onPreStep.size(); ++i)
        if (this->onPreStep[i]())
            this->onPreStep[i] = nullptr;
}

auto db2OffstepListener::PostStep() -> void
{
    for (auto i = 0; i < onPostStep.size(); ++i)
        if (this->onPostStep[i]())
            this->onPostStep[i] = nullptr;
}