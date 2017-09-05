#include "gtest/gtest.h"
#include <aliSystem.hpp>
#include <vector>

namespace {


  const double ms = 0.001; // 1 ms
  using Time    = aliSystem::Time;
  using TVec    = std::vector<Time::TP>;
  using TVecPtr = std::shared_ptr<TVec>;
  using Stats   = aliSystem::Stats;
  using Pool    = aliSystem::Threading::Pool;
  using Queue   = aliSystem::Threading::Queue;
  using Work    = aliSystem::Threading::Work;
  using QVec    = std::vector<Queue::Ptr>;
  QVec queues;

  void msleep(double ms) {
    usleep((useconds_t)(ms*1000));
  }
}

TEST(aliSystemThreadingPool, general) {
  QVec              queues;
  TVecPtr           triggered(new TVec);
  TVecPtr           expected(new TVec);
  size_t            numQueues  = 4;
  const std::string name       = "testPool";
  Pool::Ptr         pool       = Pool::Create(name, 0);
  Stats::Ptr        queueStats = Stats::Create("threading pool test queues");
  Stats::Ptr        workStats  = Stats::Create("threading pool test work");
  Work::Ptr         work       = Work::Create(workStats,
					      [=](bool &requeue) {
						static std::mutex lock;
						requeue = false;
						msleep(100);
						std::lock_guard<std::mutex> g(lock);
						triggered->push_back(Time::Now());
					      });
  for (size_t i=0;i<numQueues;++i) {
    Queue::Ptr queue = pool->AddQueue("testPoolQueue", 1, queueStats);
    queues.push_back(queue);
  }
  ASSERT_STREQ(name.c_str(),pool->Name().c_str());
  for (size_t numThreads=0;
       numThreads<4; // increment to 3
       ++numThreads) {
    pool->SetNumThreads(numThreads);
    msleep(10);
    ASSERT_EQ(pool->GetNumThreads(), numThreads) << " after set " << pool->GetNumThreads() << ", " << numThreads;
  }
  pool->SetNumThreads(0);
  ASSERT_EQ(pool->GetNumThreads(), (size_t)3) << " num threads should not decrement";
  //
  // Verify basic execution
  Time::TP now = Time::Now();
  for (size_t i=0;i<numQueues;++i) {
    queues[i]->AddWork(work);
  }
  expected->push_back(now + std::chrono::milliseconds(100));
  expected->push_back(now + std::chrono::milliseconds(100));
  expected->push_back(now + std::chrono::milliseconds(100));
  expected->push_back(now + std::chrono::milliseconds(200)); // 4 queues, 3 threads -> 2 rounds
  pool->Flush();
  ASSERT_EQ(expected->size(), triggered->size());
  for (size_t i=0;i<4;++i) {
    Time::TP t1 = expected ->operator[](i);
    Time::TP t2 = triggered->operator[](i);
    ASSERT_TRUE(t1-t2<std::chrono::milliseconds(3));
    ASSERT_TRUE(t2-t1<std::chrono::milliseconds(3));
  }
}

TEST(aliSystemThreadingPool, flushAndStop) {
  using IPtr = std::shared_ptr<size_t>;
  QVec              queues;
  size_t            numQueues  = 6;
  size_t            numThreads = 4;
  size_t            numItems   = 100; // 100 * 10ms/4threads -> 0.25 seconds
  IPtr              runCount(new size_t(0));
  double            workDelay  = 10;
  const std::string name       = "testPool";
  Stats::Ptr        workStats  = Stats::Create("pool testing work stats");
  Pool::Ptr         pool       = Pool::Create(name, numThreads);
  Work::Ptr         work       = Work::Create(workStats,
					      [=](bool &requeue) {
						static std::mutex lock;
						requeue = false;
						msleep(workDelay);
						std::lock_guard<std::mutex> g(lock);
						++(*runCount);
					      });
  msleep(25);
  ASSERT_EQ(pool->GetNumThreads(), numThreads);
  for (size_t i=0;i<numQueues;++i) {
    // concurrency is set to make sure all items could drain
    // in workDelay*numItems/numThreads to make the test simpler.
    // At present Pool is a round-robin processor, but if changed
    // to some other scheduling algorithm in the future, this might
    // need re-examination.
    Stats::Ptr queueStats  = aliSystem::Stats::Create("threading pool test queues");
    size_t     concurrency = i+numThreads;
    Queue::Ptr queue       = pool->AddQueue("testPoolQueue", concurrency, queueStats);
    queues.push_back(queue);
  }
  for (size_t i=0;i<numItems;++i) {
    queues[i%numQueues]->AddWork(work);
  }
  msleep(numItems*workDelay/numThreads/2);  // ~1/2 the work should be done
  pool->Stop(false);              // no wait
  size_t rCnt = *runCount;
  ASSERT_NEAR(rCnt, numItems/2, 5);
  msleep(workDelay*2); // let anything that was running flush out
  ASSERT_TRUE(*runCount<=rCnt+numThreads);
  ASSERT_EQ(pool->GetNumThreads(), (size_t)0) << "all threads should have exited at this point";

  // restart & stop w/wait
  pool->SetNumThreads(numThreads);
  pool->Flush();
  msleep(workDelay*2);
  ASSERT_EQ(*runCount, numItems);
  for (size_t i=0;i<numItems;++i) {
    queues[i%numQueues]->AddWork(work);
  }
  pool->Stop(true);
  ASSERT_EQ(pool->GetNumThreads(),(size_t)0) << "all threads should have terminated before stop returns";
  ASSERT_TRUE(*runCount<2*numItems) << "Queues should not be drained at this point";
}

TEST(aliSystemThreadingPool, statsCheck) {
  QVec       queues;
  double     overheadLimit    = 0.01; // ratio of overhead, 1%
  double     delayInSeconds   = 0.050;
  size_t     numQueues        = 5;
  size_t     numThreads       = 4;
  size_t     numItems         = 400;
  double     expectedRunTime  = delayInSeconds*numItems/numThreads;
  double     totalRunTime     = delayInSeconds*numItems;
  Pool::Ptr  pool             = Pool::Create("gross stats check", numThreads);
  Stats::Ptr queueStats       = Stats::Create("queue stats");
  Stats::Ptr workStats        = Stats::Create("work stats");
  Work::Ptr  work             = Work::Create(workStats,
					     [=](bool &) {
					       msleep(1000*delayInSeconds);
					     });
  ASSERT_NEAR(expectedRunTime, 5.0, 0.5);
  for (size_t i=0;i<numQueues;++i) {
    Queue::Ptr queue = pool->AddQueue("testPoolQueue", 1, queueStats);
    queues.push_back(queue);
  }
  for (size_t i=0;i<numItems;++i) {
    queues[i%numQueues]->AddWork(work);
  }
  pool->Flush();
  Time::Dur workTime  = workStats->RunTime();
  Time::Dur queueTime = queueStats->RunTime();
  Time::Dur poolTime  = pool->GetStats()->RunTime();
  DEBUG("Expected run time " << expectedRunTime);
  DEBUG("total run time    " << totalRunTime);
  DEBUG("work time         " << workTime);
  DEBUG("queue time        " << queueTime);
  DEBUG("pool time         " << poolTime);
  ASSERT_NEAR(1-Time::ToSeconds(workTime)  / totalRunTime, 0, overheadLimit);
  ASSERT_NEAR(1-Time::ToSeconds(queueTime) / totalRunTime, 0, overheadLimit);
  ASSERT_NEAR(1-Time::ToSeconds(poolTime)  / totalRunTime, 0, overheadLimit);
}
