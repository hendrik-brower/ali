#include "gtest/gtest.h"
#include <aliSystem.hpp>
#include <vector>

namespace {

  const double ms = 0.001; // 1 ms
  using Stats    = aliSystem::Stats;
  using Pool     = aliSystem::Threading::Pool;
  using Queue    = aliSystem::Threading::Queue;
  using Work     = aliSystem::Threading::Work;
  using Sem      = aliSystem::Threading::Semaphore;
  using Listener = aliSystem::Listener<const Queue::WPtr&>;
  using IPtr     = std::unique_ptr<size_t>;
  using Time     = aliSystem::Time;
  struct MyWork {
    using Ptr = std::shared_ptr<MyWork>;
    MyWork(const std::string &name)
      : stats(Stats::Create(name)) {
    }
    void Run(bool &requeue) {
      requeue = false;
      usleep(100000);
      std::lock_guard<std::mutex> g(lock);
      ++cnt;
    }
    size_t Cnt() const{ return cnt; }
    const Stats::Ptr &GetStats() const { return stats; }
  private:
    std::mutex lock;
    size_t     cnt = 0;
    Stats::Ptr stats;
  };

  struct DetailedTest : testing::Test {
    Sem::Ptr    sem;
    std::string name;
    IPtr        runCount;
    Stats::Ptr  queueStats;
    Stats::Ptr  workStats;
    Queue::Ptr  queue;
    Work::Ptr   work;
    size_t      usDelay = 5000; // default delay to 5ms
    const size_t initialMaxConcurrency = 3;
    void AddWork(size_t num) {
      THROW_IF(!queue, "Invalid queue");
      for (size_t i=0;i<num;++i) {
	queue->AddWork(work);
      }
    }
    void SetUp() {
      sem.reset(new Sem);
      runCount.reset(new size_t(0));
      name       = "detailed testing";
      queueStats = Stats::Create(name+" queue stats");
      workStats  = Stats::Create(name+" work stats");
      queue      = Queue::Create(name+" queue", sem, initialMaxConcurrency, queueStats);
      work       = Work::Create(workStats,
				[=](bool &) {
				  std::mutex lock;
				  usleep(usDelay);
				  std::lock_guard<std::mutex> g(lock);
				  ++(*runCount);
				});
    }
    void TearDown() {
      sem.reset();
      queueStats.reset();
      workStats.reset();
      queue.reset();
      work.reset();
    }
    static void SetUpTestCase() {
    }
    static void TearDownTestCase() {
    }
  };

}

TEST(aliSystemThreadingQueue, general) {
  // testing with pool
  MyWork::Ptr p1(new MyWork("work1"));
  MyWork::Ptr p2(new MyWork("work2"));
  MyWork::Ptr p3(new MyWork("work3"));
  Pool::Ptr   pool = Pool::Create("queueTestPool", 5);
  Queue::Ptr  q1   = pool->AddQueue("q1", 3, Stats::Create("q1Stats"));
  Queue::Ptr  q2   = pool->AddQueue("q2", 1, Stats::Create("q2Stats"));
  Queue::Ptr  q3   = pool->AddQueue("q3", 1, Stats::Create("q3Stats"));
  Work::Ptr   w1   = Work::Create(p1->GetStats(), [=](bool &requeue) {p1->Run(requeue);});
  Work::Ptr   w2   = Work::Create(p2->GetStats(), [=](bool &requeue) {p2->Run(requeue);});
  Work::Ptr   w3   = Work::Create(p3->GetStats(), [=](bool &requeue) {p3->Run(requeue);});
  q1->Stop();
  q2->Stop();
  q3->Stop();
  for (size_t i=0;i<1000;++i) {
    q1->AddWork(w1);
    q2->AddWork(w2);
    q3->AddWork(w3);
  }
  ASSERT_EQ(q1->NumPending(),1000u);
  ASSERT_EQ(q2->NumPending(),1000u);
  ASSERT_EQ(q3->NumPending(),1000u);
  ASSERT_EQ(w1->GetStats()->Count(),0u);
  ASSERT_EQ(w2->GetStats()->Count(),0u);
  ASSERT_EQ(w3->GetStats()->Count(),0u);
  ASSERT_EQ(q1->QueueStats()->Count(),0u);
  ASSERT_EQ(q2->QueueStats()->Count(),0u);
  ASSERT_EQ(q3->QueueStats()->Count(),0u);
  q1->Start();
  q2->Start();
  q3->Start();
  sleep(5);
  q1->Stop();
  q2->Stop();
  q3->Stop();
  ASSERT_NEAR(w1->GetStats()->Count(),5*3*10,5);
  ASSERT_NEAR(w2->GetStats()->Count(),5*1*10,5);
  ASSERT_NEAR(w3->GetStats()->Count(),5*1*10,5);
  ASSERT_EQ(q1->QueueStats()->Count(),w1->GetStats()->Count());
  ASSERT_EQ(q2->QueueStats()->Count(),w2->GetStats()->Count());
  ASSERT_EQ(q3->QueueStats()->Count(),w3->GetStats()->Count());
}

