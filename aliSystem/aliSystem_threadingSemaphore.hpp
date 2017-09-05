#ifndef INCLUDED_ALI_SYSTEM_THREADING_SEMAPHORE
#define INCLUDED_ALI_SYSTEM_THREADING_SEMAPHORE

#include <aliSystem_time.hpp>
#include <memory>
#include <mutex>
#include <semaphore.h>

namespace aliSystem {
  namespace Threading {

    /// @brief semaphore class
    struct Semaphore {
      using Ptr = std::shared_ptr<Semaphore>;  ///< shared poiter

      /// @brief construct a semaphore pointer
      /// @param count specifies the initial count for the semaphore
      /// @return the constructed semaphore pointer.
      static Ptr Create(size_t count=0);

      /// @brief constructor
      /// @param count specifies the initial count for the semaphore
      explicit Semaphore(size_t count=0);

      /// @brief destructor
      ~Semaphore();

      /// @brief post will increment the semaphore
      /// @return 0 on success, non-zero indicates an error
      int Post();
      
      /// @brief wait will wait until the semaphore count exceeds 0
      /// @return 0 on success, non-zero indicates an error
      int Wait();

      /// @brief Timedwait will wait until the count exceeds 0 or the time
      /// reaches the passed value.
      /// @param tp is the absolute time to wait until
      /// @return 0 on success, non-zero indicates an error
      /// @note The passed time is an absolute utc time.
      int TimedWait(const Time::TP &tp);
      
    private:
      
      sem_t  sem; ///< raw os semaphore
    };

  }
}

#endif
