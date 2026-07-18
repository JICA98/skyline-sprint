#ifndef WORLD_H
#define WORLD_H

#include "physics.h"
#include "util/random.h"
#include <vector>
#include <SDL2/SDL.h>

constexpr float CHUNK_WIDTH = 1920.0f; // Width of one chunk in logical pixels
constexpr int MAX_ACTIVE_CHUNKS = 3;

struct Platform {
    AABB box;
    bool isHazard;
};

struct Collectible {
    AABB box;
    bool active;
};

struct Drone {
    AABB box;
    float startY;
    float animTime;
};

struct Chunk {
    int index;
    float startX;
    std::vector<Platform> platforms;
    std::vector<Collectible> shards;
    std::vector<Drone> drones;
};

class World {
public:
    World();
    void Reset(uint32_t seed, Random& prng);
    void Update(float playerX, Random& prng);

    // Physics collision resolver
    void ResolvePlayerCollision(class Player& player);

    // Shard collision checker
    int CheckCollectibleCollisions(const AABB& playerBox);

    // Visual hazard collision checker
    bool CheckHazardCollision(const AABB& playerBox);

    // Origin rebasing to avoid floating point precision decay
    void Rebase(float offset, class Player& player, float& cameraX);

    void Render(SDL_Renderer* renderer, SDL_Texture* platformTex, SDL_Texture* warningTex, SDL_Texture* shardTex, SDL_Texture* enemyTex, float cameraX, float cameraY = 0.0f);

    int GetFallbackCount() const { return fallbackCount; }
    float GetLastGeneratedX() const { return lastGeneratedX; }

private:
    void GenerateChunk(int index, float startX, Random& prng);
    bool ValidateChunk(const Chunk& chunk);
    void GenerateFallbackChunk(Chunk& chunk);

    std::vector<Chunk> activeChunks;
    int fallbackCount;
    float lastGeneratedX;
    int nextChunkIndex;
};

#endif // WORLD_H
