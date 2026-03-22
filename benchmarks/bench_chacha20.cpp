#include <benchmark/benchmark.h>
#include "chacha20.hpp"
#include <vector>

using namespace tinycrypto;

static void BM_ChaCha20_Scalar(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<uint8_t> in(size, 0x11);
    std::vector<uint8_t> out(size, 0);
    
    uint8_t key[32] = {0};
    uint8_t nonce[12] = {0};

    // To prevent compiler from optimizing out AVX2 or Scalar, we explicitly run the instance
    for (auto _ : state) {
        ChaCha20 ctx(key, nonce, 0);
        ctx.encrypt_scalar(in.data(), out.data(), in.size());
        benchmark::DoNotOptimize(out);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

static void BM_ChaCha20_AVX2(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<uint8_t> in(size, 0x11);
    std::vector<uint8_t> out(size, 0);
    
    uint8_t key[32] = {0};
    uint8_t nonce[12] = {0};

    for (auto _ : state) {
        ChaCha20 ctx(key, nonce, 0);
        ctx.encrypt_avx2(in.data(), out.data(), in.size()); // Exposes AVX2 explicitly
        benchmark::DoNotOptimize(out);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

BENCHMARK(BM_ChaCha20_Scalar)->Arg(1024 * 1024)->Arg(10 * 1024 * 1024)->Arg(100 * 1024 * 1024);
BENCHMARK(BM_ChaCha20_AVX2)->Arg(1024 * 1024)->Arg(10 * 1024 * 1024)->Arg(100 * 1024 * 1024)->Arg(1024 * 1024 * 1024);
