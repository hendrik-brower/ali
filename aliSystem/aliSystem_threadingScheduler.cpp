#include <aliSystem_threadingScheduler.hpp>
#include <aliSystem_componentRegistry.hpp>
#include <aliSystem_component.hpp>
#include <aliSystem_logging.hpp>
#include <aliSystem_threadingPool.hpp>
#include <aliSystem_threadingSemaphore.hpp>
#include <queue>
#include <vector>

namespace aliSystem {
  namespace Threading {
    namespace {

      struct InitObj : public aliSystem::Component {
      protected:
	void Init();
	void Fini();
      };

      struct Item {
	using Ptr       = std::shared_ptr<Item>;
	using ItemQueue = std::priority_queue<Ptr,
					      std::vector<Ptr>,
					      std::function<bool(Ptr,Ptr)>>;
	Time::TP   targetTime;
	Queue::Ptr targetQueue;
	Work::Ptr  targetWork;
	static bool Less(const Ptr &a, const Ptr &b);
      };
      
      struct Monitor {
	using Ptr = std::shared_ptr<Monitor>;
	Monitor();
	static void Run(const Ptr &ptr, bool &requeue);
	void Run(bool &requeue);
	void Post() { sem.Post(); }
	void Schedule(Item::Ptr &item);
      private:
	std::mutex      lock;
	Item::ItemQueue pending;
	Semaphore       sem;
      };

      // ****************************************************************************************
      // init implementation
      Pool::Ptr    pool;
      Queue::Ptr   schedulingQueue;
      Queue::Ptr   sharedQueue;
      Monitor::Ptr monitor;
      void Init() {
	Stats::Ptr     sharedQueueStats;
	Stats::Ptr     monitorQueueStats;
	Stats::Ptr     monitorWorkStats;
	Semaphore::Ptr sem(new Semaphore);
	sharedQueueStats  = Stats::Create("Shared queue stats");
	monitorQueueStats = Stats::Create("Monitor queue stats");
	monitorWorkStats  = Stats::Create("Monitor work stats");
	pool              = Pool::Create("DelayedDispatcher", 2);
	schedulingQueue   = pool->AddQueue("scheduling queue", 1, monitorQueueStats);
	sharedQueue       = pool->AddQueue("shared queue", 1, sharedQueueStats);
	monitor.reset(new Monitor);
	Work::Ptr wPtr  = Work::Create(monitorWorkStats,
				       [=] (bool &requeue) {
					 Monitor::Run(monitor, requeue);
				       });
	schedulingQueue->AddWork(wPtr);
      }
      void Fini() {
	schedulingQueue->Stop();
	sharedQueue->Stop();
	schedulingQueue.reset();
	sharedQueue.reset();
	monitor->Post();
	pool->Stop(true);
	pool.reset();
	monitor.reset();
      }	

      // ****************************************************************************************
      // Item implementation
      bool Item::Less(const Ptr &a, const Ptr &b) {
	return b->targetTime < a->targetTime;
      }
      // ****************************************************************************************
      // MonitorWork implementation
      Monitor::Monitor()
	: pending(Item::Less) {
      }
      void Monitor::Run(const Ptr &ptr, bool &requeue) {
	if (ptr) {
	  ptr->Run(requeue);
	}
      }
      void Monitor::Run(bool &requeue) {
	requeue = true; // ignored when queue is stopped
	Time::TP tm     = Time::Now();
	Time::TP target = tm + std::chrono::seconds(30);
	if (true) {
	  std::lock_guard<std::mutex> g(lock);
	  while (!pending.empty() && pending.top()->targetTime <= tm) {
	    const Item::Ptr &iPtr = pending.top();
	    iPtr->targetQueue->AddWork(iPtr->targetWork);
	    pending.pop();
	  }
	  if (!pending.empty()) {
	    target = pending.top()->targetTime;
	  }
	}
	sem.TimedWait(target);
      }
      void Monitor::Schedule(Item::Ptr &item) {
	std::lock_guard<std::mutex> g(lock);
	pending.push(item);
	if (pending.top()==item) {
	  sem.Post();
	}
      }
    }

   void Scheduler::RegisterInitFini(ComponentRegistry &cr) {
      cr.Register("aliSystem::Threading::Scheduler", Init, Fini);
    }

    void Scheduler::Schedule(const Queue::Ptr &targetQueue,
		  const Time::TP   &targetTime,
		  const Work::Ptr  &targetWork) {
      if (targetQueue && targetWork) {
	Monitor::Ptr mPtr = monitor;
	if (mPtr) {
	  Item::Ptr item(new Item);
	  item->targetTime  = targetTime;
	  item->targetQueue = targetQueue;
	  item->targetWork  = targetWork;
	  mPtr->Schedule(item);
	}
      }
    }

    void Scheduler::Schedule(const Time::TP  &targetTime,
		  const Work::Ptr &targetWork) {
      Schedule(sharedQueue, targetTime, targetWork);
    }

  }
}