// testing without pool
TEST_F(DetailedTest, Start_Stop_NumPending_IsBusy) {
  ASSERT_FALSE(queue->IsBusy());
  ASSERT_EQ(queue->NumPending(), 0u);
  AddWork(2);
  ASSERT_TRUE(queue->IsBusy());
  ASSERT_EQ(queue->NumPending(), 2u);
  Work::Ptr next = queue->Next();
  ASSERT_EQ(queue->NumPending(), 1u);
  ASSERT_TRUE(next) << "Failed to retrieve a work unit";
  ASSERT_FALSE(queue->IsStopped());
  queue->Stop();
  ASSERT_TRUE(queue->IsBusy());
  //
  // Verify work runs once its been fetched, even if the queue is stopped.
  ASSERT_EQ(*runCount, 0u);
  queue->Run(next);
  ASSERT_EQ(*runCount, 1u);
  ASSERT_TRUE(queue->IsBusy());
  ASSERT_TRUE(queue->IsStopped());
  ASSERT_EQ(queue->NumPending(), 1u);
  next = queue->Next();
  ASSERT_FALSE(next) << "retrieved work unit after queue stopped";
  ASSERT_EQ(queue->NumPending(), 1u) << "Work should not have been pulled from the queue";
  queue->Start(); 
  next = queue->Next();
  ASSERT_EQ(queue->NumPending(), 0u);
  ASSERT_TRUE(queue->IsBusy());
  ASSERT_TRUE(next) << "work available after re-starting the queue";
  queue->Run(work);
  ASSERT_FALSE(queue->IsBusy());
}

TEST_F(DetailedTest, OnIdle) {
  Listener::Ptr lPtr = Listener::Create(name+" listener", [=](const Queue::WPtr &) {
      ++(*runCount);
    });
  queue->OnIdle()->Register(lPtr,true);
  AddWork(2);
  Work::Ptr next = queue->Next();
  ASSERT_EQ(*runCount,0u);
  queue->Run(next);
  ASSERT_EQ(*runCount,1u);
}

TEST_F(DetailedTest, concurrency) {
  Work::Ptr work = Work::Create(workStats,
				[=](bool &) {
				  *runCount = queue->CurrentConcurrency();
				});
  queue->AddWork(work);
  ASSERT_EQ(queue->GetMaxConcurrency(), initialMaxConcurrency);
  ASSERT_EQ(queue->CurrentConcurrency(), 0u);
  queue->Run(queue->Next());
  ASSERT_EQ(*runCount,1u);

  queue->SetMaxConcurrency(9);
  ASSERT_EQ(queue->GetMaxConcurrency(), 9u);
}

TEST_F(DetailedTest, clear) {
  AddWork(25);
  ASSERT_EQ(queue->NumPending(), 25u);
  queue->Next();
  ASSERT_EQ(queue->NumPending(), 24u);
  queue->Clear();
  ASSERT_EQ(queue->NumPending(),  0u);
}
TEST_F(DetailedTest, stopAfter) {
  const size_t groupCnt = 5;
  AddWork(groupCnt);
  queue->StopAfter(work);
  AddWork(groupCnt);
  ASSERT_EQ(queue->StoppedStats()->Count(),0u) << "never stopped";
  for (size_t i=0;i<2*groupCnt;++i) {
    Work::Ptr next = queue->Next();
    queue->Run(next);
  }
  ASSERT_EQ(*runCount,groupCnt+1);
  ASSERT_EQ(queue->StoppedStats()->Count(),1u) << "stopped once";
}

TEST_F(DetailedTest, runTime) {
  const size_t cnt = 100;
  AddWork(cnt);
  for (size_t i=0;i<cnt;++i) {
    Work::Ptr next = queue->Next();
    queue->Run(next);
  }
  ASSERT_NEAR(Time::ToSeconds(queue->QueueStats()->RunTime()),
	      double(cnt*usDelay)/1000/1000, 0.050);
  ASSERT_NEAR(Time::ToSeconds(workStats->RunTime()),
	      double(cnt*usDelay)/1000/1000, 0.050);
}

