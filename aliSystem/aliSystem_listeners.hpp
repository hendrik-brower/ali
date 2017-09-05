#ifndef INCLUDED_ALI_SYSTEM_LISTENERS
#define INCLUDED_ALI_SYSTEM_LISTENERS

#include <aliSystem_listener.hpp>
#include <aliSystem_logging.hpp>
#include <aliSystem_util.hpp>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace aliSystem {

  ///
  /// @brief Listeners provides a container for functions that should
  /// be triggered as a collection.
  ///
  /// Calls to Notify can trigger all or a subset of notification 
  /// functions. Additionally, the commit function will be called 
  /// from behind the listeners mutex so callers triggering a
  /// notification can use this class as a latch guard.
  ///
  template<typename DataType_, typename InfoType_=std::string>
  struct Listeners {
    using DataType   = DataType_;                                   ///< data type
    using InfoType   = InfoType_;                                   ///< info type
    using Ptr        = std::shared_ptr<Listeners>;                  ///< shared pointer
    using LPtr       = typename Listener<DataType,InfoType>::Ptr;   ///< shared pointer to a listener
    using WLPtr      = typename Listener<DataType,InfoType>::WPtr;  ///< weak pointer to a listener
    using LMap       = std::unordered_map<void*, LPtr>;             ///< maintain shared ptr, key is the raw address
    using WLMap      = std::unordered_map<void*, WLPtr>;            ///< maintain weak ptr, key is the raw address
    using NotifyFn   = Util::NotifyFn<DataType>;                    ///< specialized notify fn
    using SelectFn   = Util::SelectFn<InfoType>;                    ///< specialized select fn
    using CommitFn   = Util::CommitFn;                              ///< commit fn

    /// @brief create a collection of listeners
    /// @param name is simply a name to describe the collection of listeners
    static Ptr  Create(const std::string &name);

    /// @brief Create and register a listener.
    /// If the listeners pointer is not initialized this funtion will throw
    /// an exception.  The returned pointer may be held by a clien to extend
    /// the life of the listener.  If useWeak is false, the life of the returned
    /// pointer (and its registration to the listeners objectc) will span the
    /// greater of the callers retention or the life of the listeners objectc.
    /// If useWeak is true, the life of the listener will be limited to the
    /// callers use.  When a weakly held listener is released by the caller,
    /// the listeners object will essentially unregister the litener.
    /// @param ptr listeners to which a listener should be registered
    /// @param info describing the listener.  This value can be used
    ///        by the listeners object when notifying registered listeners
    ///        with some criteria.
    /// @param notifyFn the function to call when a notification is triggered
    /// @param useWeak flag to indicate whether the listeners object should
    ///        leave ownership to the caller (true) or should share
    ///        ownership of the listener object (false).
    /// @note If the passted listeners pointer is null, this call will throw
    ///       an exception.
    static LPtr Register(const Ptr      &ptr,
			 const InfoType &info,
			 const NotifyFn &notifyFn,
			 bool            useWeak);

    /// @brief register a listener created elsewhere
    /// See the previous form of register for more details.
    /// @param ptr listeners to which a listener should be registered.
    /// @param lPtr listener to register
    /// @param useWeak flag to indicate whether the registered listener
    ///        should be owned only by the caller (thus unregistering
    ///        the listener from the listeners object when the client
    ///        releases the pointer), or shared the the listeners object.
    /// @note If the listeners object pointer or listener object pointer is
    ///       null, the function will throw an exception.
    static void Register(const Ptr   &ptr,
			 const LPtr  &lPtr,
			 bool         useWeak);

    /// @brief name of the listeners object
    /// @return the name given the object when it was created
    const std::string &Name();

    /// @brief register a listener created elsewhere
    /// See the previous form of register for more details.
    /// @param lPtr listener to register
    /// @param useWeak flag to indicate whether the registered listener
    ///        should be owned only by the caller (thus unregistering
    ///        the listener from the listeners object when the client
    ///        releases the pointer), or shared the the listeners object.
    /// @note If the listener object pointer is null, the function will throw an
    ///       exception.
    void Register(const LPtr &lPtr, bool useWeak);

    /// @brief notify all registered listeners.
    ///
    ///        For listeners registered with weak pointers, if they are released
    ///        before this function triggers their notification function, they
    ///        will be unregistered.
    /// @param arg is an argument that is passed to the listener's notify function
    /// @return true
    bool Notify(DataType arg);

    /// @brief notify all registered listeners.
    /// For listeners registered with weak pointers, if they are released before
    /// this function triggers their notification function, they will be unregistered.
    /// @param arg is an argument that is passed to the listener's notify function
    /// @param selectFn Function used to determine which listeners to notify.
    ///        The select function is passed the info of each
    ///        listener and only the ones for which the function returns
    ///        true will be notified.
    /// @return true
    bool Notify(DataType arg, const SelectFn &selectFn); // Notify selected

    /// @brief notify all registered listeners.
    /// For listeners registered with weak pointers, if they are released before
    /// this function triggers their notification function, they will be unregistered.
    /// @param arg is an argument that is passed to the listener's notify function
    /// @param commitFn The commit function is called just before notifying
    ///        all registered listeners.  If the commit function returns
    ///        false, no listeners are notified.
    /// @return the same result as the commit function
    /// @note the commit function is called under the class's general mutex.  The commit
    ///       function should not call back into other fuctions (directly or indirectly)
    ///       in this class that also lock the listeners object's mutex.
    /// @note The commit function allows classes holding a listeners object the ability
    ///       to use the atomic behavior associated with adding listeners and notifying
    ///       them to coordinate behaviors surrounding the management of registered
    ///       listeners and the notification of them.
    bool Notify(DataType arg, const CommitFn &commitFn);

    /// @brief notify selected registered listeners.
    /// For listeners registered with weak pointers, if they are released before
    /// this function triggers their notification function, they will be unregistered.
    /// @param arg is an argument that is passed to the listener's notify function
    /// @param selectFn Function used to determine which listeners to notify.
    ///        The select function is passed the info of each
    ///        listener and onnly the ones for which the function returns
    ///        true will be notified.
    /// @param commitFn The commit function is called just before notifying
    ///        all registered listeners.  If the commit function returns
    ///        false, no listeners are notified.
    /// @return the same result as the commit function
    /// @note The commit function is called under the class's general mutex.  The commit
    ///       function should not call back into other fuctions (directly or indirectly)
    ///       in this class that also lock the listeners object's mutex.
    /// @note The commit function allows classes holding a listeners object the ability
    ///       to use the atomic behavior associated with adding listeners and notifying
    ///       them to coordinate behaviors surrounding the management of registered
    ///       listeners and the notification of them.
    bool Notify(DataType        arg,
		const SelectFn &selectFn,
		const CommitFn &commitFn);

    /// @brief listeners destrutor
    ~Listeners();
    
  private:

    /// @brief listeners construtor
    Listeners();

    /// @brief internal registeration function to ensure consistent behavior
    /// @param g a lock guard that should hold the lock for the listeners lock
    ///          member.
    /// @param lPtr listener to register
    /// @param useWeak flag indicating whether the given listener should
    ///        be solely owned by the caller (true) or jointly owned
    //         by both the caller and the listeners object (false);
    void Register(std::lock_guard<std::mutex> &g, const LPtr  &lPtr, bool useWeak);

    std::string name;  ///< name of listeners
    std::mutex  lock;  ///< lock for serializing updates to lMap & wlMap
    LMap        lMap;  ///< map of strong listners
    WLMap       wlMap; ///< map of weak listeners (purged in notification routines)
  };



  // ****************************************************************************************
  // Implementation
  template<typename DataType_, typename InfoType_>
  typename Listeners<DataType_, InfoType_>::Ptr Listeners<DataType_,InfoType_>::Create(const std::string &name) {
    Ptr rtn(new Listeners<DataType,InfoType>);
    rtn->name = name;
    return rtn;
  }
  template<typename DataType_, typename InfoType_>
  typename Listener<DataType_,InfoType_>::Ptr
  Listeners<DataType_,InfoType_>::Register(const Ptr      &ptr,
					   const InfoType &info,
					   const NotifyFn &notifyFn,
					   bool            useWeak) {
    typename Listener<DataType,InfoType>::Ptr
      lPtr = Listener<DataType,InfoType>::Create(info, notifyFn);
    Listeners<DataType,InfoType>::Register(ptr, lPtr, useWeak);
    return lPtr;
  }
  template<typename DataType_, typename InfoType_>
  void Listeners<DataType_,InfoType_>::Register(const Ptr   &ptr,
						const LPtr  &lPtr,
						bool         useWeak) {
    THROW_IF(!ptr,  "Listeners uninitialized");
    ptr->Register(lPtr, useWeak);
  }
  template<typename DataType_, typename InfoType_>
  const std::string &Listeners<DataType_,InfoType_>::Name() { return name; }
  template<typename DataType_, typename InfoType_>
  void Listeners<DataType_,InfoType_>::Register(const LPtr  &lPtr, bool useWeak) {
    std::lock_guard<std::mutex> g(lock);
    Register(g, lPtr, useWeak);
  }
  template<typename DataType_, typename InfoType_>
  bool Listeners<DataType_,InfoType_>::Notify(DataType arg) {
    return Notify(arg, Util::MatchAll<InfoType>, Util::Always);
  }
  template<typename DataType_, typename InfoType_>
  bool Listeners<DataType_,InfoType_>::Notify(DataType        arg,
					      const SelectFn &selectFn) {
    return Notify(arg, selectFn, Util::Always);
  }
  template<typename DataType_, typename InfoType_>
  bool Listeners<DataType_,InfoType_>::Notify(DataType        arg,
					      const CommitFn &commitFn) {
    return Notify(arg, Util::MatchAll<InfoType>, commitFn);
  }
  template<typename DataType_, typename InfoType_>
  bool Listeners<DataType_,InfoType_>::Notify(DataType        arg,
					      const SelectFn &selectFn,
					      const CommitFn &commitFn) {
    bool rtn = false;
    std::lock_guard<std::mutex> g(lock);
    if (!commitFn || commitFn()) {
      rtn = true;
      for (typename LMap::iterator it=lMap.begin(); it!=lMap.end(); ++it) {
	LPtr lPtr = it->second;
	if (selectFn(lPtr->Info())) {
	  lPtr->Notify(arg);
	}
      }
      for (typename WLMap::iterator it=wlMap.begin(); it!=wlMap.end();) {
	LPtr lPtr = it->second.lock();
	if (lPtr) {
	  ++it;
	  if (selectFn(lPtr->Info())) {
	    lPtr->Notify(arg);
	  }
	} else {
	  it = wlMap.erase(it);
	}
      }
    }
    return rtn;
  }
  template<typename DataType_, typename InfoType_>
  Listeners<DataType_,InfoType_>::~Listeners() {}
  template<typename DataType_, typename InfoType_>
  Listeners<DataType_,InfoType_>::Listeners() {}
  template<typename DataType_, typename InfoType_>
  void Listeners<DataType_,InfoType_>::Register(std::lock_guard<std::mutex> &,
						const LPtr &lPtr,
						bool useWeak) {
    THROW_IF(!lPtr, "Listener uninitialized");
    if (useWeak) {
      wlMap[lPtr.get()] = lPtr;
    } else {
      lMap[lPtr.get()] = lPtr;
    }
  }

  
}

#endif
