#ifndef INCLUDED_ALI_SYSTEM_STATS_GUARD
#define INCLUDED_ALI_SYSTEM_STATS_GUARD

#include <aliSystem_stats.hpp>
#include <aliSystem_time.hpp>


namespace aliSystem {

  ///
  /// @brief A stats object utility class.
  ///
  /// A stats guard is used to track time for a a specific stats object
  /// and increment those stats when the guard goes out of scope.
  ///
  struct StatsGuard {
    /// @brief stats guard constructor
    /// @param sPtr pointer to a stats object
    /// @note if the passed stats pointer is null, this
    ///       class has no effect.
    StatsGuard(Stats::Ptr sPtr);

    /// @brief stats guard destructor
    ~StatsGuard();
    
  private:
    
    Stats::Ptr sPtr;
    Time::TP   startTime;
  };

  
}

#endif
