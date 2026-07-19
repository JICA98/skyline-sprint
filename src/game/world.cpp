#include "world.h"
#include "player.h"
#include "constants.h"
#include "util/logger.h"
#include "audio/audio_manager.h"
#include <math.h>
#include <iostream>
#include <algorithm>

using namespace PhysicsTuning;

World::World() : fallbackCount(0), lastGeneratedX(0.0f), nextChunkIndex(0) {
    activeChunks.reserve(MAX_ACTIVE_CHUNKS);
}

void World::Reset(uint32_t seed, Random& prng) {
    (void)seed;
    activeChunks.clear();
    fallbackCount = 0;
    lastGeneratedX = 0.0f;
    nextChunkIndex = 0;

    // Generate initial safe chunk (Index 0)
    Chunk startChunk;
    startChunk.platforms.reserve(8);
    startChunk.shards.reserve(16);
    startChunk.drones.reserve(4);
    startChunk.index = nextChunkIndex++;
    startChunk.startX = 0.0f;
    // Flat safe starting zone
    Platform startPlat = { { 0.0f, 600.0f, CHUNK_WIDTH, 480.0f }, false };
    startChunk.platforms.push_back(startPlat);
    activeChunks.push_back(startChunk);
    lastGeneratedX = CHUNK_WIDTH;

    // Generate next 2 chunks ahead using PRNG
    for (int i = 0; i < 2; ++i) {
        GenerateChunk(nextChunkIndex++, lastGeneratedX, prng);
        lastGeneratedX += CHUNK_WIDTH;
    }
}

void World::Update(float playerX, Random& prng) {
    // 1. Update bobbing animation for all drones in active chunks
    for (auto& chunk : activeChunks) {
        for (auto& drone : chunk.drones) {
            drone.animTime += DT * 4.0f; // Bobbing rate
            drone.box.y = drone.startY + sinf(drone.animTime) * 35.0f;
        }
    }

    // 2. If player crosses the boundary of the current chunk, retire oldest and generate next
    if (activeChunks.size() >= 2 && playerX > activeChunks[1].startX) {
        // Retire chunk behind (activeChunks[0])
        activeChunks.erase(activeChunks.begin());

        // Generate next chunk ahead
        GenerateChunk(nextChunkIndex++, lastGeneratedX, prng);
        lastGeneratedX += CHUNK_WIDTH;
    }
}

