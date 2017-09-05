#ifndef INCLUDED_ALI_LUA_CORE_MAKE_TABLE_UTIL
#define INCLUDED_ALI_LUA_CORE_MAKE_TABLE_UTIL

#include <aliLuaCore_types.hpp>
#include <memory>
#include <mutex>
#include <string>

struct lua_State;
namespace aliLuaCore {


    /// @brief MakeTableUtil enable the ability to define simple or complex tables that can later be serialized to
    ///        a Lua interpreter.
    /// @note For some simpler sequences such as string lists defined from vectors or sets of strings,
    ///       one might find functions from the aliLuaCore::Tables namespace a more convenient interface.
    /// @note This class essentially defines a vector of key value pairs.  If one defines a key, then
    ///       subsequently redefines it, when the table is transfered to a Lua interperter, the first
    ///       value will be written with any subsequent values.   No optimizations to minimize
    ///       inefficiencies in memory use by the MakeTableUtil class or the added IO to the Lua
    ///       interpreter to address.  Nor are they likely to be pursued.  As the frequency of this
    ///       behavior is generally controlled by the client code using this class and should be
    ///       relatively easy to minimize.  As this class is designed, this utilty provides tremendous
    ///       flexibility, which is its primary intent as a generic tool.
    /// @note When moving tables to and from C++, unless they are "transfered" as a reference to an
    ///       interpreter and a stack location (or maybe a copied reference held in some other are of
    ///       the Lua state), the table is extracted and re-inserted as a copy.  Thus, changes to the
    ///       table once its pushed back to an interpreter are not reflected in the original and vis
    ///       versa.
    /// @note At present, this class does not provide any utility for extracting a table from an interpreter.
    ///       This capabibility might be worth adding in the future as it gives a modifable interface rather
    ///       than the more opaque MakeFn that something like aliLuaCore::Values::GetMakeFn woudl provide.
    /// @note Data strutures contained in this class are protected with a mutex, so concurrent access and/or
    ///       manipluation of a specific instance is valid.  However, any MakeFn that is held in this
    ///       object, must also be multi-threaded safe, otherwise calls to MakeTableUtil::Make or when running
    ///       functions returned from MakeTableUtil::GetMakeFn, may not be thread safe.  All functions
    ///       defined in the aliLuaCore namespace produce thread-safe functions.
    struct MakeTableUtil {
      using Ptr = std::shared_ptr<MakeTableUtil>;  ///< Shared pointer to the MakeTableUtil

      /// @brief constructor
      MakeTableUtil();
      
      /// @brief Copy constructor is deleted.
      MakeTableUtil(const MakeTableUtil &) = delete;
      
      /// @brief Assignment operator is is deleted.
      MakeTableUtil &operator=(const MakeTableUtil &) = delete;
      
      /// @brief define a generic key/value pair for the table.
      /// @param key is a Make function that should produce 1 value, which will be used as the key.
      /// @param value is a MakeFn that defines the value for the given key.
      /// @note If the make function for either the key or the value produces zero or more than one
      ///       value, when one attempts to write the table to a Lua state, that call will throw an
      ///       exception.
      void Set(const MakeFn &key,
	       const MakeFn &value);

      /// @brief set a boolean value for a string key for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetBoolean(const std::string &key, bool   value);

      /// @brief set an integer value for a string key for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetNumber (const std::string &key, int    value);

      /// @brief set a double value for a string key for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetNumber  (const std::string &key, double value);

      /// @brief set a string value for a string key for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetString (const std::string &key, const std::string &value);

      /// @brief set a MakeFn value for a string key for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      /// @note If the make function produces zer or more than one value, when one attempts to write the
      ///       table to a Lua state, that call will throw an exception.
      void SetMakeFn (const std::string &key, const MakeFn  &value);

      /// @brief set a boolean value for an integer index for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetBooleanForIndex(int key, bool value);

      /// @brief set an integer value for an integer index for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetNumberForIndex(int key, int value);

      /// @brief set a double value for an integer index for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetNumberForIndex(int key, double value);

