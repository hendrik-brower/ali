#include <aliSystem_threadingWork.hpp>
#include <aliSystem_logging.hpp>
#include <aliSystem_statsGuard.hpp>

namespace aliSystem {
  namespace Threading {

    Work::Ptr Work::Create(const aliSystem::Stats::Ptr &stats,
			   const WorkFn                &workFn) {
      THROW_IF(!workFn, "Attempt to create undefined work");
      Ptr rtn(new Work);
      rtn->workFn = workFn;
      rtn->stats  = stats;
      return rtn;
    }
    Work::~Work() {}
    const aliSystem::Stats::Ptr &Work::GetStats() const {
      return stats;
    }
    void Work::Run(bool &requeue) const {
      requeue = false;
      if (workFn) {
	StatsGuard g(stats);
	workFn(requeue);
      }
    }
    std::ostream &operator<<(std::ostream &out, const Work &o) {
      aliSystem::Stats::Ptr stats = o.stats;
      if (stats) {
	out << "threadingWork(stats = " << stats->Name() << ")";
      } else {
	out << "threadingWork()";
      }
      return out;
    }
    Work::Work() {}

  }
}
