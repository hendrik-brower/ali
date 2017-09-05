#ifndef INCLUDED_ALI_LUA_CORE_OBJECT
#define INCLUDED_ALI_LUA_CORE_OBJECT

#include <lua.hpp>
#include <aliLuaCore_functionMap.hpp>
#include <aliLuaCore_functions.hpp>
#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_util.hpp>
#include <aliLuaCore_values.hpp>
#include <aliSystem.hpp>
#include <memory>
#include <string>
#include <sstream>
#include <ostream>

namespace aliLuaCore {

  struct Exec;

  /// @brief The Object class provides a utility inferface for transfering
  ///        c++ object to and from a Lua interpreter.
  ///
  /// This class essentially extends the interface of MT to associate it
  /// with a specific object type, which offers improved type safety as
  /// well as simpler and easier to read code.
  ///
  /// In most cases, a developer will prefer a StaticObject.  This interface
  /// is realy only preferable if the objects are dynamically defined during
  /// the execution of the program rather than at compile time.
  ///
  /// @tparam the object to be managed
  template <typename T>
  struct Object {
    using Ptr     = std::shared_ptr<Object>; ///< shared pointer to the Object
    using TPtr    = std::shared_ptr<T>;      ///< shared pointer to the Object's type
    using WTPtr   = std::weak_ptr<T>;        ///< weak pointer to the Object
    using ExecPtr = std::shared_ptr<Exec>;   ///< forward declaration
    
    /// @brief Create an Object instance.
    /// @param name name of type to define
    /// @param fnMap function map defining funtions for the wrapped MT object
    /// @param allowDup pass through to MT::Create's allowDup
    /// @return A pointer to a Object
    /// @note name must be a globally unique string for all defined MT's,
    ///       Object's, and StaticObjects.
    static Ptr Create(const std::string      &name,
		      const FunctionMap::Ptr &fnMap,
		      bool allowDup);
    
    /// @brief Create an Object instance.
    /// @param name name of type to define
    /// @param fnMap function map defining funtions for the wrapped MT object
    /// @param allowDup pass through to MT::Create's allowDup
    /// @param serializeFn pass through to MT::Create's serializeFn
    /// @param deserializeFn pass through to MT::Create's deserializeFn
    /// @return A pointer to a Object
    /// @note name must be a globally unique string for all defined MT's,
    ///       Object's, and StaticObjects.
    static Ptr Create(const std::string      &name,
		      const FunctionMap::Ptr &fnMap,
		      bool allowDup,
		      MT::SerializeFn serializeFn,
		      MT::DeserializeFn deserializeFn);

    /// @brief Return the MT associated with the Object.
    /// @return MT pointer.
    const MT::Ptr &GetMT() const;
    
    /// @brief Fetch the name of the object.
    /// @return the name
    const std::string &Name() const;
    
    /// @brief push the name of the Object onto the Lua stack
    /// @param L Lua state pointer
    /// @return 1, which is the number of items pushed into the
    ///         Lua stack.
    int ToString(lua_State *L) const;
    
    /// @brief Fetch the serialzation of an instance of the Object's type value
    /// @param tPtr pointer to an object of the type manged by this Object
    /// @return serialization of the passed object
    std::string GetString(const TPtr &tPtr) const;
    
    /// @brief Push an instance of the Object's type onto the Lua stack
    /// @param L Lua stack to push the value
    /// @param tPtr the value to push
    /// @note tPtr may be null
    int Make(lua_State *L, const TPtr &tPtr) const;
    
    /// @brief Push a weak instance of the Object's type onto the Lua stack
    /// @param L Lua stack to push the value
    /// @param wtPtr the value to push
    /// @note wtPtr may be null
    int MakeWeak(lua_State *L, const WTPtr &wtPtr) const;
    
    /// @brief GetMakeFn return a MakeFn for constructing an instance of
    ///        the Object's managed type.
    /// @param tPtr a pointer to an istance of the Object's managed type
    ///        to wrap in a make fn
    /// @return the MakeFn to enable constructing the passed instance.
    MakeFn GetMakeFn(const TPtr &tPtr) const;
    
    /// @brief GetMakeWeakFn return a MakeFn for constructing a weak instance
    ///        of the Object's managed type.
    /// @param wtPtr a weak pointer to an istance of the Object's managed
    ///        type to wrap in a make fn
    /// @return the MakeFn to enable constructing the passed instance.
    MakeFn GetMakeWeakFn(const WTPtr &wtPtr) const;
    
