#ifndef INCLUDED_ALI_LUA_CORE_MT
#define INCLUDED_ALI_LUA_CORE_MT

#include <aliLuaCore_functions.hpp>
#include <aliSystem.hpp>
#include <memory>
#include <mutex>
#include <vector>
#include <lua.hpp>

struct lua_State;
namespace aliLuaCore {

  // FIXME: add MT::IsWeak(L,index)/MT::IsStrong(L,index).
  //        example use: aliLuaCore::Queue's "CreateListener"
  

  struct Exec;
  /// @brief MT is short for metatable.  The MT class defines an interface between
  ///        C++ code and Lua that helps leverage Lua's metatable based objects.
  ///
  /// The MT class allows defining c++ types and a Lua interface for those types
  /// that can be pushed into a Lua interpreter and then used within Lua like native
  /// Lua objects.  Objects defined by the MT are held as shared pointers within Lua.
  /// Within Lua, these objects can be passed around like any other object.  From C++,
  /// the associated functions are generally used to define the functionality that
  /// can be accessed from Lua.
  ///
  /// When defining MT objects, one can specify whether the objects can be transfered
  /// between Lua interpreters and/or serialized and deserialzied to an external
  /// representation.
  ///
  /// Duplication is the term used to refer to the ability to transfer the object
  /// from one interpreter to another.  What this really means is that for an
  /// object for which duplication is supported, a MakeFn can be created using
  /// MT class methods along with an interpreter's state & the index.  This allows
  /// generic modules like aliLuaCore::Tables or aliLuaCore::Values to allow transfering
  /// objects to a container that can then be applied to another interpreter to
  /// reconstitute the object.  When wrapped in this MakeFn, if the object in
  /// the Lua stack was a strong pointer, it will be held in the MakeFn as a
  /// strong pointer.  If the Lua stack item held a weak pointer, the MakeFn will
  /// also hold a weak pointer.
  /// 
  /// When re-applying a MakeFn that holds a MT object, that operation will
  /// only succeed if the MT's metatable has been defined within the target
  /// interpreter.  In general, when defining a MT object, one would alos register
  /// a aliLuaCore::Module handler that would register the MT to any new interpreters.
  /// Doing this before any general container is constructed ensures that all
  /// interperters defined by the aliLuaCore::Exec will support the use of the given MT.
  /// Alternatively, one could choose not to automated MT registration through
  /// a aliLuaCore::Module function, but instead manually register the MT.  Though this
  /// this may allow more efficient interpreter startup, it will tend to be more
  /// error prone.
  ///
  /// Serialization is the term used for extrating a representation of an object
  /// to a stream which could then be deserialized back into an interpreter.  Unlike
  /// duplication, which passes around a shared pointer that refers to the exact same
  /// object instannce, serialization essentially copies pertinent information
  /// so that a new object with the same state can be constructed from the previously
  /// serialized information.  These API can be used to extract an object, send it to
  /// another process or machine, potentially by way of a database or file storage or
  /// series of network packets where it can be reconstructed.  Obviously in this
  /// manner, memory based links (ie shared/weak/raw pointers to the original object)
  /// will not point to the new construted object.  However, this API allows one to
  /// easily define the process by which information is passed as a copy of the
  /// original.  With serialization, there is no requirement that the deserialization
  /// occurs in a different interpreter.  The process of serialization and
  /// deserialization can be done in the same interprerter from which the object
  /// was originally constructed.   However, in this case Dupliation or merely
  /// leveraging Lua's reference mechanisms are more efficient.  However, for both of
  /// these, access is to a reference, where as serialization/deserialization defines
  /// the access as a copy to a new object.
  ///
  /// When defining serializable objets, the client code needs to ensure that it can
  /// prperly handle versioning of the serialized representation to avoid deserialization
  /// errors.
  ///
  /// Objects pushed into Lua using a MT, can be constructed as weak or strong
  /// pointers.  In the case of weak, Lua will hold a std::weak_ptr.  In the case
  /// of a strong, Lua will hold a std::shared_ptr.  The object's GC routine
  /// will be called when all references to an object are lost.  However, if the
  /// object is used as a weak key or value in a table that is walked over explicitly,
  /// then it is possible the routine walking over the table entires will have access
  /// to the already GC'd object.  In Lua this is referred to as resurection.
  /// If that object's C++ routines are called when walking over the record,
  /// any called C++ function will receive a null pointer and should behave accordingly.
  /// It is best to avoid producing Lua errors within a GC function.
  ///
  /// MT objects have a set of pre-defined fucntions.  It also will wrap some functinos.
  /// The __gc is wrapped so that the MT object can handle memory allocation and
  /// deallocation with the nuances resulting from resurection.  The __tostring
  /// is wrapped to allow returning a descriptive string even if the object has been
  /// released.   Other function that are added include a Release, IsValid, ToWeak, ToStrong,
  /// MTName.  IsValid will return true if the object's GC or release method has been
  /// called and false otherwise.  Release will reset the object's shared pointer.
  /// ToWeak will return a weak reference for a strong object (or another weak reference
  /// if the object is already a weak pointer).  ToStrong will return a strong pointer
  /// based object.  If done with a weak pointer, the strong pointer will only be
  /// valid if the weak pointer refers to an object that has at least one strong pointer
  /// refering to it.
  ///
  /// In general, when dealinng with a Lua interpreter that holds some C++ object, there
  /// will be some casting since the interpreter holds a void pointer.  Any illegal casting
  /// will result in undefined behavior (hopefully a seg fault).  Using helper API's to
  /// avoid manual casting is the most robust way to avoid these errors.  The MT class
  /// pushes the onous on the client code.  However, the aliLuaCore::Object and 
  /// aliLuaCore::StaticObject interfaces properly wrap this handling and these API's should 
  /// be used as much as possible rather than using MT's Get and Make* directly.
  ///
  struct MT {
    using Ptr           = std::shared_ptr<MT>; ///< instance pointer
    using WPtr          = std::weak_ptr<MT>;   ///< weak instance pointer
    using TPtr          = std::shared_ptr<void>; ///< type the MT object represents    
    using WTPtr         = std::weak_ptr<void>;   ///< weak type that the MT represents
    using SObj          = aliSystem::Codec::Serializer; ///< serializer
    using DObj          = aliSystem::Codec::Deserializer; ///< deserializer
    using ExecPtr       = std::shared_ptr<Exec>; ///< an execution object

