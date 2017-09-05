#ifndef INCLUDED_ALI_SYSTEM_HOLD
#define INCLUDED_ALI_SYSTEM_HOLD

#include <memory>
#include <string>
#include <vector>

namespace aliSystem {

  struct ComponentRegistry;
  /// @brief Hold provides a mechanism for dynamically defining the process life cycle.
  ///
  /// Hold represents a global state of the application.  When the application is
  /// started various bits of code can grap a hold object and when all of these
  /// objects are released, any call to WaitForHold (a blocking call) is released.
  ///
  /// The library provides a decenralized mechanism for defining the state of a
  /// process.  Any library can obtain a hold and particpate in the management of
  /// the processes life cycle.  This can lead to circumstances where code improperlly
  /// obtains or retains a hold.  This would result in a process that does not
  /// terminate as expected.  Since holds are fetched with the file & line number
  /// as well as a reason for the hold, these issues should be relatively easy to
  /// debug.
  /// 
  struct Hold {
    using Ptr  = std::shared_ptr<Hold>; ///< shared pointer
    using WPtr = std::weak_ptr<Hold>;   ///< weak pointer
    using Vec  = std::vector<Ptr>;      ///< vector of shared pointers
    
    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliSystem::RegisterInitFini.
    static void RegisterInitFini(ComponentRegistry &cr);

    /// @brief Creaate allows construting a hold object.
    /// @param file this should be passed as the compiler macro __FILE__
    ///        or some other appropriate value tha would indicate the source
    ///        obtaining the hold.
    /// @param line this should be passed as the compiler macro __LINE__.
    /// @param reason a string explaining why the hold was obtained.
    static Ptr Create(const std::string &file, size_t line, const std::string &reason);

    /// WaitForHolds will block until all holds have been released.
    static void WaitForHolds();

    /// GetHolds will retrieve a list of holds that are currently outstanding.
    /// @param holds a vector to which the existing holds will be copied.
    /// @note holds returned in the vector will held by the vector as well
    ///       as any original requester, which if not released, may block
    ///       WaitForHolds calls logner then appropriate.
    static void GetHolds(Vec &holds);

    /// @brief returns a pointer to the file pointer passed when creating the hold.
    /// returns file name
    const std::string &GetFile() const;

    /// @brief returns the line number passed when creating the hold.
    /// returns the line
    size_t GetLine() const;

    /// @brief returns the reason passed when creating thee hold.
    /// @return the reason
    const std::string &GetReason() const;

    /// @brief hold destrutor
    ~Hold();
    
  private:
    
    /// @brief hold constructor
    Hold();

    std::string file;    ///< file name from which the hold was obtained
    size_t      line;    ///< line number defining where the hold was created
    std::string reason;  ///< reason for obtaining the hold
  };
  
}

#endif