void World::GenerateChunk(int index, float startX, Random& prng) {
    Chunk chunk;
    chunk.platforms.reserve(8);
    chunk.shards.reserve(16);
    chunk.drones.reserve(4);
    chunk.index = index;
    chunk.startX = startX;

    // Pick a random pattern family (1 to 5)
    int pattern = prng.Range(1, 6);

    if (pattern == 1) {
        // Disjoint rooftops: 3 rooftops with gaps
        float curX = startX;
        float platY = 600.0f;
        for (int i = 0; i < 3; ++i) {
            float width = static_cast<float>(prng.Range(300, 500));
            Platform plat = { { curX, platY, width, 480.0f }, false };
            chunk.platforms.push_back(plat);

            // Shards on platforms
            for (int k = 0; k < 3; ++k) {
                Collectible shard = { { curX + 50.0f + k * 100.0f, platY - 50.0f, 16.0f, 16.0f }, true };
                chunk.shards.push_back(shard);
            }

            float gapSize = static_cast<float>(prng.Range(150, 280));
            if (i < 2) {
                float gapX = curX + width;
                Drone drone = { { gapX + gapSize / 2.0f - 16.0f, platY - 140.0f, 32.0f, 32.0f }, platY - 140.0f, 0.0f };
                chunk.drones.push_back(drone);
            }
            curX += width + gapSize;
        }
    }
    else if (pattern == 2) {
        // Stepped platforms: going up
        float curX = startX;
        float platY = 700.0f;
        for (int i = 0; i < 3; ++i) {
            float width = static_cast<float>(prng.Range(350, 450));
            Platform plat = { { curX, platY, width, 480.0f }, false };
            chunk.platforms.push_back(plat);

            // Spawn drone above step 1/2
            if (i == 1) {
                Drone drone = { { curX + width / 2.0f - 16.0f, platY - 140.0f, 32.0f, 32.0f }, platY - 140.0f, 0.0f };
                chunk.drones.push_back(drone);
            }

            // Double jump shard arc in the gap
            if (i < 2) {
                float gapX = curX + width;
                float gapW = static_cast<float>(prng.Range(180, 250));
                for (int k = 0; k < 3; ++k) {
                    float sx = gapX + k * (gapW / 4.0f) + 30.0f;
                    float sy = platY - 100.0f - sinf((k / 2.0f) * M_PI) * 80.0f;
                    Collectible shard = { { sx, sy, 16.0f, 16.0f }, true };
                    chunk.shards.push_back(shard);
                }
            }

            platY -= 80.0f; // Step up (safe: 80 px)
            curX += width + static_cast<float>(prng.Range(180, 250));
        }
    }
    else if (pattern == 3) {
        // Obstacle rooftops: Flat with hazard walls
        float width = CHUNK_WIDTH;
        Platform base = { { startX, 600.0f, width, 480.0f }, false };
        chunk.platforms.push_back(base);

        // Add 2 hazard spikes/walls
        float hx1 = startX + 500.0f;
        Platform hazard1 = { { hx1, 540.0f, 32.0f, 60.0f }, true }; // 60px height spike
        chunk.platforms.push_back(hazard1);

        float hx2 = startX + 1200.0f;
        Platform hazard2 = { { hx2, 540.0f, 32.0f, 60.0f }, true };
        chunk.platforms.push_back(hazard2);

        // Shards before and after obstacles
        Collectible s1 = { { hx1 - 100.0f, 500.0f, 16.0f, 16.0f }, true };
        Collectible s2 = { { hx1 + 100.0f, 500.0f, 16.0f, 16.0f }, true };
        chunk.shards.push_back(s1);
        chunk.shards.push_back(s2);
    }
    else if (pattern == 4) {
        // Floating platforms
        float curX = startX;
        float platY = 650.0f;
        for (int i = 0; i < 3; ++i) {
            float width = static_cast<float>(prng.Range(300, 400));
            Platform plat = { { curX, platY, width, 50.0f }, false }; // Floating slab (50px thickness)
            chunk.platforms.push_back(plat);

            // Floating island decoration underneath
            Platform dec = { { curX + 50.0f, platY + 50.0f, width - 100.0f, 30.0f }, false };
            chunk.platforms.push_back(dec);

            // Shards on top
            Collectible s = { { curX + width / 2.0f, platY - 40.0f, 16.0f, 16.0f }, true };
            chunk.shards.push_back(s);

            float gapSize = static_cast<float>(prng.Range(200, 300));
            if (i < 2) {
                float gapX = curX + width;
                Drone drone = { { gapX + gapSize / 2.0f - 16.0f, platY - 100.0f, 32.0f, 32.0f }, platY - 100.0f, 0.0f };
                chunk.drones.push_back(drone);
            }
            curX += width + gapSize;
            platY += static_cast<float>(prng.Range(0, 120)) - 60.0f;
        }
    }
    else {
        // Flat solid rooftop with high shards
        Platform base = { { startX, 600.0f, CHUNK_WIDTH, 480.0f }, false };
        chunk.platforms.push_back(base);
        for (int k = 0; k < 5; ++k) {
            Collectible s = { { startX + 200.0f + k * 300.0f, 500.0f, 16.0f, 16.0f }, true };
            chunk.shards.push_back(s);
        }
    }

    // Validate generated layout. If impossible, fallback to flat safe rooftop
    if (!ValidateChunk(chunk)) {
        GenerateFallbackChunk(chunk);
    }

    activeChunks.push_back(chunk);
}

bool World::ValidateChunk(const Chunk& chunk) {
    // Check transitions between consecutive platforms
    for (size_t i = 0; i < chunk.platforms.size(); ++i) {
        if (chunk.platforms[i].isHazard) continue;

        // Compare against next non-hazard platform
        for (size_t j = i + 1; j < chunk.platforms.size(); ++j) {
            if (chunk.platforms[j].isHazard) continue;

            const AABB& p1 = chunk.platforms[i].box;
            const AABB& p2 = chunk.platforms[j].box;

            // If j is adjacent horizontally (p2.x > p1.x)
            if (p2.x > p1.x + p1.w) {
                float gap = p2.x - (p1.x + p1.w);
                // Maximum safe jump distance is 350 pixels
                if (gap > 350.0f) {
                    return false;
                }

                // Vertical delta going up cannot exceed 160 pixels
                float heightDelta = p1.y - p2.y; // positive means step up
                if (heightDelta > 160.0f) {
                    return false;
                }
                break; // Checked next adjacent platform
            }
        }
    }
    return true;
}

void World::GenerateFallbackChunk(Chunk& chunk) {
    fallbackCount++;
    Logger::Log(LogLevel::Warning, "World", "Impossible layout generated. Deploying fallback chunk " + std::to_string(chunk.index));

    chunk.platforms.clear();
    chunk.shards.clear();

    // Solid safe rooftop platform across the entire chunk
    Platform flat = { { chunk.startX, 600.0f, CHUNK_WIDTH, 480.0f }, false };
    chunk.platforms.push_back(flat);

    // Place a few shards safely
    for (int i = 0; i < 4; ++i) {
        Collectible s = { { chunk.startX + 300.0f + i * 400.0f, 520.0f, 16.0f, 16.0f }, true };
        chunk.shards.push_back(s);
    }
}

