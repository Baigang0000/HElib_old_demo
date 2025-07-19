#include <benchmark/benchmark.h>
#include <helib/helib.h>
#include "simple_binary_ckks.h"

using namespace helib;

// === Standard CKKS KeyGen ===
static void BM_KeyGen_Standard(benchmark::State& state) {
  long m = state.range(0);
  for (auto _ : state) {
    Context context = ContextBuilder<CKKS>()
                        .m(m).bits(300).precision(20)
                        .build();
    SecKey secretKey(context);
    secretKey.GenSecKey();
  }
}
BENCHMARK(BM_KeyGen_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

// === Binary CKKS KeyGen ===
static void BM_KeyGen_Binary(benchmark::State& state) {
  long N = state.range(0);
  for (auto _ : state) {
    BinaryCKKS::SimpleBinaryCKKS scheme(128, N);
    BinaryCKKS::SimpleBinaryCKKSKeys keys = scheme.keyGen();
    benchmark::DoNotOptimize(keys);
  }
}
BENCHMARK(BM_KeyGen_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

// === Binary CKKS Multiplication ===
static void BM_Mul_Binary(benchmark::State& state) {
  long N = state.range(0);
  BinaryCKKS::SimpleBinaryCKKS scheme(128, N);
  auto keys = scheme.keyGen();

  std::vector<long> input(N, 1);
  auto p = scheme.encode(input);
  auto ct1 = scheme.encrypt(p, keys);
  auto ct2 = scheme.encrypt(p, keys);

  for (auto _ : state) {
    auto ct3 = scheme.multiply(ct1, ct2, keys);
    benchmark::DoNotOptimize(ct3);
  }
}
BENCHMARK(BM_Mul_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

// === Standard CKKS Multiplication ===
static void BM_Mul_Standard(benchmark::State& state) {
  long m = state.range(0);
  Context context = ContextBuilder<CKKS>()
                      .m(m).bits(300).precision(20)
                      .build();
  SecKey secretKey(context);
  secretKey.GenSecKey();
  addSome1DMatrices(secretKey);
  const PubKey& publicKey = secretKey;

  EncryptedArrayCx ea(context);
  long slots = ea.size();
  PtxtArray ptxt1(context, std::vector<std::complex<double>>(slots, 1.0));
  PtxtArray ptxt2(context, std::vector<std::complex<double>>(slots, 1.0));

  Ctxt ctxt1(publicKey), ctxt2(publicKey);
  ptxt1.encrypt(ctxt1);
  ptxt2.encrypt(ctxt2);

  for (auto _ : state) {
    Ctxt ctxt_res = ctxt1;
    ctxt_res.multiplyBy(ctxt2);
    benchmark::DoNotOptimize(ctxt_res);
  }
}
BENCHMARK(BM_Mul_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

BENCHMARK_MAIN();
