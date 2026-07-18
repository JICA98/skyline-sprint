#include "physics.h"
#include <algorithm>

bool Physics_AABBIntersect(const AABB& a, const AABB& b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

float Physics_GetOverlapX(const AABB& a, const AABB& b) {
    float overlap1 = (b.x + b.w) - a.x;
    float overlap2 = (a.x + a.w) - b.x;
    if (overlap1 <= 0.0f || overlap2 <= 0.0f) return 0.0f;
    return (overlap1 < overlap2) ? overlap1 : -overlap2;
}

float Physics_GetOverlapY(const AABB& a, const AABB& b) {
    float overlap1 = (b.y + b.h) - a.y;
    float overlap2 = (a.y + a.h) - b.y;
    if (overlap1 <= 0.0f || overlap2 <= 0.0f) return 0.0f;
    return (overlap1 < overlap2) ? overlap1 : -overlap2;
}
