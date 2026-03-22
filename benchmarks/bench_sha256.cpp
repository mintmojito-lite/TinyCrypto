#include <benchmark/benchmark.h>
#include "sha256.hpp"
#include <vector>

using namespace tinycrypto;

static void BM_SHA256(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<uint8_t> data(size, 0xAA); // dummy data payload
    
    for (auto _ : state) {
        auto hash = SHA256::hash(data.data(), data.size());
        benchmark::DoNotOptimize(hash);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

// Real-world performance vectors:
// 1 KB payload
BENCHMARK(BM_SHA256)->Arg(1024);
// 1 MB payload
BENCHMARK(BM_SHA256)->Arg(1024 * 1024);
// 100 MB payload
BENCHMARK(BM_SHA256)->Arg(100 * 1024 * 1024);