    /// @brief Check to see if a given stack value is the type managed
    ///        by the Object.
    /// @param L is the Lua State
    /// @param index is the stack index tocheck
    /// @return true if the given index is a type managed by this Object.
    ///         False otherwise.
    /// @note This function will return true for managed values that
    ///       contain a null pointer or strong pointer.
    bool Is(lua_State *L, int index) const;
    
    /// @brief Retrieve a manged value from the given stack index
    /// @brief L the Lua state from which the value is sought
    /// @brief index the index from which the value should be retrievevd
    /// @brief allowNull defines the behavior if the specified index
    ///        does not point to a value of the managed type.
    /// @note If allowNull is false, this function will throw an
    ///       exception if the give value is not the same type as the
    ///       Object's managed value.
    /// @note If allow null is true, this call will return a null pointer
    ///       for any value other than a value that is the same type as
    ///       the type managed by the Object and the value holds a valid
    ///       pointer (rather than being a null pointer value).
    TPtr Get(lua_State *L, int index, bool allowNull) const;
    
    /// @brief GetTableValue is used to retrieve a value from a table.
    /// @param L the Lua State
    /// @param tableIndex is the stack index of the table from which
    ///        a value is sought.
    /// @param key is the field in which the sought value is stored
    /// @param tPtr a reference to the value to populate with the
    ///        value held in the given field
    /// @param allowNull is a flag to indicate what to do if the
    ///        give key does not contain a value that is the same type
    ///        as the Object's managed type or if it does, but that
    ///        value is null.
    /// @note This call will throw an exception if the give value
    ///       is not of the expected type and allowNull is false.
    /// @note This funciton will return a null pointer only if
    ///       allowNull is true and the target value is null, or
    ///       or the given value is not the same type os the Object's
    ///       managed type.
    /// @note This function only retrieves string keys. The string value
    ///       "N" may be retrieved, but the table's index value N should
    ///       be retrieved via the Object::GetTableIndex function.
    void GetTableValue(lua_State         *L,
		       int                tableIndex,
		       const std::string &key,
		       TPtr              &tPtr,
		       bool               allowNull) const;
    
    /// @brief GetTableIndex is used to retrieve a value from a table.
    /// @param L the Lua State
    /// @param tableIndex is the stack index of the table from which
    ///        a value is sought.
    /// @param index is the table index in which the sought value is stored
    /// @param tPtr a reference to the value to populate with the
    ///        value held in the given field
    /// @param allowNull is a flag to indicate what to do if the
    ///        give key does not contain a value that is the same type
    ///        as the Object's managed type or if it does, but that
    ///        value is null.
    /// @note This call will throw an exception if the give value
    ///       is not of the expected type and allowNull is false.
    /// @note This funciton will return a null pointer only if
    ///       allowNull is true and the target value is null, or
    ///       or the given value is not the same type os the Object's
    ///       managed type.
    /// @note This function only retrieves integer indexes. To retrieve
    ///       a value stored in a string key, use the function
    ///       Object::GetTableValue.
    void GetTableIndex(lua_State *L,
		       int        tableIndex,
		       int        index,
		       TPtr      &tPtr,
		       bool       allowNull) const;
    
    /// @brief TableValueIs will check to see if a given table field is
    ///        the same type as the type managed by the Object.
    /// @param L is the Lua state
    /// @param tableIndex is the index of the table
    /// @param key is the field to check
    /// @return true if the given table field holds a type managed by
    ///         by the Object.
    /// @note This function does not indicate whether the given field holds
    ///       a valid value.  If the value is a null pointer to a type managed
    ///       by this Object, the function will return true.
    /// @note To check a numeric index, see Object::TableIndexIs.
    bool TableValueIs (lua_State *L, int tableIndex, const std::string &key) const;

    /// @brief TableIndexIs will check to see if a given table index is
    ///        the same type as the type managed by the Object.
    /// @param L is the Lua state
    /// @param tableIndex is the index of the table
    /// @param index is the index within the table to check
    /// @return true if the given table's index holds a type managed by
    ///         by the Object.
    /// @note This function does not indicate whether the given index holds
    ///       a valid value.  If the value is a null pointer to a type managed
    ///       by this object, the function will return true.
    /// @note To check a field, see Object::TableValueIs.
    bool TableIndexIs (lua_State *L, int tableIndex, int index) const;

    /// @brief Register the Object's MT into a give exec engine.
    /// @param ePtr the Exec to which the MT should be registered
    void Register(const ExecPtr &ePtr) const;


    /// @brief Serialization function for Object objects.
    /// @tparam FT is the type of Object to serialize
    /// @param out is stream to which the object should be written
    /// @param o is the exec to serialize
    /// @return the stream passed as paramter out
    template<typename FT>
    friend std::ostream &operator<<(std::ostream &out, const Object<FT> &o);

