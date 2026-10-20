#include "MapObstacles.h"
#include "MapLayout.generated.h"
#include "Consts.h"
#include "LowPolyModels.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
    constexpr float skin = 0.25f;
    bool AtHeight(const MapLayout::Rock& rock, float y)
    {
        return y >= -8.0f && y <= rock.top + GameConsts::FLOOR_MODEL_Y + 1.0f;
    }
    Vec3 Flat(Vec3 v) { v.y = 0.0f; return v; }
    Vec3 ClampArena(Vec3 p, float radius)
    {
        const float limit = GameConsts::ARENA_HALF_SIZE - radius - 1.0f;
        p.x = std::clamp(p.x, -limit, limit);
        p.z = std::clamp(p.z, -limit, limit);
        return p;
    }
    Vec3 PushOut(Vec3 p, float radius)
    {
        const Vec3 requested = p;
        for (int pass = 0; pass < 8; ++pass)
        {
            bool changed = false;
            for (const auto& rock : MapLayout::rocks)
            {
                if (!AtHeight(rock, p.y)) continue;
                Vec3 d{ p.x-rock.x, 0, p.z-rock.z };
                const float rr = rock.radius + radius + skin;
                if (d.LengthSq() >= rr*rr) continue;
                d = d.LengthSq() > 0.0001f ? d.Normalized() : Vec3{1,0,0};
                p.x = rock.x + d.x * rr;
                p.z = rock.z + d.z * rr;
                changed = true;
            }
            p = ClampArena(p, radius);
            if (!changed) break;
        }
        const auto free = [radius](const Vec3& candidate)
        {
            const float limit=GameConsts::ARENA_HALF_SIZE-radius-1.0f;
            if (std::fabs(candidate.x)>limit || std::fabs(candidate.z)>limit) return false;
            for (const auto& rock:MapLayout::rocks)
            {
                if (!AtHeight(rock,candidate.y)) continue;
                const float x=candidate.x-rock.x, z=candidate.z-rock.z;
                const float rr=rock.radius+radius+skin-0.001f;
                if (x*x+z*z<rr*rr) return false;
            }
            return true;
        };
        // Intersecting footprints can bounce a simple push-out between two rocks.
        // Find the closest exposed boundary when a landing/spawn starts inside that union.
        if (!free(p))
        {
            float best=std::numeric_limits<float>::max();
            for (const auto& rock:MapLayout::rocks)
            {
                if (!AtHeight(rock,p.y)) continue;
                for (int i=0;i<64;++i)
                {
                    const float angle=i*6.283185307f/64;
                    const float rr=rock.radius+radius+skin+0.01f;
                    const Vec3 candidate{rock.x+std::cos(angle)*rr,p.y,rock.z+std::sin(angle)*rr};
                    const float d=Flat(candidate-requested).LengthSq();
                    if (d<best && free(candidate)) { best=d; p=candidate; }
                }
            }
        }
        return p;
    }
    bool ClearSegment(const Vec3& a, const Vec3& b, float radius)
    {
        const Vec3 delta = Flat(b-a);
        const float lengthSq = delta.LengthSq();
        for (const auto& rock : MapLayout::rocks)
        {
            if (!AtHeight(rock, a.y)) continue;
            const Vec3 rel{rock.x-a.x, 0, rock.z-a.z};
            const float t = lengthSq > 0.0001f ? std::clamp(Vec3::Dot(rel,delta)/lengthSq,0.0f,1.0f) : 0;
            const float rr = rock.radius + radius + skin;
            if ((rel-delta*t).LengthSq() < rr*rr) return false;
        }
        return true;
    }

    // Shared ground flow field, rebuilt at most five times/second. Fixed storage;
    // conservative cell padding keeps the largest normal enemy clear of corners.
    constexpr int side = 41, cells = side*side;
    constexpr float step = 34.0f, origin = -680.0f;
    constexpr unsigned short unreachable = 65535;
    std::array<bool,cells> blocked{};
    std::array<unsigned short,cells> distanceField{};
    std::array<int,cells> queue{};
    bool initialized = false;
    float refresh = 0;
    Vec3 Cell(int i) { return Vec3{origin+(i%side)*step,0,origin+(i/side)*step}; }
    int Index(const Vec3& p)
    {
        const int x = std::clamp(static_cast<int>(std::lround((p.x-origin)/step)),0,side-1);
        const int z = std::clamp(static_cast<int>(std::lround((p.z-origin)/step)),0,side-1);
        return z*side+x;
    }
}

bool MapObstacles::Enabled()
{
    return GameConsts::FLOOR_USE_MODEL && LowPolyModels::GetHandle(LowPolyId::Floor) >= 0;
}

