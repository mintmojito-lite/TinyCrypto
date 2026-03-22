#include <benchmark/benchmark.h>
#include "aes256.hpp"
#include <vector>

using namespace tinycrypto;

static void BM_AES256_ECB(benchmark::State& state) {
    std::array<uint8_t, 32> key = {0};
    Aes256Ctx ctx;
    aes256_init(ctx, key);
    
    std::vector<uint8_t> in(state.range(0), 0);
    std::vector<uint8_t> out(state.range(0), 0);
    
    for (auto _ : state) {
        for (size_t i = 0; i < in.size(); i += 16) {
            aes256_encrypt_block(ctx, in.data() + i, out.data() + i);
        }
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

BENCHMARK(BM_AES256_ECB)->RangeMultiplier(1024)->Range(32, 1024 * 1024);

static void BM_AES256_CTR(benchmark::State& state) {
    std::array<uint8_t, 32> key = {0};
    Aes256Ctx ctx;
    aes256_init(ctx, key);
    
    std::array<uint8_t, 16> nonce_counter = {0};
    
    std::vector<uint8_t> in(state.range(0), 0);
    std::vector<uint8_t> out(state.range(0), 0);
    
    for (auto _ : state) {
        aes256_ctr_encrypt(ctx, nonce_counter, in.data(), out.data(), in.size());
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

BENCHMARK(BM_AES256_CTR)->RangeMultiplier(1024)->Range(32, 1024 * 1024);