    /// @brief destructor
    ~Object();
    
  private:

    /// @brief construtor
    Object(const std::string &name_, MT::Ptr mtPtr_);
    
    std::string name;  ///< object's name
    MT::Ptr     mtPtr; ///< object's MT
  };


  template<typename T>
  typename Object<T>::Ptr Object<T>::Create(const std::string      &name,
					    const FunctionMap::Ptr &fnMap,
					    bool allowDup) {
    Ptr rtn(new Object(name, MT::Create(name, fnMap, allowDup)));
    return rtn;
  }
  template<typename T>
  typename Object<T>::Ptr Object<T>::Create(const std::string      &name,
					    const FunctionMap::Ptr &fnMap,
					    bool allowDup,
					    MT::SerializeFn serializeFn,
					    MT::DeserializeFn deserializeFn) {
    Ptr rtn(new Object(name, MT::Create(name, fnMap, allowDup, serializeFn, deserializeFn)));
    return rtn;
  }
  
  template<typename T>
  const MT::Ptr &Object<T>::GetMT() const { return mtPtr; }
  template<typename T>
  const std::string &Object<T>::Name() const { return name; }
  template<typename T>
  int Object<T>::ToString(lua_State *L) const {
    std::string str = GetString(Get(L,1,true));
    return Values::MakeString(L,str);
  }
  template<typename T>
  std::string Object<T>::GetString(const TPtr &tPtr) const {
    std::stringstream out;
    if (tPtr) {
      out << *tPtr;
    } else {
      out << name << "<released>";
    }
    return out.str();
  }
  template<typename T>
  int Object<T>::Make(lua_State *L, const TPtr &tPtr) const {
    return mtPtr->MakeObject(L,tPtr);
  }
  template<typename T>
  int Object<T>::MakeWeak(lua_State *L, const WTPtr &wtPtr) const {
    return mtPtr->MakeWeakObject(L, wtPtr);
  }
  template<typename T>
  MakeFn Object<T>::GetMakeFn(const TPtr &tPtr) const {
    return [=](lua_State *L) {
      return mtPtr->MakeObject(L, tPtr);
    };
  }
  template<typename T>
  MakeFn Object<T>::GetMakeWeakFn(const WTPtr &wtPtr) const {
    return [=](lua_State *L) {
      return mtPtr->MakeWeakObject(L, wtPtr);
    };
  }
  template<typename T>
  bool Object<T>::Is(lua_State *L, int index) const {
    return mtPtr->Is(L,index);
  }
  template<typename T>
  typename Object<T>::TPtr Object<T>::Get(lua_State *L, int index, bool allowNull) const {
    return std::static_pointer_cast<T>(mtPtr->Get(L, index, allowNull));
  }
  template<typename T>
  void Object<T>::GetTableValue(lua_State *L,
				int tableIndex,
				const std::string &key,
				typename Object<T>::TPtr &tPtr,
				bool allowNull) const {
    StackGuard g(L,1);
    lua_getfield(L, tableIndex, key.c_str());
    tPtr = Get(L, -1, allowNull);
  }
  template<typename T>
  void Object<T>::GetTableIndex(lua_State *L,
				int tableIndex,
				int index,
				typename Object<T>::TPtr &tPtr,
				bool allowNull) const {
    StackGuard g(L,2);
    lua_rawgeti(L, tableIndex, index);
    tPtr = Get(L, -1, allowNull);
  }
  template<typename T>
  bool Object<T>::TableValueIs (lua_State *L, int tableIndex, const std::string &key) const {
    StackGuard g(L,1);
    lua_getfield(L, tableIndex, key.c_str());
    return Is(L,-1);
  }
  template<typename T>
  bool Object<T>::TableIndexIs (lua_State *L, int tableIndex, int index             ) const {
    StackGuard g(L,2);
    lua_rawgeti(L, tableIndex, index);
    return Is(L,-1);
  }
  template<typename T>
  void Object<T>::Register(const ExecPtr &ePtr) const {
    mtPtr->Register(ePtr);
  }
  /// @brief stream output for Object
  /// @param out stream to write the object's information
  /// @param o the object to write to the stream
  /// @return the same stream passed to the function
  template<typename FT>
  std::ostream &operator<<(std::ostream &out, const Object<FT> &o) {
    out << "Object(" << o.name << ")";
    return out;
  }
  template<typename T> Object<T>::~Object() {}
  template<typename T>
  Object<T>::Object(const std::string &name_,
		    MT::Ptr mtPtr_)
    : name(name_),
      mtPtr(mtPtr_) {
  }
    
}

#endif
