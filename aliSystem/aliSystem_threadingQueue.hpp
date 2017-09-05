#ifndef INCLUDED_ALI_SYSTEM_THREADING_QUEUE
#define INCLUDED_ALI_SYSTEM_THREADING_QUEUE

#include <aliSystem_listener.hpp>
#include <aliSystem_listeners.hpp>
#include <aliSystem_stats.hpp>
#include <aliSystem_threadingSemaphore.hpp>
#include <aliSystem_threadingWork.hpp>
#include <memory>
#include <list>
#include <mutex>
#include <ostream>

namespace aliSystem {
  namespace Threading {

    struct Pool;
    ///
    /// @brief A queue for storing pending work for a thread pool.
    ///
    /// The Threading::Queue can be constructed independently of
    /// a Threading::Pool; however, if one does, they should make sure
    /// they are driving it in an expected way.  For example, the
    /// maximum concurrency for a queue should not be exceeded by
    /// calling Queue::Run with work units that would exceed this number.
    /// Also, the Queue::Next function is paired with the Queue::Run,
    /// passing work to the Queue::Run function that was not extracted
    /// from the Queue::Next is inappropriate and may result in
    /// unexpected behavior.  As a final note in this space, one should
    /// realize that queues are defined for a specific purpose and the
    /// concurrency characteristics and the type of work pushed through
    /// a queue should align to the intended purpose of the queue.
    /// Failing to do this may result in unexpected system behaviors.
    ///
    struct Queue {
      using Ptr        = std::shared_ptr<Queue>;  ///< shared pointer
      using WPtr       = std::weak_ptr<Queue>;    ///< weak pointer
      using WorkQueue  = std::list<Work::Ptr>;    ///< list of work
      using QListener  = Listener<const WPtr&>;   ///< listener
      using QListeners = Listeners<const WPtr&>;  ///< listeners

      /// @brief Create a Threading::Queue.
      /// In general, one would not call this function directly.  Instead,
      /// they should obtain a queue from a call to Threading::Pool::Add.
      /// @param name name of the queue
      /// @param semPtr a semaphore pointer used to coordinate a queue
      ///        and its hosting Threading::Pool (or potentially some other
      ///        custom thread pool).  The semaphore is triggered whenever
      ///        work is available to execute. Typically this semaphore is
      ///        shared with every queue in a Threading::Pool and the POol
      ///        is responsible for finding a queue that has work to execute.
      ///        Extra triggering of this semaphore will cause the Pool to
      ///        hunt for a queue with work more then necessary, but aside from
      ///        some inefficient processor cycles, it should not cause serious
      ///        harm.
      /// @param maxConcurrency initial concurrency for the returned queue
      /// @param queueStats stats used to record stats related to the
      ///        execution of work units passing through the queue.  The
      ///        queue's stats need not be dedicaed to a single queue or only
      ///        to queue objects.  Its up to the system designer to choose
      ///        the meaning of stats objects and their associations.
      /// @return a queue object
      /// @note Queues are always created in a 'running' state. If one does not
      ///       want the queue to immediately start running added work, they
      ///       should stop the queue before adding any items.
      static Ptr Create(const std::string    &name,
			const Semaphore::Ptr &semPtr,
			size_t                maxConcurrency,
			const Stats::Ptr     &queueStats);

      /// @brief queue destructor
      ~Queue();

      /// @brief fetch the queue's name
      /// @return the queue's name
      const std::string &Name() const;

      /// @brief fetch the queue's work stats
      /// @return The stats associated with the work units passed through the queue.
      const Stats::Ptr &QueueStats() const;

      /// @brief fetch the queue's "last" marker stats
      /// @return The stats associated with any "last" terminators passed through
      ///          the queue.
      /// @note Whenever one calls StopAfter, two items are placed in the queue, one
      ///       is the last unit of work that the queue should perfome.  The second is
      ///       a marker that is used to stop the queue.  The stopped stats are associated
      ///       with these markers.  In general, their execution time will be very small
      ///       and more likely than not of little interest.  However, the stat's count
      ///       might indicate how many times the queue has been stopped.
      const Stats::Ptr &StoppedStats() const;

      /// @brief return an indicaton of the queue's state.
      /// @return the state of the queue
      /// @note A busy state is defined as the condition where either:
      ///       a) a queue has pending items
      ///       b) a queue has something currently executing through the queue
      /// @note One can trigger on a transition to idle state by adding a listener
      ///       to the 'OnIdle' Listeners object.
      bool IsBusy() const;

      /// @brief return an indication of the queue's run state.
      /// @return true if the queue has been stopped
      /// @note A queue that is in the process of stopping is not considered 'Stopped'
      /// @note A stopped queue's busy/idle state is computed the same way, so
      ///       an idle, but stopped queue will not immediately begin working when
      ///       a work unit is added, but once a work unit is added, the queue will
      //        report that it "is busy."
      bool IsStopped() const;

      /// @brief IsFrozen returns true if the queue has been frozen.
      /// @return true if the queue has been frozen, false otherwise.
      bool IsFrozen() const;

      /// @brief get the 'on idle' listeners object.
      /// When the queue becomes idle all listeners registered to the returned
      /// Listeners object will be notified.
      const QListeners::Ptr &OnIdle() const;

      /// @brief obtain the current concurrency for the queue
      /// @return the current concurrency for queue
      /// @note This should be a value between [0, max concurrency]
      /// @note This number may change between subsequent calls from any single
      ///       thread as work may begin or stop on other queues concurrently
      ///       in a way that would alter the current concurrency of a queue.
      size_t CurrentConcurrency() const;

      /// @brief maximume concurrency for a queue
      /// @return the current maximum concurrency for a queue
      size_t GetMaxConcurrency() const;

