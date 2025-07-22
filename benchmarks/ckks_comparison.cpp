#include <benchmark/benchmark.h>
#include <helib/helib.h>
#include "simple_binary_ckks.h"

#include <sstream>
#include <complex>
using namespace helib;

/* -------------------------------------------------- Helpers ---------- */
// Byte-size of HElib objects via serialization
static size_t bytes(const Ctxt& x)  { std::stringstream s; x.writeTo(s); return s.str().size(); }
static size_t bytes(const PubKey& x){ std::stringstream s; x.writeTo(s); return s.str().size(); }
static size_t bytes(const SecKey& x){ std::stringstream s; x.writeTo(s); return s.str().size(); }

// Quick rule-of-thumb for Binary CKKS: N coeffs × 8 bytes
static inline size_t binPolyBytes(long N) { return static_cast<size_t>(N) * sizeof(uint64_t); }

/* ========================================== STANDARD CKKS (HElib) === */
static void BM_KeyGen_Standard(benchmark::State& st) {
  const long m = st.range(0);
  Context ctx = ContextBuilder<CKKS>().m(m).bits(300).precision(20).build();
  for (auto _ : st) {
    SecKey sk(ctx);
    sk.GenSecKey();
    benchmark::DoNotOptimize(sk);
  }
  SecKey sk(ctx); sk.GenSecKey();
  PubKey pk = sk;
  st.counters["pk_bytes"] = bytes(pk);
  st.counters["sk_bytes"] = bytes(sk);
}
BENCHMARK(BM_KeyGen_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Enc_Standard(benchmark::State& st) {
  const long m = st.range(0);
  Context ctx = ContextBuilder<CKKS>().m(m).bits(300).precision(20).build();
  SecKey sk(ctx); sk.GenSecKey();
  const PubKey& pk = sk;
  EncryptedArrayCx ea(ctx);
  PtxtArray ptxt(ctx, std::vector<std::complex<double>>(ea.size(), 1.0));
  for (auto _ : st) {
    Ctxt ct(pk); ptxt.encrypt(ct);
    benchmark::DoNotOptimize(ct);
  }
  Ctxt ct(pk); ptxt.encrypt(ct);
  st.counters["cipher_bytes"] = bytes(ct);
}
BENCHMARK(BM_Enc_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Dec_Standard(benchmark::State& st) {
  const long m = st.range(0);
  Context ctx = ContextBuilder<CKKS>().m(m).bits(300).precision(20).build();
  SecKey sk(ctx); sk.GenSecKey();
  const PubKey& pk = sk;
  EncryptedArrayCx ea(ctx);
  PtxtArray ptxt(ctx, std::vector<std::complex<double>>(ea.size(), 1.0));
  Ctxt ct(pk); ptxt.encrypt(ct);
  for (auto _ : st) {
    PtxtArray out(ctx); out.decrypt(ct, sk);
    benchmark::DoNotOptimize(out);
  }
}
BENCHMARK(BM_Dec_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Add_Standard(benchmark::State& st) {
  const long m = st.range(0);
  Context ctx = ContextBuilder<CKKS>().m(m).bits(300).precision(20).build();
  SecKey sk(ctx); sk.GenSecKey();
  const PubKey& pk = sk;
  EncryptedArrayCx ea(ctx);
  PtxtArray ptxt(ctx, std::vector<std::complex<double>>(ea.size(), 1.0));
  Ctxt a(pk), b(pk); ptxt.encrypt(a); ptxt.encrypt(b);
  for (auto _ : st) {
    Ctxt c = a; c += b;
    benchmark::DoNotOptimize(c);
  }
  Ctxt c = a; c += b;
  st.counters["cipher_bytes"] = bytes(c);
}
BENCHMARK(BM_Add_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Mul_Standard(benchmark::State& st) {
  const long m = st.range(0);
  Context ctx = ContextBuilder<CKKS>().m(m).bits(300).precision(20).build();
  SecKey sk(ctx); sk.GenSecKey(); addSome1DMatrices(sk);
  const PubKey& pk = sk;
  EncryptedArrayCx ea(ctx);
  PtxtArray pt(ctx, std::vector<std::complex<double>>(ea.size(), 1.0));
  Ctxt a(pk), b(pk); pt.encrypt(a); pt.encrypt(b);
  for (auto _ : st) {
    Ctxt c = a; c.multiplyBy(b);
    benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(BM_Mul_Standard)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

/* ========================================== BINARY CKKS (custom) === */
static void BM_KeyGen_Binary(benchmark::State& st) {
  const long N = st.range(0);
  for (auto _ : st) {
    BinaryCKKS::SimpleBinaryCKKS sch(128, N);
    benchmark::DoNotOptimize(sch.keyGen());
  }
  st.counters["sk_bytes"] = binPolyBytes(N);
}
BENCHMARK(BM_KeyGen_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Enc_Binary(benchmark::State& st) {
  const long N = st.range(0);
  BinaryCKKS::SimpleBinaryCKKS sch(128, N);
  auto keys = sch.keyGen();
  auto p = sch.encode(std::vector<long>(N, 1));
  for (auto _ : st) {
    auto ct = sch.encrypt(p, keys);
    benchmark::DoNotOptimize(ct);
  }
  st.counters["cipher_bytes"] = binPolyBytes(N);
}
BENCHMARK(BM_Enc_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Dec_Binary(benchmark::State& st) {
  const long N = st.range(0);
  BinaryCKKS::SimpleBinaryCKKS sch(128, N);
  auto keys = sch.keyGen();
  auto p = sch.encode(std::vector<long>(N, 1));
  auto ct = sch.encrypt(p, keys);
  for (auto _ : st) {
    auto out = sch.decrypt(ct, keys);
    benchmark::DoNotOptimize(out);
  }
}
BENCHMARK(BM_Dec_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Add_Binary(benchmark::State& st) {
  const long N = st.range(0);
  BinaryCKKS::SimpleBinaryCKKS sch(128, N);
  auto keys = sch.keyGen();
  auto p = sch.encode(std::vector<long>(N, 1));
  auto a = sch.encrypt(p, keys);
  auto b = sch.encrypt(p, keys);
  for (auto _ : st) {
    auto c = sch.add(a, b);
    benchmark::DoNotOptimize(c);
  }
  st.counters["cipher_bytes"] = binPolyBytes(N);
}
BENCHMARK(BM_Add_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

static void BM_Mul_Binary(benchmark::State& st) {
  const long N = st.range(0);
  BinaryCKKS::SimpleBinaryCKKS sch(128, N);
  auto keys = sch.keyGen();
  auto p = sch.encode(std::vector<long>(N, 1));
  auto a = sch.encrypt(p, keys);
  auto b = sch.encrypt(p, keys);
  for (auto _ : st) {
    auto c = sch.multiply(a, b, keys);
    benchmark::DoNotOptimize(c);
  }
  st.counters["cipher_bytes"] = binPolyBytes(N);
}
BENCHMARK(BM_Mul_Binary)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(8192);

BENCHMARK_MAIN();
