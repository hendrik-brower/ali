#ifndef INCLUDED_ALI_SYSTEM_THREADING_POOL
#define INCLUDED_ALI_SYSTEM_THREADING_POOL

#include <aliSystem_threadingQueue.hpp>
#include <aliSystem_threadingSemaphore.hpp>
#include <aliSystem_threadingWork.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace aliSystem {
  namespace Threading {

    ///
    /// @brief A Threading::Pool is used to process Thraeding::Work object
    /// on a set of associated Threading::Queue objects.
    ///
    /// In general, a pool is created, then queues are added to the pool.
    /// The pool maintains a set of processing threads, which can be increased
    /// as necessary or desired.  Work flows through the queues and is picked
    /// out for processing within the Pool as the work becomes avilable in
    /// the Threading::Queue and as the Threading::Queue releases it for
    /// execution.
    ///
    /// Ultimately the computer on which a program is running defines the
    /// amounnt of concurrency of a process, however the OS can interrupt these
    /// threads and multiplex the processing on the Pool's threads.  THe pools
    /// in contrast execute Threading::Work units and any thread within the pool
    /// can only process one work unit at a time.  The number of concurrent work
    /// items is limited by the number of threads within a threading pool.  Each
    /// Threading::Queue associated with the pool also has a concurrency limit.
    /// Though queues simply provide a queue of work, they make work units
    /// available to the thread pool, but even if a queue has more work than
    /// it's allocated concurrency, no more than the specified work units
    /// will be processed at any given time.
    ///
    /// A Threading::Pool can be stopped and restarted.  When stopped, it will
    /// shutdown threads as they complete their work units.  If there is no
    /// immediate work being processed on a thread, it will generally exit
    /// immediately, but given that the shutdown of the threads is completed
    /// on a thread, the thread might not be released immediately, but rather
    /// some short time after requesting the Pool to stop.
    ///
    /// One can also shutdown a Threading::Pool with a call that will not return
    /// until after the threads in the pool exit.  The time it takes to shutdown
    /// Threading::Pool completely depends on the work executing on its threads.
    /// In general, it is recommended to define Threading::Work units that
    /// execute in some reasonable time.  Preferably less then a few seconds,
    /// though depending on the application developers design, one might accept
    /// work units that take much longer to execute.
    ///
    /// The Threading::Pool object is relatively efficent, but it is not optimized
    /// for the highest possible performance.  Because of this, one should probably
    /// design Threading::Work units that are not too small.  Work units that execute
    /// on the order of micro seconds should be completely reasonable.  Nano-second
    /// work units might come with a relatively high overhead.  For systems that
    /// process large quantities of work units should pay more attention to the
    /// efficiencies.  A mostly idle system will not suffer significant performance
    /// penalities for quick executing work units.  However, regardless of the
    /// sustained efficiency, latency will always exist for individual work units.
    /// So even though a mostly idle system might not suffer negative aspects due
    /// to sustained efficiencies, its possible the overhead will negatively impacts
    /// their system's design when analyzing latency between enqueuing work to an
    /// empty queue and its execution.
    ///
    struct Pool {
      using Ptr  = std::shared_ptr<Pool>;   ///< shared pointer
      using WPtr = std::weak_ptr<Pool>;     ///< weak pointer
      using QVec = std::vector<Queue::Ptr>; ///< vector of queues

      /// @brief Create a thread pool.
      /// @param name name of the Threading::Pool
      /// @param numThreads initial number of threads for the pool.
      /// @param stats is a stats object to use to record stats for the
      ///        newly created thread pool.
      /// @note A Threading::Pool may be initialized with 0 initial threads
      ///       and later increased.
      /// @note A Threading::Pool's associated queues may allow more or less
      ///       concurrency than the thread pool.  The total limit of concurrency
      ///       will be the lessor of the thread pool's number of threads and
      ///       the sum of the associated queue's maximum concurrencies.
      static Ptr Create(const std::string &name,
			size_t             numThreads,
			const Stats::Ptr  &stats = nullptr);

      /// @brief retrieve the name of the pool
      /// @return the name of the thread pool
      const std::string &Name() const;

      /// @brief retrieve the thread pool's execution stats.
      /// @return stats describing the numer of processed work units
      ///          as well as the time spent processing those units.
      const Stats::Ptr &GetStats();

      /// @brief flush the associated Threading::Pool.
      ///
      /// Flush will return when whatever was in the queue at the time
      /// this function was called is completed.  It will also stop
      /// each queue presently in the thread pool.
      void Flush();

      /// @brief Stop the thread pool.
      ///
      /// Stop will shutdown the threads associated with this pool.  If
      /// wait is true, the function will not return until the last
      /// thread has exited.  If another thread calls SetNumThreads
      /// with a value greater than the number of threads live at that
      /// time, this function may not return until another call to
      /// stop is made (since the SetNumThreads sets the 'run' flag
      /// to true.
      void Stop(bool wait);

      /// @brief retrieve the number of threads in the thread pool
      /// @return number of threads
      size_t GetNumThreads() const;

      /// @brief SetNumThreads will increase the number of threads in the pool
      ///        if the passed number is larger then the current number of
      ///        threads.
      /// @param numThreads the minimum number of threads to run
      ///        in the thread pool
      /// @note If the queue was flagged to 'stop' and threads are exiting
      ///       while this function is executing, when the function returns
      ///       it is possible that the threads that were exiting in the
      ///       brief moment between when run is set true and the mutex
      ///       locked, will exit and thus the final thread count will be
      ///       less than the specified numThreads.
      ///       In general, it is expected that most uses of this library
      ///       will not repeatedly start & stop the pool.  If that is not
      ///       the case, then this aspect of the implementatoin should be
      ///       revisited.
      void SetNumThreads(size_t numThreads);

      /// @brief Add a queue to the thread pool.
      /// Queues are attached to a thread pool and cannot be trasferred
      /// to a different thread pool.
      /// @param queueName name for the queue to add
      /// @param maxConcurrency maximum initial concurrency for the
      ///        new queue.
      /// @param stats stats to associate with the new queue.
      /// @note It is possible to use the same stats object for multiple
      ///       queues.  How a designer leverages the queues and the stats
      ///       related to execution of work on those queues is up to the
      ///       system designer.
      Queue::Ptr AddQueue(const std::string &queueName,
			  size_t             maxConcurrency,
			  const Stats::Ptr  &stats);
      
      /// @brief Serialize a Pool
      /// @param out is the stream to write information about the pool
      /// @param o is the object to write to the stream
      /// @return the stream written to.
      friend std::ostream &operator<<(std::ostream &out, const Pool &o);
      
    private:

      /// @brief constructor for a pool.
      Pool();

      /// @brief A thread poool's thread function.
      /// Each thread within a pool will run this function.
      static void Run(Ptr pool);

      /// @brief fetch the next work unit.
      ///
      /// The pool currently uses a round robin technique for choosing
      /// the next queue from which to draw work.  In the future more
      /// advanced scheduling algorithms may be exposed via the queue's
      /// stats, which will allow more advanced scheduling algorithms.
      /// @param curIdx [in/out] last index from which a work unit was
      ///        drawn for execution.  Currently each thread maintains its
      ///        own curIdx, so the distribution of execution of queues
      ///        may not be truely round robin, but rather some close
      ///        approximation.
      /// @param queue [out] queue from which work was selected.
      ///        If a non-null queue pointer is returned, then
      ///        the returned work should also be non-nil.
      /// @param work [out] the next unit of work
      void Next(size_t &curIdx, Queue::Ptr &queue, Work::Ptr &work);

      std::mutex     lock;        ///< lock used to guard manipulations to various members
      WPtr           THIS;        ///< weak pointer to this object
      std::string    name;        ///< name of the pool
      size_t         numThreads;  ///< numThreads current count of live threads
      bool           run;         ///< flag to indicate whether the threads should run or exit
      QVec           queues;      ///< vector of associated queues
      Semaphore::Ptr sPtr;        ///< semaphore used by queues to trigger fetch cycles
      Semaphore      done;        ///< done semaphore used to coordinate stop that waits
      Stats::Ptr     stats;       ///< stats for the pool's execution
    };

  }
}

#endif
