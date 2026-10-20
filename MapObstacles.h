#pragma once
#include "Vector3.h"

// Solid reef footprints. Heights are measured from the actor's ground anchor.
// All queries become pass-through when the floor model is unavailable.
namespace MapObstacles
{
    bool Enabled();
    bool Overlaps(const Vec3& position, float radius);
    Vec3 Resolve(const Vec3& previous, const Vec3& desired, float radius);
    Vec3 Move(const Vec3& position, const Vec3& delta, float radius);
    void UpdateNavigation(const Vec3& target, float dt);
    Vec3 SteerDirection(const Vec3& position, const Vec3& desired, float radius);
    Vec3 ChaseDirection(const Vec3& position, const Vec3& target, float radius);
}