    /// @brief DupFn is a function that can extract a make function that can later
    ///        be used to push the object into an interpreter
    using DupFn         = std::function<MakeFn(lua_State*, int)>;

    /// @brief SerializationFn defines an interface that allows translating
    ///        an obect that a meta table refers to an externalized representation.
    using SerializeFn   = std::function<void(lua_State*, int, SObj&)>;

    /// @brief DeserializationFn defines an interface that allows constructing
    ///        an object that the metatable refers to into a Lua interperter
    ///        from an externalized representation.
    using DeserializeFn = std::function<void(lua_State*, DObj &)>;

    using WPVec         = std::vector<WPtr>; ///< vector of weak MT pointers
    
    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief Create provides a mechanism for defining a MT.
    /// @param name is the name of the MT.  This much be globally unique.
    /// @param fnMap is a set of functions that will be defined for the
    ///        object.
    /// @param allowDup is a flag that if true, allows duplication for the
    ///        given MT object values.
    static Ptr Create(const std::string      &name,
		      const FunctionMap::Ptr &fnMap,
		      bool                    allowDup);

    /// @brief Create provides a mechanism for defining a MT.
    /// @param name is the name of the MT.  This much be globally unique.
    /// @param fnMap is a set of functions that will be defined for the
    ///        object.
    /// @param allowDup is a flag that if true, allows duplication for the
    ///        given MT object values.
    /// @param serializeFn defines a method that can be used to serialize
    ///        a representation of the object.
    /// @param deserializeFn defines a method that can be used to deserialize
    ///        a perviously serialized object.
    static Ptr Create(const std::string      &name,
		      const FunctionMap::Ptr &fnMap,
		      bool                    allowDup,
		      SerializeFn             serializeFn,
		      DeserializeFn           deserializeFn);

    /// @brief destructor
    ~MT();

    /// @brief return the name of the MT.
    /// @return a reference to the MT's name.
    const std::string &Name() const;

