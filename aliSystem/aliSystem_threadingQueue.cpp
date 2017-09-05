#include <aliSystem_threadingQueue.hpp>
#include <aliSystem_logging.hpp>
#include <aliSystem_statsGuard.hpp>
#include <aliSystem_threadingWork.hpp>
#include <exception>

namespace aliSystem {
  namespace Threading {

    // ****************************************************************************************
    // Queue implementation
    Queue::Ptr Queue::Create(const std::string    &name,
			     const Semaphore::Ptr &semPtr,
			     size_t               maxConcurrency,
			     const Stats::Ptr     &queueStats) {
      Ptr rtn(new Queue);
      rtn->THIS         = rtn;
      rtn->name         = name;
      rtn->queueStats   = queueStats;
      rtn->stoppedStats = Stats::Create(name+":stopped");
      rtn->semPtr       = semPtr;
      rtn->onIdle       = QListeners::Create(name+"-onIdle");
      rtn->SetMaxConcurrency(maxConcurrency);
      return rtn;
    }
    Queue::~Queue() {}
    const std::string            &Queue::Name        () const { return name;         }
    const Stats::Ptr             &Queue::QueueStats  () const { return queueStats;   }
    const Stats::Ptr             &Queue::StoppedStats() const { return stoppedStats; }
    bool                          Queue::IsBusy      () const { return isBusy;       }
    bool                          Queue::IsStopped   () const { return isStopped;    }
    bool                          Queue::IsFrozen    () const { return isFrozen;     }
    const Queue::QListeners::Ptr &Queue::OnIdle      () const { return onIdle;       }
    size_t                        Queue::CurrentConcurrency() const { return curConcurrency; }
    size_t                        Queue::GetMaxConcurrency () const { return maxConcurrency; }
    size_t                        Queue::NumPending        () const { return pending.size(); }
    void Queue::Freeze() {
      // perform this action under the lock so that it is not called
      // in the middle of post or next or some other time that
      // maxConcurrency is being used to control the object.
      std::lock_guard<std::mutex> g(lock);
      isFrozen = true;
    }
    void Queue::SetMaxConcurrency(size_t maxConcurrency_) {
      std::lock_guard<std::mutex> g(lock);
      THROW_IF(isFrozen && maxConcurrency!=maxConcurrency_,
	       "Attempt to change the maximum concurrency after the queue has been frozen");
      size_t orig = maxConcurrency;
      maxConcurrency = maxConcurrency_;
      for (size_t i=orig; i<maxConcurrency_; ++i) {
	Post(g);
      }
    }
    void Queue::AddWork(const Work::Ptr &work) {
      std::lock_guard<std::mutex> g(lock);
      AddWork(g,work);
    }
    void Queue::Stop() {
      std::lock_guard<std::mutex> g(lock);
      isStopped = true;
      posted    = 0;
    }
    void Queue::StopAfter(const Work::Ptr &last) {
      THROW_IF(!last, "Attempt to add undefined work");
      std::lock_guard<std::mutex> g(lock);
      if (last) {
	AddWork(g, last);
	AddWork(g, Threading::Work::Create(stoppedStats,
					   [=](bool &requeue) {
					     requeue = false;
					     Ptr ptr = THIS.lock();
					     if (ptr) {
					       ptr->Stop();
					     }
					   }));
      }
    }
    void Queue::Start() {
      bool notify = false;
      if (true) {
	std::lock_guard<std::mutex> g(lock);
	if (isStopped) {
	  isStopped = false;
	  notify = pending.empty() && curConcurrency==0;
	  for (size_t i=0;i<maxConcurrency && i<pending.size();++i) {
	    Post(g);
	  }
	}
      }
      if (notify) {
	onIdle->Notify(THIS);
      }
    }
    void Queue::Clear() {
      std::lock_guard<std::mutex> g(lock);
      pending.clear();
      posted = 0;
    }



    std::ostream &operator<<(std::ostream &out, const Queue &o) {
      out << "threadingQueue(name = "   << o.name
	  << ", isBusy = "              << YES_NO(o.isBusy)
	  << ", isStopped = "           << YES_NO(o.isStopped)
	  << ", posted = "              << o.posted
	  << ", number pending = "      << o.pending.size()
	  << ", current concurrency = " << o.curConcurrency
	  << ", max concurrency = "     << o.maxConcurrency
	  << ")";
      return out;
    }



    void Queue::AddWork(std::lock_guard<std::mutex> &g, const Work::Ptr &work) {
      THROW_IF(!work, "Attempt to add undefined work");
      isBusy = true;
      pending.push_back(work);
      Post(g);
    }
    void Queue::Post(std::lock_guard<std::mutex> &) {
      if (!isStopped && posted+curConcurrency < maxConcurrency && pending.size()>posted) {
	++posted;
	semPtr->Post();
      }
    }
    void Queue::Run(const Work::Ptr &work) {
      bool notify = false;
      if (work) {
	bool requeue = false;
	std::exception_ptr p;
	try {
	  StatsGuard statsGuard(queueStats);
	  work->Run(requeue);
	} catch (std::exception &e) {
	  p = std::current_exception();
	}
	if (requeue) { AddWork(work); }
	std::lock_guard<std::mutex> g2(lock);
	--curConcurrency;
	Post(g2);
	if (curConcurrency==0 && posted==0 && pending.empty() && !isStopped) {
	  notify = true;
	  isBusy = false;
	}
	if (p) {
	  std::rethrow_exception(p);
	}
      }
      if (notify) {
	// outside of lock so a listener can post to the queue
	onIdle->Notify(THIS);
      }
    }
    Work::Ptr Queue::Next() {
      Work::Ptr next;
      std::lock_guard<std::mutex> g(lock);
      if (!isStopped
	  && posted>0
	  && curConcurrency<maxConcurrency
	  && !pending.empty()) {
	next = pending.front();
	pending.pop_front();
	--posted;
	++curConcurrency;
      }
      return next;
    }


    
    Queue::Queue()
      : isBusy(false),
	isStopped(false),
	isFrozen(false),
	maxConcurrency(0),
	curConcurrency(0),
	posted(0) {
    }


    
  }
}
