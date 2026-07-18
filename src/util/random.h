#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

class Random {
public:
    Random(uint64_t init_state = 0x853c49e6748fea9bULL, uint64_t init_seq = 0xda3e39cb94b95bdbULL);
    void Seed(uint64_t init_state, uint64_t init_seq = 0xda3e39cb94b95bdbULL);
    uint32_t Next();
    uint32_t Range(uint32_t min, uint32_t max); // [min, max)
    float Float(); // [0.0f, 1.0f)
private:
    uint64_t state;
    uint64_t inc;
};

#endif // RANDOM_H
