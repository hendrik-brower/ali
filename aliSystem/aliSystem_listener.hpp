#ifndef INCLUDED_ALI_SYSTEM_LISTENER
#define INCLUDED_ALI_SYSTEM_LISTENER

#include <aliSystem_logging.hpp>
#include <aliSystem_util.hpp>
#include <memory>
#include <string>

namespace aliSystem {

  ///
  /// @brief A listener is an object that encapsulates a notify function to be 
  /// called as a notification representing the occurance of some event.
  ///
  /// Listeners contain a description so that a set of listener objects may
  /// be seletively triggered.  Refer to the collection of Listeners::Notify
  /// functions for more details on this.
  ///
  template<typename DataType_, typename InfoType_=std::string>
  struct Listener {
    using Ptr      = std::shared_ptr<Listener>; ///< shared pointer
    using WPtr     = std::weak_ptr  <Listener>; ///< weak pointer
    using DataType = DataType_;                 ///< data type
    using InfoType = InfoType_;                 ///< info type
    using NotifyFn = Util::NotifyFn<DataType>;  ///< notify function

    /// @brief construct a listener with the given name and notify function.
    /// @param info for a listener
    /// @param notifyFn is the function to call when being notified
    static Ptr Create(const InfoType &info, const NotifyFn &notifyFn);
    
    /// @brief listener destructor
    ~Listener();
    
    /// @brief retrieve info about the listener
    /// @return the info passed when creating the listener
    const InfoType &Info() const;
    
    /// @brief call the notify function for the listener
    void Notify(DataType t);
    
  private:
    
    /// @brief listener constructor
    Listener();
    
    InfoType    info;     ///< info about the listener
    NotifyFn    notifyFn; ///< notification function
  };


  // ****************************************************************************************
  // Implementation
  template<typename DataType_, typename InfoType_>
  typename Listener<DataType_,InfoType_>::Ptr
  Listener<DataType_,InfoType_>::Create(const InfoType &info, const NotifyFn &notifyFn) {
    Ptr rtn(new Listener);
    rtn->info     = info;
    rtn->notifyFn = notifyFn;
    return rtn;
  }
  template<typename DataType_, typename InfoType_>
  Listener<DataType_,InfoType_>::~Listener() {}
  template<typename DataType_, typename InfoType_>
  const InfoType_ &Listener<DataType_,InfoType_>::Info() const { return info; }
  template<typename DataType_, typename InfoType_>
  void Listener<DataType_,InfoType_>::Notify(DataType data) {
    notifyFn(data);
  }
  template<typename DataType_, typename InfoType_>
  Listener<DataType_,InfoType_>::Listener() {}

}

#endif
