#ifndef INCLUDED_ALI_SYSTEM_THREADING_SCHEDULER
#define INCLUDED_ALI_SYSTEM_THREADING_SCHEDULER

#include <aliSystem_threadingQueue.hpp>
#include <aliSystem_threadingWork.hpp>
#include <aliSystem_time.hpp>

namespace aliSystem {
  
  struct ComponentRegistry;
  
  namespace Threading {

    /// @brief Scheduler provides an interface for scheduling work
    ///        within a aliSystem::Threading::Pool.
    struct Scheduler {

      /// @brief Initialize hold module
      /// @param cr is a component registry to which any initialzation
      ///        and finalization logic should be registered.
      /// @note This function should only be called from aliSystem::RegisterInitFini.
      static void RegisterInitFini(ComponentRegistry &cr);
      
      ///
      /// \brief General work scheduler for aliSystem::Threading.
      ///
      /// The scheduler runs a background thread that takes care of injecting the targeted work
      /// functions into the targeted queue as soon after the targeted time as possible.  In
      /// general, this should be quite accurate, however if a large number of items are 
      /// specified to execute at the same time, they will be injected one after the other.
      /// Also delays can occur due to actual thread execution scheduling determined by the OS.
      ///
      ///   \param targetQueue - the queue in which the target work will be injected.
      ///   \param targetWork  - the work that will be injected into the targeted queue.
      ///   \param targetTime  - the time at which the work should be injected.
      ///
      ///  Notes, this library does not currently support work revocation.  In general, one can
      ///  define their work objects however they like.  To support revocable work at this time,
      ///  simply design the work object to support revocation by a flag or weak pointer or
      ///  something of that nature that will allow its Work::Run function to essentially do
      ///  nothing.
      ///
      static void Schedule(const Queue::Ptr &targetQueue,
			   const Time::TP   &targetTime,
			   const Work::Ptr  &targetWork);

      ///
      /// \brief General work scheduler for aliSystem::Threading.
      ///
      /// This scheduling function is identical to the function where a targetQueue is passed
      /// with the exception that the work function is schedule to execute on a generic queue
      /// maintained by this module.  This generic queue restricts concurrency and is only
      /// appropriate for work that is quick to execute and where the work's targeted time
      /// is somewhat flexible since it could be delayed by other work scheduled on this
      /// generic pool.
      ///
      static void Schedule(const Time::TP  &targetTime,
			   const Work::Ptr &targetWork);

    };
    
  }
}

#endif
