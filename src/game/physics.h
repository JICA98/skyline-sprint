#ifndef PHYSICS_H
#define PHYSICS_H

struct AABB {
    float x, y;
    float w, h;
};

// Check if two boxes overlap
bool Physics_AABBIntersect(const AABB& a, const AABB& b);

// Check if box overlaps with any horizontal or vertical line/box
float Physics_GetOverlapX(const AABB& a, const AABB& b);
float Physics_GetOverlapY(const AABB& a, const AABB& b);

#endif // PHYSICS_H
