#ifndef INCLUDED_ALI_SYSTEM_THREADING_WORK
#define INCLUDED_ALI_SYSTEM_THREADING_WORK

#include <aliSystem_stats.hpp>
#include <functional>
#include <memory>
#include <ostream>
#include <string>

namespace aliSystem {
  namespace Threading {

    /// @brief Work encapsulates a unit of logic to run through a Threading::Queue.
    ///
    /// See documentation about Threading::Pool for more information about
    /// the interactions between a Threading::Pool and a Threading::Queue.  These
    /// are the two main elements related to the Work object.
    ///
    /// In general, a unit of work is an arbitrary function call.  The signature
    /// includes a boolean reference, requeue, that if set to true by the WorkFn,
    /// the Threading::Queue will requeue the work item.  This behavior is dependent
    /// upon the Threading::Queue class, or if some alternative execution management
    /// API, that alternative API.
    ///
    /// The Work object is very flexible, and although one should probably not extend
    /// the class, the work function can encapsulate additional data and API's to
    /// solve a wide variety of problems.  In an extreme case, a work object might
    /// simply point to some other mechanism that defines what it needs to be executed.
    /// In this case, this object could be considered merely a queueing and scheduling
    /// utility object.
    ///
    /// The pairing of stats with work might suggest that one would create a stats
    /// object for each work element.  This is not the case however.  Instead, it is
    /// recommended that a designer associate a type of work to a stats object.  In
    /// this way, the program will automatically compute runtime statistics for
    /// work flowing through the system by type.  If one processes a variety of jobs
    /// through the system, and each job runs some number of related or unrelated
    /// work elements, it might also make sense to manage statistics by these composite
    /// jobs.  Outside of the Work class, a Threading::Queue and Threading::Pool classes
    /// also provides hooks for collecting runtime statistics.
    struct Work {
      using Ptr    = std::shared_ptr<Work>;              ///< shared pointer
      using WorkFn = std::function<void(bool &requeue)>; ///< work function signature

      /// @brief Create a work object
      /// @param stats the stats object to use to track execution time.
      ///        This value may be null if one does not wish to accumulate
      ///        statistics.
      /// @param workFn is the function to run when the item is to be processed.
      static Ptr Create(const aliSystem::Stats::Ptr &stats,
			const WorkFn                &workFn);

      /// @brief Work destructor.
      ~Work();

      /// @brief GetStats retrieves the stats pointer
      /// @return the work objects stats container
      const aliSystem::Stats::Ptr &GetStats() const;

      /// @brief Run will trigger the execution of the workFn associated
      //  with the object.
      /// @param requeue defaults to false (though this is dependent on the
      ///        container calling the Run function, normally it would be
      ///        Threading::Queue).  If set to true before returning, it
      ///        tells the caller that the work unit should be requeued
      ///        for subsequent execution.
      /// @note The use of a requeue parameter rather than a member variable
      ///       in the Work object allows a single work object to run through
      ///       multiple queues and independently decide whether or not it
      ///       needs to requeue itself independently of other instances.
      void Run(bool &requeue) const;

      /// @brief stream output function for the work object.
      /// @param out the stream to which teh object should be serialized
      /// @param o the object to serialize
      /// @return the stream that was parked as out
      friend std::ostream &operator<<(std::ostream &out, const Work &o);
      
    private:
      
      /// @brief work constructor.
      Work();
      
      WorkFn            workFn; ///< instance's work fucntion
      Stats::Ptr        stats;  ///< instance's stats object (may be null)
    };

  }
}

#endif
