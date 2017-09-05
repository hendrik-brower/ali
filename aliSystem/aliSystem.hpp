#ifndef INCLUDED_SYSTEM
#define INCLUDED_SYSTEM

#include <aliSystem_basicCodec.hpp>
#include <aliSystem_codec.hpp>
#include <aliSystem_codecDeserialize.hpp>
#include <aliSystem_codecDeserializer.hpp>
#include <aliSystem_codecSerialize.hpp>
#include <aliSystem_codecSerializer.hpp>
#include <aliSystem_component.hpp>
#include <aliSystem_componentRegistry.hpp>
#include <aliSystem_hold.hpp>
#include <aliSystem_listener.hpp>
#include <aliSystem_listeners.hpp>
#include <aliSystem_logging.hpp>
#include <aliSystem_registry.hpp>
#include <aliSystem_stats.hpp>
#include <aliSystem_statsGuard.hpp>
#include <aliSystem_threading.hpp>
#include <aliSystem_threadingPool.hpp>
#include <aliSystem_threadingQueue.hpp>
#include <aliSystem_threadingScheduler.hpp>
#include <aliSystem_threadingSemaphore.hpp>
#include <aliSystem_threadingWork.hpp>
#include <aliSystem_time.hpp>
#include <aliSystem_util.hpp>

/// @brief The aliSystem namespace defines the basic application level
///        elements the ali interfaces.
namespace aliSystem {

  /// @brief RegisterInitFini triggers the registration of any
  ///        aliSystem::Components defined in the aliSystem library.
  /// @param cr is a component registry to which any initialzation
  ///        and finalization logic should be registered.
  void RegisterInitFini(ComponentRegistry &cr);

}

#endif