void World::ResolvePlayerCollision(Player& player) {
    AABB pBox = player.GetAABB();

    // 1. Move player on X axis and check collisions
    player.x += player.vx * DT;
    pBox.x = player.x;

    for (const auto& chunk : activeChunks) {
        for (const auto& plat : chunk.platforms) {
            if (plat.isHazard) continue; // Resolved in hazard checker instead

            if (Physics_AABBIntersect(pBox, plat.box)) {
                float overlapX = Physics_GetOverlapX(pBox, plat.box);
                // Resolve horizontal overlap
                player.x += overlapX;
                pBox.x = player.x;
                player.vx = 0.0f;
            }
        }
    }

    // 2. Move player on Y axis and check collisions
    player.y += player.vy * DT;
    pBox.y = player.y;
    bool wasGrounded = player.grounded;
    player.grounded = false;

    for (const auto& chunk : activeChunks) {
        for (const auto& plat : chunk.platforms) {
            if (plat.isHazard) continue;

            if (Physics_AABBIntersect(pBox, plat.box)) {
                float overlapY = Physics_GetOverlapY(pBox, plat.box);
                // Resolve vertical overlap
                player.y += overlapY;
                pBox.y = player.y;

                if (overlapY < 0.0f) {
                    // Resolved upwards: landed on a floor!
                    player.vy = 0.0f;
                    player.grounded = true;
                } else if (overlapY > 0.0f) {
                    // Resolved downwards: hit a ceiling!
                    player.vy = 0.0f;
                }
            }
        }
    }

    if (player.grounded && !wasGrounded) {
        AudioManager::PlaySound(SoundType::Landing);
        Logger::Log(LogLevel::Debug, "Physics", "Player landed on floor.");
    }
}

int World::CheckCollectibleCollisions(const AABB& playerBox) {
    int scoreIncrease = 0;
    for (auto& chunk : activeChunks) {
        for (auto& shard : chunk.shards) {
            if (shard.active && Physics_AABBIntersect(playerBox, shard.box)) {
                shard.active = false;
                scoreIncrease += 10; // 10 points per energy shard
            }
        }
    }
    return scoreIncrease;
}

bool World::CheckHazardCollision(const AABB& playerBox) {
    for (const auto& chunk : activeChunks) {
        // Platform hazards
        for (const auto& plat : chunk.platforms) {
            if (plat.isHazard && Physics_AABBIntersect(playerBox, plat.box)) {
                return true;
            }
        }
        // Drone hazards
        for (const auto& drone : chunk.drones) {
            if (Physics_AABBIntersect(playerBox, drone.box)) {
                return true;
            }
        }
    }
    return false;
}

void World::Rebase(float offset, Player& player, float& cameraX) {
    Logger::Log(LogLevel::Info, "World", "Rebasing coordinates. Offset: " + std::to_string(offset));

    player.x -= offset;
    player.prevX -= offset;
    cameraX -= offset;
    lastGeneratedX -= offset;

    for (auto& chunk : activeChunks) {
        chunk.startX -= offset;
        for (auto& plat : chunk.platforms) {
            plat.box.x -= offset;
        }
        for (auto& shard : chunk.shards) {
            shard.box.x -= offset;
        }
        for (auto& drone : chunk.drones) {
            drone.box.x -= offset;
            drone.startY = drone.startY; // no change, but keep compiler happy
        }
    }
}

