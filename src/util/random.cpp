#include "random.h"

Random::Random(uint64_t init_state, uint64_t init_seq) {
    Seed(init_state, init_seq);
}

void Random::Seed(uint64_t init_state, uint64_t init_seq) {
    state = 0U;
    inc = (init_seq << 1u) | 1u;
    Next();
    state += init_state;
    Next();
}

uint32_t Random::Next() {
    uint64_t oldstate = state;
    state = oldstate * 6364136223846793005ULL + inc;
    uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = static_cast<uint32_t>(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint32_t Random::Range(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    return min + (Next() % (max - min));
}

float Random::Float() {
    return static_cast<float>(Next()) / static_cast<float>(0xFFFFFFFF);
}
