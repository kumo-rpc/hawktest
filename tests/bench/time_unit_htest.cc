#include <hawktest/bench/benchmark_api.h>
#include <hawktest/bench/state.h>
#include <hawktest/test/htest.h>

namespace benchmark {
namespace internal {

namespace {

class DummyBenchmark : public benchmark::Benchmark {
 public:
  DummyBenchmark() : Benchmark("dummy") {}
  void Run(State& /*state*/) override {}
};

TEST(DefaultTimeUnitTest, TimeUnitIsNotSet) {
  DummyBenchmark benchmark;
  EXPECT_EQ(benchmark.GetTimeUnit(), kNanosecond);
}

TEST(DefaultTimeUnitTest, DefaultIsSet) {
  DummyBenchmark benchmark;
  EXPECT_EQ(benchmark.GetTimeUnit(), kNanosecond);
  SetDefaultTimeUnit(kMillisecond);
  EXPECT_EQ(benchmark.GetTimeUnit(), kMillisecond);
}

TEST(DefaultTimeUnitTest, DefaultAndExplicitUnitIsSet) {
  DummyBenchmark benchmark;
  benchmark.Unit(kMillisecond);
  SetDefaultTimeUnit(kMicrosecond);

  EXPECT_EQ(benchmark.GetTimeUnit(), kMillisecond);
}

}  // namespace
}  // namespace internal
}  // namespace benchmark