    /// @brief GetMT allows extracting the MT pointer for an object
    ///        in a Lua interpreter.
    /// @param L is the Lua State
    /// @param index is the stack offset for the item to which the MT is
    ///        sought.
    /// @param ignoreNonMT allows specifying the behavior if the
    ///        specified index does is not an object with a metatable.
    /// @return A MT::Ptr, which might be a null pointer if ignoreNonMT
    ///         is true.
    /// @note the function will throw if ignoreNonMT is false and
    ///       the specified index has no metatable.
    /// @note The function will throw if the index for the MT object
    ///       is not a user data value.
    /// @note This function casts the MT object field in to a MT::Ptr
    ///       and care should be made to avoid non-MT objects from
    ///       setting this field to a user data value.
    static Ptr GetMT(lua_State *L, int index, bool ignoreNonMT);

    /// @brief GetMT returns a MT::Ptr for the specified name.
    /// @param name of MT to retrieve.
    /// @return A MT::Ptr for the given name (names must be globally unique).
    static Ptr GetMT(const std::string &name);

    /// @brief Is will determine if the noted stack index is an object
    ///        of the MT's type.
    /// @param L Lua state
    /// @param index is the stack index to check
    /// @note This function will also return true if the specified index
    ///       is a registered drived type.
    bool Is (lua_State *L, int index);

    /// @brief Get will retrieve an object pointer if for the item
    ///        at the specified stack index.
    /// @param L is the Lua State from which the object should be retrieved.
    /// @param index is the stack index from which the object should be
    ///        retrieved.
    /// @param allowNull allows ignoring errors in fetching an object
    ///        in case: 1) the stack index is LUA_TNIL, 2) the stack index
    ///        is LUA_TNONE, or 3) stack index holds a released poitner
    ///        that is of the MT's type or a registered derived type of
    ///        the MT's type.
    /// @return A pointer to the object.
    /// @note If the target index is a weak pointer of a valid type, it
    ///       will be converted to a strong pointer (if it is still valid).
    /// @note The return type is a void pointer.  In general users should
    ///       call this function inderecty through a aliLuaCore::StaticObject
    ///       or aliLuaCore::Object, which are templated types.  Users of those
    ///       API's are much less likely to experience programatic errors
    ///       related to conversions.
    TPtr Get(lua_State *L, int index, bool allowNull=false);

    /// @brief MakeObject will construct an object and push it onto the Lua
    ///        stack.
    /// @param L the Lua State.
    /// @param tPtr the object to push on the Lua stack
    /// @return This function should always return 1.
    /// @note In general users should call this function inderecty through
    ///       aaliLuaCore::StaticObject::Make or aliLuaCore::Object::Make to take
    ///       advantage of those classes type safety.
    int MakeObject(lua_State *L, const TPtr &tPtr);

    /// @brief MakeWeakObject will construct a weak object and push it onto
    ///        the Lua stack.
    /// @param L the Lua State.
    /// @param wtPtr the object to push on the Lua stack
    /// @return This function should always return 1.
    /// @note In general users should call this function inderecty through
    ///       aaliLuaCore::StaticObject::Make or aliLuaCore::Object::Make to take
    ///       advantage of those classes type safety.
    int MakeWeakObject(lua_State *L, const WTPtr &wtPtr);

    /// @brief ToWeak will convert the given stack value to a weak pointer.
    ///        The converted value will be pushed onto the stack.
    /// @param L the Lua state
    /// @param index is the stack index to convert.
    /// @return This function will always return 1.
    /// @note This function will throw if the specified index cannot be converted
    ///       to an object of this MT's type.
    /// @note The created object may be null if the targeted index contains
    ///       a released strong or weak pointer.
    /// @note The pushed value will share ownership (weakly) with the original,
    ///       this function does *NOT* make an independnt copy of the object.
    /// @note This function will throw an exception if the specified value's
    ///       type is not compatible with the instance of the MT.
    int ToWeak  (lua_State *L, int index);

    /// @brief ToStrong will convert the given stack value to a strong pointer.
    ///        The converted value will be pushed onto the stack.
    /// @param L the Lua state
    /// @param index is the stack index to convert.
    /// @return This function will always return 1.
    /// @note This function will throw if the specified index cannot be converted
    ///       to an object of this MT's type.
    /// @note The created object may be null if the targeted index contains
    ///       a released strong or weak pointer.
    /// @note The pushed value will share ownership of the original, this function
    ///       does *NOT* make an independnt copy of the object.
    /// @note This function will throw an exception if the specified value's
    ///       type is not compatible with the instance of the MT.
    int ToStrong(lua_State *L, int index);

