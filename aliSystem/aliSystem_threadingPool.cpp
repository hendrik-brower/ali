#include <aliSystem_threadingPool.hpp>
#include <aliSystem_logging.hpp>
#include <aliSystem_statsGuard.hpp>
#include <aliSystem_threadingQueue.hpp>
#include <aliSystem_threadingSemaphore.hpp>
#include <thread>

namespace aliSystem {
  namespace Threading {
  
    // ****************************************************************************************
    // Pool Implementation
    Pool::Ptr Pool::Create(const std::string &name,
			   size_t             numThreads,
			   const Stats::Ptr  &stats) {
      Ptr rtn(new Pool);
      rtn->THIS       = rtn;
      rtn->name       = name;
      rtn->numThreads = 0;
      rtn->run        = true;
      rtn->sPtr.reset(new Semaphore);
      rtn->stats      = stats ? stats : Stats::Create(name + " stats");
      rtn->SetNumThreads(numThreads);
      return rtn;
    }
    const std::string &Pool::Name() const { return name; }
    const Stats::Ptr &Pool::GetStats() { return stats; }
    void Pool::Flush() {
      static Stats::Ptr flushStats = Stats::Ptr(new Stats("flushing queues"));
      size_t sz;
      Semaphore::Ptr flushSem(new Semaphore);;
      Work::Ptr last = Work::Create(flushStats,
				    [=](bool &) {
				      if (flushSem) {
					flushSem->Post();
				      }
				    });
      if (true) {
	std::lock_guard<std::mutex> g(lock);
	sz = queues.size();
	for (QVec::iterator it=queues.begin(); it!=queues.end(); ++it) {
	  Queue::Ptr qPtr = *it;
	  if (qPtr) {
	    qPtr->StopAfter(last);
	  }
	}
      }
      for (size_t i=0;i<sz;++i) {
	flushSem->Wait();
      }
    }
    void Pool::Stop(bool wait) {
      if (true) {
	run = false;
	std::lock_guard<std::mutex> g(lock);
	for (size_t i=0;i<numThreads; ++i) {
	  sPtr->Post();
	}
      }
      while (wait && numThreads>0) {
	done.Wait();
      }
    }
    size_t Pool::GetNumThreads() const {
      return numThreads;
    }
    void Pool::SetNumThreads(size_t numThreads_) {
      std::lock_guard<std::mutex> g(lock);
      Ptr pool = THIS.lock();
      if (pool) {
	run = true;
	for (size_t i=numThreads;i<numThreads_; ++i) {
	  std::thread t(Run, pool);
	  t.detach();
	}
      }
    }
    Queue::Ptr Pool::AddQueue(const std::string &queueName,
			      size_t             maxConcurrency,
			      const Stats::Ptr  &stats) {
      Queue::Ptr rtn = Queue::Create(queueName, sPtr, maxConcurrency, stats);
      std::lock_guard<std::mutex> g(lock);
      queues.push_back(rtn);
      return rtn;
    }
    std::ostream &operator<<(std::ostream &out, const Pool &o) {
      out << "aliSystem::Threading::Pool(name=" << o.Name() << ")";
      return out;
    }
    Pool::Pool() {}
    void Pool::Run(Ptr pool) {
      if (pool) {
	if (true) {
	  std::lock_guard<std::mutex> g(pool->lock);
	  ++pool->numThreads;
	}
	size_t     curIdx = 0;
	Queue::Ptr queue;
	Work::Ptr  work;
	while (pool->run) {
	  pool->Next(curIdx, queue, work);
	  if (work && queue) {
	    StatsGuard statsGuard(pool->stats);
	    queue->Run(work);
	  }
	  work.reset();
	  queue.reset();
	}
	if (true) {
	  std::lock_guard<std::mutex> g(pool->lock);
	  --pool->numThreads;
	}
	pool->done.Post();
      }
    }
    void Pool::Next(size_t &curIdx, Queue::Ptr &queue, Work::Ptr &work) {
      DEBUG("wating for next");
      sPtr->Wait();
      DEBUG("getting next");
      std::lock_guard<std::mutex> g(lock);
      size_t sz = queues.size();
      for (size_t i=0;i<sz;++i) {
	curIdx = (curIdx+1)%sz;
	DEBUG("trying " << curIdx << " of " << sz << " qs " << queues.size());
	queue  = queues[curIdx];
	DEBUG("queue " << *queue);
	work   = queue->Next();
	if (work) {
	  return;
	}
      }
      //ERROR_IF(run,"Nothing found");
    }

  }
}