bool MapObstacles::Overlaps(const Vec3& p, float radius)
{
    if (!Enabled()) return false;
    for (const auto& rock : MapLayout::rocks)
    {
        if (!AtHeight(rock,p.y)) continue;
        const float x = p.x-rock.x, z = p.z-rock.z, rr = rock.radius+radius+skin;
        if (x*x+z*z < rr*rr) return true;
    }
    return false;
}

Vec3 MapObstacles::Resolve(const Vec3& previous, const Vec3& desired, float radius)
{
    if (!Enabled()) return desired;
    Vec3 p = PushOut(previous,radius);
    Vec3 remaining = Flat(desired-previous);
    for (int iteration=0; iteration<5 && remaining.LengthSq()>0.00001f; ++iteration)
    {
        float first = 1.0f;
        Vec3 normal{};
        for (const auto& rock : MapLayout::rocks)
        {
            const Vec3 rel{p.x-rock.x,0,p.z-rock.z};
            const float rr = rock.radius+radius+skin;
            const float a = remaining.LengthSq(), b = Vec3::Dot(rel,remaining);
            const float c = rel.LengthSq()-rr*rr;
            const float discriminant = b*b-a*c;
            if (b>=0 || discriminant<0) continue;
            const float t = (std::max)(0.0f,(-b-std::sqrt(discriminant))/a);
            const float y = previous.y+(desired.y-previous.y)*t;
            if (t>first || !AtHeight(rock,y)) continue;
            first = t;
            normal = (rel+remaining*t).Normalized();
        }
        p += remaining*first;
        remaining *= 1.0f-first;
        if (first>=1.0f) break;
        p += normal*0.01f;
        const float into = Vec3::Dot(remaining,normal);
        if (into<0) remaining -= normal*into;
    }
    p.y = desired.y;
    // Landing on a reef moves the actor to its edge; reefs are not platforms.
    return PushOut(ClampArena(p,radius),radius);
}

Vec3 MapObstacles::Move(const Vec3& p, const Vec3& delta, float radius)
{
    return Resolve(p,p+delta,radius);
}

void MapObstacles::UpdateNavigation(const Vec3& target, float dt)
{
    if (!Enabled()) { initialized=false; refresh=0; return; }
    if (!initialized)
    {
        for (int i=0;i<cells;++i) blocked[i]=Overlaps(Cell(i),49.0f);
        initialized=true;
    }
    refresh-=dt;
    if (refresh>0) return;
    refresh=0.2f;
    int goal=Index(target);
    if (blocked[goal])
    {
        float closest=std::numeric_limits<float>::max();
        for (int i=0;i<cells;++i) if (!blocked[i])
        {
            const float d=Flat(Cell(i)-target).LengthSq();
            if (d<closest) { closest=d; goal=i; }
        }
    }
    distanceField.fill(unreachable);
    int head=0,tail=0;
    queue[tail++]=goal; distanceField[goal]=0;
    while (head<tail)
    {
        const int i=queue[head++], x=i%side, z=i/side;
        const int neighbours[]={x>0?i-1:-1,x<side-1?i+1:-1,z>0?i-side:-1,z<side-1?i+side:-1};
        for (int n:neighbours) if (n>=0 && !blocked[n] && distanceField[n]==unreachable)
        {
            distanceField[n]=static_cast<unsigned short>(distanceField[i]+1);
            queue[tail++]=n;
        }
    }
}

Vec3 MapObstacles::ChaseDirection(const Vec3& p, const Vec3& target, float radius)
{
    const Vec3 direct=Flat(target-p).Normalized();
    if (!Enabled() || !initialized || ClearSegment(p,target,radius+2)) return direct;
    const int index=Index(p), cx=index%side, cz=index/side;
    float best=std::numeric_limits<float>::max();
    Vec3 waypoint=p;
    // Look ahead across several cells, smoothing the four-neighbour field.
    for (int z=(std::max)(0,cz-3);z<=(std::min)(side-1,cz+3);++z)
        for (int x=(std::max)(0,cx-3);x<=(std::min)(side-1,cx+3);++x)
        {
            const int i=z*side+x;
            if (blocked[i] || distanceField[i]==unreachable) continue;
            Vec3 candidate=Cell(i); candidate.y=p.y;
            const float travel=Flat(candidate-p).Length();
            if (travel<2 || !ClearSegment(p,candidate,radius+2)) continue;
            const float score=distanceField[i]*step+travel*0.85f;
            if (score<best) { best=score; waypoint=candidate; }
        }
    return best<std::numeric_limits<float>::max() ? Flat(waypoint-p).Normalized() : direct;
}