    /// @brief Dup will create a make function that encapsulates the value
    ///        at the given index.
    ///
    ///        If the value is a weak pointer the created MakeFn will wrap a
    ///        weak pointer.  If the object is a strong the MakeFn will wrap
    ///        a strong pointer.
    /// @param L Lua State.
    /// @param index is the index to extract
    /// @return A MakeFn.  The erturned MakeFn will always return 1 when
    ///         executed.
    /// @note The function will throw an exception if the given index is not
    ///       a MT object.   It will also throw if the object's MT does not
    ///       support duplication.
    static MakeFn Dup(lua_State *L, int index);

    /// @brief Serialize will call the serialization function for a MT
    ///        object at the given index.
    /// @param L the Lua State.
    /// @param index within the Lua stack to serialize.
    /// @param sObj is the serializer to use
    /// @note This function will throw an exception if the type of MT object
    ///       does not support serialization.
    static void   Serialize(lua_State *L,
			    int        index,
			    SObj      &sObj);

    /// @brief Deserialize will call the deserialization function for input
    ///        stream and if the type encoded in the is a recognized MT that
    ///        supports deserialization, the object will be pushed onto
    ///        the stack of the given interpreter.
    /// @param L the Lua State.
    /// @param dObj is the deserializer to use
    /// @note This function will throw an exception if the type of MT object
    ///       does not support deserialization.
    static void Deserialize(lua_State *L,
			    DObj      &dObj);

    /// @brief Register will register an MT in the passed Exec.
    /// @param ePtr the Exec to which the type should be registered.
    /// @note In general, one should register a call to this with a registered
    ///       aliLuaCore::Module function to ensure one's types are generally
    ///       available for use.  One could also use the packet search API's
    ///       to dynamically load types based on calls to require.
    void Register(const ExecPtr &ePtr);

    /// @brief Check to see if the given type supports duplication.
    /// @return true if the type can support duplication, false if not.
    bool CanDup() const;

    /// @brief Check to see if the given type supports serialization.
    /// @return true if the type can support serialization, false if not.
    bool CanSerialize() const;

    /// @brief Check to see if the specified function exists in the given
    ///        object type's metatable.
    /// @param fnName name of function to check
    /// @return true if the funtion is defined, false otherwise.
    bool IsDefined(const std::string &fnName);

    /// @brief Register a derived type for the MT object.
    /// @param mtPtr the MT for the derived type.
    /// @note The type managed by the derived class should truely be
    ///       a derived class with polomorphic inheritence.
    /// @note A common use for this is to define secondary API's for a given
    ///       object type.   One can define a "base type" which supports
    ///       read only APIs and a drived type that supports read/write API's.
    ///       In c++, the system can convert, but in Lua, the use of the objects
    ///       will be restricted as expected.
    void AddDerived(const Ptr &mtPtr);
    
  private:
    /// @brief SetMT will set the metatable for the item on the top of the stack.
    /// @param L Lua state
    /// @note This is a highly unsafe function.  In general users should not call
    ///       the function and it should remain a private API.
    void SetMT(lua_State *L) const;

    /// @brief DefineReserved will add & wrap the given FuntionMap's function with
    ///        common MT functions.
    /// @note At this time, the wrapped functions are: __gc, __tostring.
    /// @note At this time, the added funtions  are MTName, Release, ToStrong,
    ///       ToWeak, IsValid.
    /// @note When creating an MT object, if the passed FunctionMap defines
    ///       functions that will be added, this function will throw an exception
    ///       and one will be unable to create the MT object.
    static void DefineReserved(const Ptr &ptr);

    /// @brief constructor
    MT();

    WPtr             THIS;    ///< weak pointer to self
    std::string      name;    ///< name of object
    FunctionMap::Ptr fnMap;   ///< functions supported by the object
    DupFn            dupFn;   ///< dupFn, if no dup is allow, then this will throw
    SerializeFn      serializeFn;    ///< serialize function
    DeserializeFn    deserializeFn;  ///< deserialize function
    bool             canDup;         ///< canDup flag
    bool             canSerialize;   ///< can serialize flag
    std::mutex       derivedLock;    ///< class lock protecting edits to the derived vector
    WPVec            derivedVec;     ///< derived objects
  };

}

#endif