      /// @brief set a string value for an integer index for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      void SetStringForIndex(int key, const std::string &value);

      /// @brief set a MakeFn value for an integer index for the given instance of the MakeTableUtil object.
      /// @param key is the table key to set
      /// @param value is the value to set
      /// @note If the make function produces zero or more than one value, when one attempts to write the
      ///       table to a Lua state, that call will throw an exception.
      void SetMakeFnForIndex(int key, const MakeFn  &value);

      /// @brief Define a subtable for the given string key.  The returned MakeTableUtil pointer can be used
      ///        to modify the subtable.  It can also be used to push just that subtable to an interpreter.
      /// @return A pointer to a MakeTableUtil that defines the subtable.
      Ptr CreateSubtable(const std::string &key);

      /// @brief Define a subtable for the given integer index key.  The returned MakeTableUtil pointer can be used
      ///        to modify the subtable.  It can also be used to push just that subtable to an interpreter.
      /// @return A pointer to a MakeTableUtil that defines the subtable.
      Ptr CreateSubtableForIndex(int key);

      /// @brief Create a subtable with a defined by a MakeFn.
      /// @param keyMakeFn is the value to use to define the key for a sub table.
      /// @return A shared pointer to a the subtable's MakeTableUtil object.  This object may be manipulated to
      ///         add values ot the sub table.  It may also be handled as a separate table for any other purpose.
      /// @note When modified, these modificatoins will be reflected in any previously retreived MakeFn obtained
      ///       through calls to MakeTableUtil::GetMakeFn.
      Ptr CreateSubtable(const MakeFn &keyMakeFn);

      /// @brief Generate a make function for the current data encapsulated by the instance of the MakeTableUtil
      ///        object.
      /// @return A MakeFn that when run will serilize the table to the passed Lua State.
      /// @note When this function is called, a copy of the items is used to create the make function.  Thus,
      ///       changes to this table after this call do not generally affect the state of the object returned
      ///       from this call.  However, because MakeFn's define a very flexible interface, it is possible that
      ///       a specific make function, which might represent a single key, value, or a whole subtable, could
      ///       be altered in such a way that the returned MakeFn could change from the time at which it was
      ///       created.  The subtable is another element that might affect this behavior.  In the specific case
      ///       of a subtable, any change to the sub table after calls to MakeTableUtil::GetMakeFn will be reflected
      ///       in any subsequent exectuion of the returned make function.
      /// @note The returned MakeFn may throw an exception when it is executed if any key or value elemnts defined
      ///       by a MakeFn that push anything other than just 1 element to the Lua state's stack.
      MakeFn GetMakeFn();

      /// @brief Generate a table object defined by the current contents of the given instance of the MakeTableUtil.
      ///        This call is essentially the same as mtu.GetMakeFn()(L), where mtu is an instance of the
      ///        MakeTableUtil class.
      /// @param L the Lua state in which the table should be pushed.
      /// @return This function will return 1, indicating the number of items pushed into the Lua interpreter.
      ///         It does not offer any indication of how many elements were pushed and poped during the process
      ///         of pushing the table into the Lua State.
      /// @note This call can throw an exception if any of the keys or values attempt to push anything other than
      ///       a single value.
      int Make(lua_State *L);

      /// @brief push a list of KVVec pairs into a table at the specified index.
      /// @param L the Lua state to modify.
      /// @param tableIndex the index of a table in the passed Lua State for which the specified key/value pairs
      ///        should be added.
      /// @param items a vector of key/value pairs to add to the given table.
      /// @return This function will return the number of values left on the stack when it returns.  It should be
      ///         zero.
      /// @note This function will throw and exeption if the passed tableIndex is not a table.
      /// @note This function will throw an excpetion if any of the first (key) or second (value) MakeFn values
      ///       held by teh KVVec produce anything other than a single item on the stack.
      static int Push(lua_State *L, int tableIndex, const KVVec &items);

    private:

      std::mutex lock;  ///< lock to guard access to the items member.
      KVVec      items; ///< items defines the list of key value pairs that define the table.
    };

}

#endif
