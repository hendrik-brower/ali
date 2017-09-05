#include <aliSystem_threadingSemaphore.hpp>
#include <time.h>

namespace aliSystem {
  namespace Threading {

    Semaphore::Ptr Semaphore::Create(size_t count) {
      return Ptr(new Semaphore(count));
    }

    Semaphore::Semaphore(size_t count) {
      sem_init(&sem, 0, count);
    }
    Semaphore::~Semaphore() {
      sem_destroy(&sem);
    }
    int Semaphore::Post() {
      return sem_post(&sem);
    }
    int Semaphore::Wait() {
      return sem_wait(&sem);
    }
    int Semaphore::TimedWait(const Time::TP &tm) {
      static struct timespec   waitTime;
      static const long long   nano = 1000*1000*1000;
      std::chrono::nanoseconds dur  = tm.time_since_epoch();
      waitTime.tv_sec  = dur.count()/nano;
      waitTime.tv_nsec = dur.count()%nano;
      return sem_timedwait(&sem, &waitTime);
    }

  }
}
