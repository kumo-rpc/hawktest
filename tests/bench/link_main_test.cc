#include <hawktest/bench/registration.h>
#include <hawktest/bench/state.h>
#include <hawktest/bench/utils.h>

namespace {
void BM_empty(benchmark::State& state) {
  for (auto _ : state) {
    auto iterations = static_cast<double>(state.iterations()) *
                      static_cast<double>(state.iterations());
    benchmark::DoNotOptimize(iterations);
  }
}
BENCHMARK(BM_empty);
}  // end namespace