void World::Render(SDL_Renderer* renderer, SDL_Texture* platformTex,
                   SDL_Texture* warningTex, SDL_Texture* shardTex,
                   SDL_Texture* enemyTex, float cameraX, float cameraY) {
    constexpr int VIEW_W = SCREEN_WIDTH;
    constexpr int VIEW_H = SCREEN_HEIGHT;

    int platformTextureW = 0;
    int platformTextureH = 0;
    if (platformTex) {
        SDL_QueryTexture(platformTex, nullptr, nullptr,
                         &platformTextureW, &platformTextureH);
    }

    for (const auto& chunk : activeChunks) {
        for (const auto& plat : chunk.platforms) {
            const float screenLeftF = plat.box.x - cameraX;
            const float screenTopF = plat.box.y - cameraY;
            const float screenRightF = screenLeftF + plat.box.w;
            const float screenBottomF = screenTopF + plat.box.h;

            if (screenRightF <= 0.0f || screenLeftF >= VIEW_W ||
                screenBottomF <= 0.0f || screenTopF >= VIEW_H) {
                continue;
            }

            const int left = std::max(0, static_cast<int>(screenLeftF));
            const int top = std::max(0, static_cast<int>(screenTopF));
            const int right = std::min(VIEW_W, static_cast<int>(screenRightF));
            const int bottom = std::min(VIEW_H, static_cast<int>(screenBottomF));
            const int visibleW = right - left;
            const int visibleH = bottom - top;

            if (visibleW <= 0 || visibleH <= 0) {
                continue;
            }

            SDL_Rect visibleRect = { left, top, visibleW, visibleH };

            if (plat.isHazard) {
                // Hazards are small; a single stretched copy is inexpensive.
                SDL_RenderCopy(renderer, warningTex, nullptr, &visibleRect);
                continue;
            }

            // The texture has already been cropped to its visible artwork.
            // Stretch one complete sprite across the whole physics platform.
            // Mapping the clipped destination back into texture space keeps
            // partially off-screen platforms correctly clipped without
            // truncating the art at the top or bottom.
            const int textureW = platformTextureW;
            const int textureH = platformTextureH;
            if (textureW <= 0 || textureH <= 0) {
                continue;
            }

            const float clippedLeft =
                static_cast<float>(left) - screenLeftF;
            const float clippedTop =
                static_cast<float>(top) - screenTopF;

            // Thin floating slabs use only the roof portion of the platform
            // artwork instead of squeezing the entire building into 30-50 px.
            const bool isThinSlab = plat.box.h <= 100.0f;
            const int sourceBaseHeight = isThinSlab
                ? std::max(1, textureH / 4)
                : textureH;

            int sourceX = static_cast<int>(
                (clippedLeft / plat.box.w) * textureW);
            int sourceY = static_cast<int>(
                (clippedTop / plat.box.h) * sourceBaseHeight);
            int sourceW = static_cast<int>(
                (static_cast<float>(visibleW) / plat.box.w) * textureW + 0.5f);
            int sourceH = static_cast<int>(
                (static_cast<float>(visibleH) / plat.box.h) *
                sourceBaseHeight + 0.5f);

            sourceX = std::max(0, std::min(sourceX, textureW - 1));
            sourceY = std::max(0, std::min(sourceY,
                                           sourceBaseHeight - 1));
            sourceW = std::max(1, std::min(sourceW,
                                           textureW - sourceX));
            sourceH = std::max(1, std::min(sourceH,
                                           sourceBaseHeight - sourceY));

            SDL_Rect sourceRect = {
                sourceX,
                sourceY,
                sourceW,
                sourceH
            };
            SDL_RenderCopy(renderer, platformTex, &sourceRect, &visibleRect);
        }

        for (const auto& shard : chunk.shards) {
            if (!shard.active) continue;

            const float sx = shard.box.x - cameraX;
            const float sy = shard.box.y - cameraY;
            if (sx + shard.box.w < -32.0f || sx > VIEW_W + 32.0f ||
                sy + shard.box.h < -32.0f || sy > VIEW_H + 32.0f) {
                continue;
            }

            // Additive soft glow under the shard (neon pickup)
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            const int cx = static_cast<int>(sx + shard.box.w * 0.5f);
            const int cy = static_cast<int>(sy + shard.box.h * 0.5f);
            SDL_SetRenderDrawColor(renderer, 250, 204, 21, 50);
            SDL_Rect glowOuter = { cx - 18, cy - 18, 36, 36 };
            SDL_RenderFillRect(renderer, &glowOuter);
            SDL_SetRenderDrawColor(renderer, 253, 224, 71, 90);
            SDL_Rect glowInner = { cx - 10, cy - 10, 20, 20 };
            SDL_RenderFillRect(renderer, &glowInner);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_Rect dstRect = {
                static_cast<int>(sx - 8.0f),
                static_cast<int>(sy - 8.0f),
                32,
                32
            };
            SDL_RenderCopy(renderer, shardTex, nullptr, &dstRect);
        }

        for (const auto& drone : chunk.drones) {
            const float sx = drone.box.x - cameraX;
            const float sy = drone.box.y - cameraY;
            if (sx + drone.box.w < -64.0f || sx > VIEW_W + 64.0f ||
                sy + drone.box.h < -64.0f || sy > VIEW_H + 64.0f) {
                continue;
            }

            // Additive thruster glow under the drone
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            const int cx = static_cast<int>(sx + drone.box.w * 0.5f);
            const int cy = static_cast<int>(sy + drone.box.h + 4.0f);
            SDL_SetRenderDrawColor(renderer, 34, 211, 238, 70);
            SDL_Rect thruster = { cx - 14, cy - 6, 28, 14 };
            SDL_RenderFillRect(renderer, &thruster);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_Rect dstRect = {
                static_cast<int>(sx - 16.0f),
                static_cast<int>(sy - 16.0f),
                64,
                64
            };
            SDL_RenderCopy(renderer, enemyTex, nullptr, &dstRect);
        }
    }
}
