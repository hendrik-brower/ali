#ifndef INCLUDED_ALI_LUA_CORE_TABLE
#define INCLUDED_ALI_LUA_CORE_TABLE

#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_object.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_values.hpp>
#include <functional>
#include <string>
#include <set>
#include <vector>

struct lua_State;
namespace aliLuaCore {
  /// @brief The aliLuaCore::Table namespace defines functions and/or classes that
  ///        faciliate operations on Lua tables.
  struct Table {

    using SSet = std::set<std::string>;          ///< string set
    using SVec = std::vector<std::string>;       ///< string vector
    using SObj = aliSystem::Codec::Serializer;   ///< serializer
    using DObj = aliSystem::Codec::Deserializer; ///< deserializer

    /// @brief IsNil will check if a given table key is null.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @returns boolean if the value is nil, false if it is not nil
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static bool IsNil(lua_State         *L,
		      int                tableIndex,
		      const std::string &key);
    
    /// @brief SetNil will set the given key of the referenced table to nil.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetNil (lua_State         *L,
			int                tableIndex,
			const std::string &key);
    
    /// @brief GetInteger retrieves an integer from a table.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @param defaultValue is a value that will be set if allowNil is true and the
    ///        extratced value is nil.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetInteger(lua_State         *L,
			   int                tableIndex,
			   const std::string &key,
			   int               &value,
			   bool               allowNil,
			   int                defaultValue=0);
    
    /// @brief Set the given table value with the passed integer.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetInteger(lua_State         *L,
			   int                tableIndex,
			   const std::string &key,
			   int                value);
    
    /// @brief Extract a double value held by the given table and key.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @param defaultValue is a value that will be set if allowNil is true and the
    ///        extratced value is nil.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetDouble (lua_State         *L,
			   int                tableIndex,
			   const std::string &key,
			   double            &value,
			   bool               allowNil,
			   double             defaultValue=0);
    
    /// @brief Set the given table value with the passed double.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.

    static void SetDouble(lua_State         *L,
			  int                tableIndex,
			  const std::string &key,
			  double             value);
    
    /// @brief Extract a string value held by the given table and key.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @param defaultValue is a value that will be set if allowNil is true and the
    ///        extratced value is nil.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetString (lua_State         *L,
			   int                tableIndex,
			   const std::string &key,
			   std::string       &value,
			   bool               allowNil,
			   const std::string &defaultValue="");
    
    /// @brief Set the given table value with the passed string.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetString(lua_State         *L,
			  int                tableIndex,
			  const std::string &key,
			  const std::string &value);
    
    /// @brief Extract a boolean value held by the given table and key.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @param defaultValue is a value that will be set if allowNil is true and the
    ///        extratced value is nil.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetBool(lua_State         *L,
			int                tableIndex,
			const std::string &key,
			bool              &value,
			bool               allowNil,
			bool               defaultValue=false);
    
    /// @brief Set the given table value with the passed bool.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetBool(lua_State         *L,
			int                tableIndex,
			const std::string &key,
			bool               value);
    
    /// @brief Create a MakeFn that holds the value of the passed table ane key.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @param defaultValue is a value that will be set if allowNil is true and the
    ///        extratced value is nil.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetMakeFn (lua_State        *L,
			   int               tableIndex,
			   const std::string &key,
			   MakeFn            &value,
			   bool               allowNil,
			   const MakeFn      &defaultValue=Values::MakeNothing);
    
    /// @brief Set the given table value with the contents wrapped by the passed MakeFn.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note If the MakeFn string does not explode into 0 or 1 arguments, the function
    ///       will throw an exception.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetMakeFn (lua_State         *L,
			   int                tableIndex,
			   const std::string &key,
			   const MakeFn      &value);
    
    /// @brief Serialize the the value of a passed table and key.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param sObj is the serializer to use when creating the serialized
    ///        representation of the object.
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetSerializedValue(lua_State         *L,
				   int                tableIndex,
				   const std::string &key,
				   SObj              &sObj,
				   bool               allowNil);
    
    /// @brief Set the given table value with the contents wrapped by the deserialization
    ///        of the passed string.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param dObj is the deserializer (and encapsulated value) to use for setting the
    ///        passed key
    /// @note If the serialized string does not contain any value, the key is set to nil
    /// @note If the serialized string does not deserialize into 0 or 1 arguments, the
    ///       function will throw an exception.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetSerializedValue(lua_State         *L,
				   int                tableIndex,
				   const std::string &key,
				   DObj              &dObj);
    
    /// @brief Extract a sequence from the given table key and add the values into the
    ///        given MakeFn value.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @param allowNil if true means the value will be set to a default value, if
    ///        false and the value is nil, the function will throw an exception.
    /// @note This function differs from GetMakeFn in that it "unwarps the values from the table"
    ///       so the returned make function, when executed will push the contents of the sequence
    ///       on the Lua stack rather than a table with the contents (which is what GetMakeFn will
    ///       do.)
    /// @note This function will throw an exception if the key of the given table is not a
    ///       Lua table.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetSequence(lua_State         *L,
			    int                tableIndex,
			    const std::string &key,
			    MakeFn            &value,
			    bool               allowNil);
    
    /// @brief Extract a sequence from the given table key and add the values into the given SSet.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @note if the key is undefined, the function will not alter the passed SSet
    /// @note The passed value is not cleared prior to loading the contents of the sequence.
    ///       That is if the vector already has N elements, and Y elements are in the table key's
    ///       sequence then after this call, the vector will contain N+Y minues any values in Y
    ///       that were already in the SSet.
    /// @note This function will throw an exception if the key of the given table is not a
    ///       Lua table.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetStringSequence(lua_State         *L,
				  int                tableIndex,
				  const std::string &key,
				  SSet              &value);
    
    /// @brief Set the given table value with a sequence defined by the strings in the passed SSet.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetStringSequence(lua_State         *L,
				  int                tableIndex,
				  const std::string &key,
				  const SSet        &value);
    
    /// @brief Extract a sequece's values and insert them as strings into the passed SVec.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value is the variable for which the value will be set
    /// @note if the key is undefined, the function will not alter the passed SSet
    /// @note The passed value is not cleared prior to loading the contents of the sequence.
    ///       That is if the vector already has N elements, and Y elemnts are in the table key's
    ///       sequence then after this call, the vector will contain N+Y values.
    /// @note This function will throw an exception if the key of the given table is not a
    ///       Lua table.
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void GetStringSequence(lua_State         *L,
				  int                tableIndex,
				  const std::string &key,
				  SVec              &value);
    
    /// @brief Set the given table value with a sequence defined by the strings in the passed SVec.
    /// @param L is the Lua State
    /// @param tableIndex is the index on the Lua stack of the table.
    /// @param key is table key on which to operate
    /// @param value to use for setting the passed key
    /// @note This function will throw an exception if the stack index defined by
    ///       tableIndex is not a Lua table.
    static void SetStringSequence(lua_State         *L,
				  int                tableIndex,
				  const std::string &key,
				  const SVec        &value);
    
  };
  
}

#endif
