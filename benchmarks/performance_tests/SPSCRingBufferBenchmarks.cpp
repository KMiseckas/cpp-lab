#include <atomic>
#include <cstdint>

#include <benchmark/benchmark.h>

#include "ring_buffers/SPSCRingBuffer.h"
#include "ring_buffers/SPSCRingBufferV2"

namespace
{

template <typename RingBuffer>
void BM_SPSCRingBufferRoundTrip(benchmark::State &state)
{
    static RingBuffer buffer;
    std::uint64_t value = 0;

    if (state.thread_index() == 0)
    {
        for (auto _ : state)
        {
            while (!buffer.push(value))
            {
            }
            ++value;
        }
    }
    else
    {
        for (auto _ : state)
        {
            while (!buffer.pop(value))
            {
            }
            benchmark::DoNotOptimize(value);
        }
    }
}

// Test basic (SPSCRingBuffer) round trip speed.
BENCHMARK_TEMPLATE(BM_SPSCRingBufferRoundTrip, SPSCRingBuffer<std::uint64_t, 2048>)->Threads(2);

// Test false sharing removed (SPSCRingBufferV2) round trip speed (faster)
BENCHMARK_TEMPLATE(BM_SPSCRingBufferRoundTrip, SPSCRingBufferV2<std::uint64_t, 2048>)->Threads(2);
} // namespace
