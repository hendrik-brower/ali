#ifndef INCLUDED_ALI_SYSTEM_REGISTRY
#define INCLUDED_ALI_SYSTEM_REGISTRY

#include <map>
#include <memory>
#include <mutex>

namespace aliSystem {

  ///
  /// @brief The registry is a generic container for maintaining a collection of objects.
  ///
  /// This class is used to enable easily registering and releasing  up items.  Though
  /// one can only extract a void pointer, the primary intent of this class is to
  /// simpify maintaining the life cycle of objects in other classes (or free functions)
  /// that need the ability to simply hold ownership and potentially managing relasing it
  /// without really needing to deal with the pointers directly.  Callers adding items
  /// to the registry could fetch the item, but its generally easier for them to hold
  /// a weak pointer and simply obtain the pointer from the weak pointer.   Or, if the
  /// caller needs to share ownership for some period, it can simply retain a shared
  /// pointer.
  ///
  struct Registry {
    using Ptr     = std::shared_ptr<Registry>;      ///< shared pointer
    using ItemPtr = std::shared_ptr<void>;          ///< item pointer
    using ItemMap = std::map<const void*, ItemPtr>; ///< map of item pointer's address to the item pointer

    /// @brief Create an object registry
    /// @param name name of the registry
    static Ptr Create(const std::string &name);

    /// @brief retrieve the registry's name
    /// @return the name of the registry
    const std::string &Name() const;

    /// @brief Register an object within the registry
    /// @param ptr the object to store
    void Register(const ItemPtr &ptr);

    /// @brief Unregister an object (remove the object from the registry)
    /// @param ptr item to remove from the registry
    /// @note items are stored by the raw address, so the item that the ItemPtr
    ///       is pointing to is the object that will be released.
    /// @note If the object does not exit, this function has no effect.
    void Unregister(const ItemPtr &ptr);

    /// @brief Unregister an object (remove the object from the registry)
    /// @param addr raw address of an object that was formerly stored
    /// @note If the object does not exit, this function has no effect.
    void Unregister(const void *addr);

    /// @brief Retrieve the shared pointer of a previously stored object.
    /// @param addr the raw address of the original item that was stored.
    /// @return a shared void pointer containing the referenced object.
    /// @note If the passed addr does not match any item in the registry
    ///       a null pointer is returned.
    ItemPtr Get(const void *addr);

    /// @brief registry destructor
    ~Registry();
    
  private:

    /// @brief registry constructor
    Registry();

    std::string name; ///< registry's name
    std::mutex  lock;   ///< lock used to serialize access to items
    ItemMap     items;  ///< items managed by an instance
  };
  
}

#endif
