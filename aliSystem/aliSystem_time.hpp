#ifndef INCLUDED_ALI_SYSTEM_TIME
#define INCLUDED_ALI_SYSTEM_TIME

#include <chrono>
#include <ostream>

namespace aliSystem {

  /// @brief Time provides a simplified interface for
  ///        commont time related logic.
  struct Time {

    using CLOCK = std::chrono::high_resolution_clock;  ///< clock type
    using TP    = CLOCK::time_point;                   ///< time point type
    using Dur   = CLOCK::duration;                     ///< durationn type

    /// @brief obtain the current time
    static TP Now();
    
    /// @brief Convert a duration to a double value.
    /// @param dur is a duration to convert.
    static double ToSeconds(const Dur &dur);

    /// @brief Convert a double value to a duration.
    /// @param delayInSeconds is a floating point number.
    /// @note The routine will attempt to retain nanosecond
    ///       precision.
    static Dur FromSeconds(double delayInSeconds);

    /// @brief convert a time point to a time_t value.
    /// @param tp is the time point to convert
    /// @return the equivalent time_t
    static time_t ToTime(const TP &tp);

    /// @brief convert a time_t to a time_t value.
    /// @param t is the t_time to convert
    /// @return the equivalent time point
    static TP FromTime(const time_t &t);

    /// @brief convert a time point to a struct timespec
    /// @param tp is the time point to convert
    /// @return the equivallent time spec
    static struct timespec ToTimeSpec(const TP &tp);

    /// @brief convert a struct timespec to a time point
    /// @param ts is the timespec to convert
    /// @return the equivallent time point
    static TP FromTimeSpec(const struct timespec &ts);

  };

}

std::ostream &operator<<(std::ostream &out, const aliSystem::Time::TP  &tp);
std::ostream &operator<<(std::ostream &out, const aliSystem::Time::Dur &dur);


#endif
