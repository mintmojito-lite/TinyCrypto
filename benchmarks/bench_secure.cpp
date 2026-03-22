#include <benchmark/benchmark.h>
#include "utils.hpp"
#include <vector>
#include <cstring>

using namespace tinycrypto::utils;

static void BM_Memcmp(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<uint8_t> a(size, 0xAA);
    std::vector<uint8_t> b(size, 0xAA);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(a.data());
        benchmark::DoNotOptimize(b.data());
        bool res = (std::memcmp(a.data(), b.data(), size) == 0);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

static void BM_ConstantTimeEqual(benchmark::State& state) {
    size_t size = state.range(0);
    std::vector<uint8_t> a(size, 0xAA);
    std::vector<uint8_t> b(size, 0xAA);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(a.data());
        benchmark::DoNotOptimize(b.data());
        bool res = constant_time_equal(a.data(), b.data(), size);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

static void BM_VectorAlloc(benchmark::State& state) {
    size_t size = state.range(0);
    for (auto _ : state) {
        std::vector<uint8_t> vec(size, 0);
        benchmark::DoNotOptimize(vec.data());
    }
}

static void BM_SecureBufferAlloc(benchmark::State& state) {
    size_t size = state.range(0);
    for (auto _ : state) {
        SecureBuffer<uint8_t> buf(size);
        benchmark::DoNotOptimize(buf.data());
    }
}

BENCHMARK(BM_Memcmp)->Arg(32)->Arg(1024)->Arg(1024*1024);
BENCHMARK(BM_ConstantTimeEqual)->Arg(32)->Arg(1024)->Arg(1024*1024);
BENCHMARK(BM_VectorAlloc)->Arg(1024)->Arg(1024*1024);
BENCHMARK(BM_SecureBufferAlloc)->Arg(1024)->Arg(1024*1024);