      /// @brief retrieve the number of pending work units
      /// @return the number of pending work units.
      size_t NumPending() const;

      /// @brief Freeze maximum concurrency value.
      /// @note Once called, the 'frozen' state of a queue cannot be removed.
      void Freeze();

      /// @brief Set the object's maximum concurrency.
      /// @param maxConcurrency the new concurrency limit
      /// @note Throws an exception if the object has been frozen
      ///       and the passed value is not equal to the current value.
      /// @note If the concurrency is less then the current value and
      ///       there is more work then the specified new concurrency,
      ///       then the new lower maximum concurrency will take affect
      ///       as active work units terminate.
      /// @note If the concurrency is greater than the current maximum,
      ///       then the max value will be increased and its possible
      ///       that a number of work units matching the incrase in
      ///       concurrency may begin executing immediately.
      void SetMaxConcurrency(size_t maxConcurrency);

      /// @brief add work to a queue
      /// Append work to the queue.  Note that all work is added to the
      /// end of a queue.  Despite this, it is possible to execute work
      /// in a different order, to do this, one can simply push work units
      /// that refer to work units from a differnet container and the
      /// work unit that they refer to depends on some alternative ordering
      /// algorithm.
      /// @param work work to add to the queue
      void AddWork(const Work::Ptr &work);

      /// @brief Stop will trigger the queue to stop returning work from
      /// the Next() function.
      /// Stop will not abort any currently running work, it can only
      /// prevent the extraction of new work.  Work is considered "currently
      /// executing work" the moment it is returned from a call to Next().
      void Stop();

      /// @brief Stop after works just like stop except that the queue
      /// is stopped after an internal work unit that triggers the stopping
      /// of a queue.
      /// @param last a unit of work after which, the queue should be stopped.
      void StopAfter(const Work::Ptr &last);

      /// @brief Re-start a queue queue.
      /// @note If one has previously called StopAfter, its possible that
      ///       the call to start will be received before the queue's
      ///       enqueued "stop work" has executed.  In which case, the
      ///       Start will have no affect and will Stop shortly after
      ///       the call to Start.
      void Start();

      /// @brief Drop all items from the queue.
      void Clear();

      /// @brief Write a short summary of the queue object to the given ostream.
      /// @param out the stream to write to
      /// @param o the Queue instance to write
      /// @return the stream passed as the out parameter
      friend std::ostream &operator<<(std::ostream &out, const Queue &o);

      /// @brief Run a work unit.
      /// Run is used to execute some work on the given queue,
      /// it should generally be running only something that was
      /// previously fetched from a call to Next.
      /// @param work pointer to item to execute under the queue,
      ///        this value should have been previously fetched from
      ///        a call to Next().  Items returned by Next(), should
      ///        note be passed to the this function exactly once and
      ///        in the same order as they are retireved.  Slight
      ///        misorderings due to races between threads should be
      ///        limited.  The Threading::Pool class ensures that this
      ///        order is maintained, though once Run is called, its
      ///        possible for a thread to be interrupted and another
      ///        threads that picks a subsequent object and passes it
      ///        through this function before the previous object's
      ///        thread resumes execution - thus resulting in slight
      ///        reorderings.  The only true way to ensure perfect
      ///        ordering is to limit a queue to a maximum concurrency
      ///        of 1.
      /// @note This function should only be used by the queue
      ///       owner (generally aliSystem::Threading::Pool).
      void Run(const Work::Ptr &work);

      /// @brief Fetch the next work unit.
      /// Next is used to fetch the next item to work on.  If the
      /// queue has no work, is stopped, or has dispatched its
      /// concurrency limit.
      /// @return work - null or a pointer to next work unit
      /// @note This function should only be used by the queue
      ///       owner (generally aliSystem::Threading::Pool).
      Work::Ptr Next();
      
    private:

      /// @brief an internal add function.
      /// This fucntion is used to add work to the queue and is defined
      /// to ensure consistency regardless of the location from which
      /// Add is called.
      /// @param g a lock guard that holds the queue's lock
      /// @param work the work to add to the queue
      /// @note The lock guard is not passed to this functcion, and whether
      ///       the passed guard holds the queue's lock is not verified.
      ///       The argumennt is meant to serve as a reminder to anyone
      ///       developing this class to ensure that they should only call
      ///       this function from a routine that already owns the lock.
      void AddWork(std::lock_guard<std::mutex> &g, const Work::Ptr &work);

      /// @brief post to the semaphore if the queue has pending work
      /// that has not already been signaled via the semaphore.
      /// @param g a lock guard that holds the queue's lock.
      /// @note See the note about the guard passed to the Add() function.
      void Post(std::lock_guard<std::mutex> &g);

      /// @brief queue's constructor.
      Queue();

      std::mutex      lock;            ///< lock used for various member values
      WPtr            THIS;            ///< weak pointer to self
      std::string     name;            ///< name of the queue
      Stats::Ptr      queueStats;      ///< stats for work passed through the queue
      Stats::Ptr      stoppedStats;    ///< stats for stop work passed throug the queue
      Semaphore::Ptr  semPtr;          ///< semaphore to trigger when work may be executed
      bool            isBusy;          ///< flag to indicate whether or not the queue is busy
      bool            isStopped;       ///< flag to indicate that the queue is stopped
      bool            isFrozen;        ///< flag to indicate that the max concurrency is fixed
      QListeners::Ptr onIdle;          ///< notifications sent when isBusy->false
      WorkQueue       pending;         ///< list of pending work
      size_t          maxConcurrency;  ///< maximum specified concurrency for an instance
      size_t          curConcurrency;  ///< current concurrency for an instance
      size_t          posted;          ///< variable to track number of outstandng posts
    };

  }
}

#endif

  
