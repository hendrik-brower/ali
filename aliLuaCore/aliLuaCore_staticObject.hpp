#ifndef INCLUDED_ALI_LUA_CORE_STATIC_OBJECT
#define INCLUDED_ALI_LUA_CORE_STATIC_OBJECT

#include <aliLuaCore_object.hpp>

namespace aliLuaCore {

  /// @brief The Object class provides a utility inferface for transfering
  ///        c++ object to and from a Lua interpreter.
  ///
  /// This class essentially extends the interface of MT to associate it
  /// with a specific object type, which offers improved type safety as
  /// well as simpler and easier to read code.
  ///
  /// In some cases, a developer will prefer a Object.  This interface
  /// is realy only preferable if the objects defined when  your executable
  /// is compiled.  If they are dynamically defined during the execution of
  /// the program, refer to the Object class.
  ///
  /// @tparam the object to be managed
  /// @tparam AliasId is a unique number used to distinguish objects
  ///         so that one can define different MT's to the same object
  /// @note An alias might be used to define a read-only StaticObject
  ///       or a ReadWrite StaticObject that refer to the same c++ type (T).
  template <typename T, int AliasId=0>
  struct StaticObject {
    using TPtr    = std::shared_ptr<T>;    ///< shared pointer to the StaticObject's type
    using WTPtr   = std::weak_ptr<T>;      ///< weak pointer to the StaticObjet's type
    using ExecPtr = std::shared_ptr<Exec>; ///< forward declaration

    /// @brief Initialize a StaticObject.
    /// @param name name of type to define
    /// @param fnMap function map defining funtions for the wrapped MT object
    /// @param allowDup pass through to MT::Create's allowDup
    /// @return A pointer to a Object
    /// @note name must be a globally unique string for all defined MT's,
    ///       Object's, and StaticObjects.
    static void Init(const std::string      &name,
		     const FunctionMap::Ptr &fnMap,
		     bool allowDup);
    
    /// @brief Initialize a StaticObject.
    /// @param name name of type to define
    /// @param fnMap function map defining funtions for the wrapped MT object
    /// @param allowDup pass through to MT::Create's allowDup
    /// @param serializeFn pass through to MT::Create's serializeFn
    /// @param deserializeFn pass through to MT::Create's deserializeFn
    /// @return A pointer to a Object
    /// @note name must be a globally unique string for all defined MT's,
    ///       Object's, and StaticObjects.
    static void Init(const std::string      &name,
		     const FunctionMap::Ptr &fnMap,
		     bool allowDup,
		     MT::SerializeFn serializeFn,
		     MT::DeserializeFn deserializeFn);
    
    /// @brief Uninitialize a StaticObject
    static void Fini();
    
    /// @brief Retrieve the Object pointer associated with the
    ///        given StaticObject.
    /// @return an Object pointer
    static typename Object<T>::Ptr GetPtr() { return ptr; }

    /// @brief Return the MT associated with the StaticObject.
    /// @return MT pointer.
    static const MT::Ptr &GetMT();
    
    /// @brief Fetch the name of the StaticObject.
    /// @return the name
    static const std::string &Name();
    
    /// @brief push the name of the StaticObject onto the Lua stack
    /// @param L Lua state pointer
    /// @return 1, which is the number of items pushed into the
    ///         Lua stack.
    static int ToString(lua_State *L);
    
    /// @brief Fetch the serialzation of an instance of the StaticObject's type value
    /// @param tPtr pointer to an object of the type manged by this StaticObject
    /// @return serialization of the passed StaticObject's value
    static std::string GetString(const TPtr &tPtr);
    
    /// @brief Push an instance of the StaticObject's type onto the Lua stack
    /// @param L Lua stack to push the value
    /// @param tPtr the value to push
    /// @note tPtr may be null
    static int Make(lua_State *L, const TPtr &tPtr);
    
    /// @brief Push a weak instance of the StaticObject's type onto the Lua stack
    /// @param L Lua stack to push the value
    /// @param wtPtr the value to push
    /// @note wtPtr may be null
    static int MakeWeak(lua_State *L, const WTPtr &wtPtr);
    
    /// @brief GetMakeFn return a MakeFn for constructing an instance of
    ///        the StaticObject's managed type.
    /// @param tPtr a pointer to an istance of the StaticObject's managed type
    ///        to wrap in a make fn
    /// @return the MakeFn to enable constructing the passed instance.
    static MakeFn GetMakeFn(const TPtr &tPtr);
    
    /// @brief GetMakeWeakFn return a MakeFn for constructing a weak instance
    ///        of the StaticObject's managed type.
    /// @param wtPtr a weak pointer to an istance of the StaticObject's managed
    ///        type to wrap in a make fn
    /// @return the MakeFn to enable constructing the passed instance.
    static MakeFn GetMakeWeakFn(const WTPtr &wtPtr);
    
