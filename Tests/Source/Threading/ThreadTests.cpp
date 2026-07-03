#include <gtest/gtest.h>

#include "Engine/Threading/Thread.h"
#include "Engine/Threading/CPUInfo.h"

#include <atomic>

using namespace engine::threading;

TEST(ThreadTest, CreateAndJoin)
{
    std::atomic<bool> flag{false};

    {
        Thread t("TestThread", [&flag]() { flag.store(true); });
        // Thread destructor auto-joins, but we can also join explicitly.
        t.Join();
    }

    EXPECT_TRUE(flag.load());
}

TEST(ThreadTest, SharedMutexExclusive)
{
    SharedMutex mutex;
    std::atomic<int> counter{0};

    constexpr int kIterations = 10;

    auto increment = [&]() {
        for (int i = 0; i < kIterations; ++i) {
            mutex.Lock();
            ++counter;
            mutex.Unlock();
        }
    };

    Thread a("MutexA", increment);
    Thread b("MutexB", increment);

    a.Join();
    b.Join();

    // Both threads increment the same counter under an exclusive lock,
    // so the final value must be exactly 2 * kIterations.
    EXPECT_EQ(counter.load(), 2 * kIterations);
}

TEST(ThreadTest, CPUInfoLogicalCores)
{
    u32 cores = CPUInfo::LogicalCoreCount();
    EXPECT_GT(cores, 0u);
}