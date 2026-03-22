#include <benchmark/benchmark.h>
#include "curve25519.hpp"

using namespace tinycrypto;

static void BM_Curve25519_Keygen(benchmark::State& state) {
    std::array<uint8_t, 32> scalar = {1, 2, 3, 4}; // Dummy scalar constraints
    std::array<uint8_t, 32> base_point = {9, 0}; // Base point x=9 uniformly defined via FIPS
    for (auto _ : state) {
        benchmark::DoNotOptimize(scalar.data());
        auto pub = curve25519(scalar, base_point);
        benchmark::DoNotOptimize(pub);
        benchmark::ClobberMemory();
    }
}

static void BM_Curve25519_DH(benchmark::State& state) {
    std::array<uint8_t, 32> alice_priv = {1, 2, 3, 4};
    std::array<uint8_t, 32> bob_pub = {5, 6, 7, 8};
    for (auto _ : state) {
        benchmark::DoNotOptimize(alice_priv.data());
        benchmark::DoNotOptimize(bob_pub.data());
        auto shared = curve25519(alice_priv, bob_pub);
        benchmark::DoNotOptimize(shared);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_Curve25519_Keygen);
BENCHMARK(BM_Curve25519_DH);