    /// @brief Check to see if a given stack value is the type managed
    ///        by the Object.
    /// @param L is the Lua State
    /// @param index is the stack index tocheck
    /// @return true if the given index is a type managed by this StaticObject.
    ///         False otherwise.
    /// @note This function will return true for managed values that
    ///       contain a null pointer or strong pointer.
    static bool Is(lua_State *L, int index);
    
    /// @brief Retrieve a manged value from the given stack index
    /// @brief L the Lua state from which the value is sought
    /// @brief index the index from which the value should be retrievevd
    /// @brief allowNull defines the behavior if the specified index
    ///        does not point to a value of the managed type.
    /// @note If allowNull is false, this function will throw an
    ///       exception if the give value is not the same type as the
    ///       StaticObject's managed value.
    /// @note If allow null is true, this call will return a null pointer
    ///       for any value other than a value that is the same type as
    ///       the type managed by the StaticObject and the value holds a
    ///       valid pointer (rather than being a null pointer value).
    static TPtr Get(lua_State *L, int index, bool allowNull);
    
    /// @brief GetTableValue is used to retrieve a value from a table.
    /// @param L the Lua State
    /// @param tableIndex is the stack index of the table from which
    ///        a value is sought.
    /// @param key is the field in which the sought value is stored
    /// @param tPtr a reference to the value to populate with the
    ///        value held in the given field
    /// @param allowNull is a flag to indicate what to do if the
    ///        give key does not contain a value that is the same type
    ///        as the StaticObject's managed type or if it does, but that
    ///        value is null.
    /// @note This call will throw an exception if the give value
    ///       is not of the expected type and allowNull is false.
    /// @note This funciton will return a null pointer only if
    ///       allowNull is true and the target value is null, or
    ///       or the given value is not the same type os the 
    ///       StaticObject's managed type.
    /// @note This function only retrieves string keys. The string value
    ///       "N" may be retrieved, but the table's index value N should
    ///       be retrieved via the StaticObject::GetTableIndex function.
    static void GetTableValue(lua_State *L,
			      int tableIndex,
			      const std::string &key,
			      TPtr &tPtr,
			      bool allowNull);
    
    /// @brief GetTableIndex is used to retrieve a value from a table.
    /// @param L the Lua State
    /// @param tableIndex is the stack index of the table from which
    ///        a value is sought.
    /// @param index is the table index in which the sought value is
    ///        stored
    /// @param tPtr a reference to the value to populate with the
    ///        value held in the given field
    /// @param allowNull is a flag to indicate what to do if the
    ///        give key does not contain a value that is the same type
    ///        as the StaticObject's managed type or if it does, but that
    ///        value is null.
    /// @note This call will throw an exception if the give value
    ///       is not of the expected type and allowNull is false.
    /// @note This funciton will return a null pointer only if
    ///       allowNull is true and the target value is null, or
    ///       or the given value is not the same type os the
    ///       StaticObject's managed type.
    /// @note This function only retrieves integer indexes. To retrieve
    ///       a value stored in a string key, use the function
    ///       StaticObject::GetTableValue.
    static void GetTableIndex(lua_State *L,
			      int tableIndex,
			      int index,
			      TPtr &tPtr,
			      bool allowNull);
    
    /// @brief TableValueIs will check to see if a given table field is
    ///        the same type as the type managed by the StaticObject.
    /// @param L is the Lua state
    /// @param tableIndex is the index of the table
    /// @param key is the field to check
    /// @return true if the given table field holds a type managed by
    ///         by the StaticObject.
    /// @note This function does not indicate whether the given field holds
    ///       a valid value.  If the value is a null pointer to a type managed
    ///       by this StaticObject, the function will return true.
    /// @note To check a numeric index, see StaticObject::TableIndexIs.
    static bool TableValueIs (lua_State *L, int tableIndex, const std::string &key);
    
    /// @brief TableIndexIs will check to see if a given table index is
    ///        the same type as the type managed by the StaticObject.
    /// @param L is the Lua state
    /// @param tableIndex is the index of the table
    /// @param index is the index within the table to check
    /// @return true if the given table's index holds a type managed by
    ///         by the StaticObject.
    /// @note This function does not indicate whether the given index holds
    ///       a valid value.  If the value is a null pointer to a type managed
    ///       by this StaticObject, the function will return true.
    /// @note To check a field, see StaticObject::TableValueIs.
    static bool TableIndexIs (lua_State *L, int tableIndex, int index);
    
