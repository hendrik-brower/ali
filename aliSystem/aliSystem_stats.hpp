#ifndef INCLUDED_ALI_SYSTEM_STATS
#define INCLUDED_ALI_SYSTEM_STATS

#include <aliSystem_time.hpp>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <time.h>

namespace aliSystem {

  ///
  /// @brief A stats object for recording runtie statisics.
  ///
  struct Stats {
    using Ptr  = std::shared_ptr<Stats>;  ///< shared pointer
    using WPtr = std::weak_ptr  <Stats>;  ///< weak pointer

    /// @brief create a stats object
    /// @param name a description of the stats object.
    /// @return a stats object
    static Ptr Create(const std::string &name);

    /// @brief stats construtor
    /// @param name of the stats object
    Stats(const std::string &name);

    /// @brief stats destructor
    ~Stats();

    /// @brief Name returns the name of the stats object
    /// @return the stats object's name
    const std::string &Name() const;

    /// @brief Count returns the current count of the stats object
    /// @return the current count
    size_t Count() const;

    /// @brief RunTime returns the total accumlated time for a stats object.
    /// @return the total accumulated time for the stats object.
    Time::Dur RunTime() const;

    /// @brief Inc increments the stats object's count and its execution time
    /// The incremental time is computed using the passed number of sconds.
    /// @param timeInc amount to increment the stats object's time
    void Inc(const Time::Dur &timeInc);
    
    /// @brief serialize the given stats object to the given stream
    /// @param out stream to which the stats object should be written
    /// @param o the object to write
    friend std::ostream &operator<<(std::ostream &out, const Stats &o);
    
  private:
    
    std::mutex  lock;    ///< lock used to guard manipulations of the count & runTime
    std::string name;    ///< name of the stats instance
    size_t      count;   ///< stats count
    Time::Dur   runTime; ///< stats runTime
  };

}

#endif