    /// @brief Register the StaticObject's MT into a give exec engine.
    /// @param ePtr the Exec to which the MT should be registered
    static void Register(const ExecPtr &ePtr);
    
  private:
    
    /// @brief wasInit is a flag indicating the object has been initialized
    static bool wasInit;
    
    /// @brief ptr holds a constructed Object pointer.   The StaticObject
    ///        is merely a static class wrapping a global Object.
    static typename Object<T>::Ptr ptr;
  };

  template<typename T, int AliasId>
  void StaticObject<T,AliasId>::Init(const std::string      &name,
				     const FunctionMap::Ptr &fnMap,
				     bool allowDup) {
    THROW_IF(ptr || wasInit, "Attempt to reinitialize a StaticObject: " << name);
    ptr = Object<T>::Create(name, fnMap, allowDup);

  }
  template<typename T, int AliasId>
  void StaticObject<T,AliasId>::Init(const std::string      &name,
				     const FunctionMap::Ptr &fnMap,
				     bool allowDup,
				     MT::SerializeFn serializeFn,
				     MT::DeserializeFn deserializeFn) { 
    THROW_IF(ptr || wasInit, "Attempt to reinitialize a StaticObject: " << name);
    ptr = Object<T>::Create(name, fnMap, allowDup, serializeFn, deserializeFn);
  }
  template<typename T, int AliasId>
  void StaticObject<T,AliasId>::Fini() {
    ptr.reset();
  }
  //
  // General interface
  template<typename T, int AliasId>
  const MT::Ptr &StaticObject<T,AliasId>::GetMT() {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->GetMT();
  }
  template<typename T, int AliasId>
  const std::string &StaticObject<T,AliasId>::Name() {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->Name();
  }
  template<typename T, int AliasId>
  int StaticObject<T,AliasId>::ToString(lua_State *L) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->ToString(L);
  }
  template<typename T, int AliasId>
  std::string StaticObject<T,AliasId>::GetString(const TPtr &tPtr) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->GetString(tPtr);
  }
  template<typename T, int AliasId>
  int StaticObject<T,AliasId>::Make(lua_State *L, const TPtr &tPtr) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->Make(L,tPtr);
  }
  template<typename T, int AliasId>
  int StaticObject<T,AliasId>::MakeWeak(lua_State *L, const WTPtr &wtPtr) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->MakeWeak(L,wtPtr);
  }
  template<typename T, int AliasId>
  MakeFn StaticObject<T,AliasId>::GetMakeFn(const TPtr &tPtr) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->GetMakeFn(tPtr);
  }
  template<typename T, int AliasId>
  MakeFn StaticObject<T,AliasId>::GetMakeWeakFn(const WTPtr &wtPtr) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->GetMakeWeakFn(wtPtr);
  }
  template<typename T, int AliasId>
  bool StaticObject<T,AliasId>::Is(lua_State *L, int index) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->Is(L,index);
  }
  template<typename T, int AliasId>
  typename StaticObject<T,AliasId>::TPtr StaticObject<T,AliasId>::Get(lua_State *L, int index, bool allowNull) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->Get(L,index,allowNull);
  }
  template<typename T, int AliasId>
  void StaticObject<T,AliasId>::GetTableValue(lua_State *L,
					      int tableIndex,
					      const std::string &key,
					      typename StaticObject<T,AliasId>::TPtr &tPtr,
					      bool allowNull) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    ptr->GetTableValue(L,tableIndex,key,tPtr, allowNull);
  }
  template<typename T, int AliasId>
  void StaticObject<T,AliasId>::GetTableIndex(lua_State *L,
					      int tableIndex,
					      int index,
					      typename StaticObject<T,AliasId>::TPtr &tPtr,
					      bool allowNull) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    ptr->GetTableIndex(L,tableIndex,index,tPtr, allowNull);
  }
  template<typename T, int AliasId>
  bool StaticObject<T,AliasId>::TableValueIs (lua_State *L, int tableIndex, const std::string &key) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->TableValueIs(L, tableIndex, key);
  }
  template<typename T, int AliasId>
  bool StaticObject<T,AliasId>::TableIndexIs (lua_State *L, int tableIndex, int index) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->TableIndexIs(L, tableIndex, index);
  }
  template<typename T, int AliasId>
  void StaticObject<T,AliasId>::Register(const ExecPtr &ePtr) {
    THROW_IF(!ptr, "Call to uninitialized StaticObject function");
    return ptr->Register(ePtr);
  }
  template<typename T, int AliasId>
  bool StaticObject<T,AliasId>::wasInit = false;
  template<typename T, int AliasId>
  typename Object<T>::Ptr StaticObject<T,AliasId>::ptr;

  
}
#endif
  
