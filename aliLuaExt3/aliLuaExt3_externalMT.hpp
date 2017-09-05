#ifndef INCLUDED_ALI_LUA_EXT3_EXTERNAL_MT
#define INCLUDED_ALI_LUA_EXT3_EXTERNAL_MT

#include <aliLuaCore.hpp>
#include <memory>
#include <set>
#include <string>

struct lua_State;
namespace aliLuaExt3 {

  struct ExternalObject;

  /// @brief The ExternalMT is essentially the definition of a Lua Object
  ///        that can be defined outside of an interpreter, but that behaves
  ///        just like any other object within the interpreter.
  ///
  /// Once one defines an ExternalMT, it can be instantiated (potentially
  /// more than once) through the ExternalObect interface.  Once created
  /// The object can be passed between interpreters just like a C++ object.
  ///
  /// The definition of the logic and API for the object comes from a script
  /// that is used when the object is created.   The ExternalObject that
  /// represents an instantiated object holds all the data in a dedicated Lua
  /// interpreter and though one might use that interpreter to hold state,
  /// they might also perfer a stateless processor that allows a pool of
  /// identical objects to act as a worker pool for generically processing
  /// a stream of requests.
  ///
  /// Within a process, these objects are maintained in an internal registry
  /// this allows "finding" an object by its name, then creating it
  /// independently from the code that defines the object.  This does however
  /// mean that ExternamMT names must be unique.
  struct ExternalMT {
    using Ptr  = std::shared_ptr<ExternalMT>;           ///< shared pointer
    using WPtr = std::weak_ptr<ExternalMT>;             ///< weak pointer
    using SSet = std::set<std::string>;                 ///< string set
    using OBJ  = aliLuaCore::StaticObject<ExternalMT>;  ///< StaticObject
    
    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Retrieve the ExternalMT for the given name.
    /// @return A (possiblly null) pointer to the associated ExternalMT.
    static Ptr Get(const std::string &mtName);

    /// @brief Create defines a type of External object (ExternalMT).
    /// @param mtName is the unique name, which can be used to look up
    ///        the ExternalMT.
    /// @param script is the script that will be used to initialize the
    ///        Lua interpreter when an object (ExternalObject) is created.
    /// @param createFn is the name of a function to run when creating
    ///        the object.
    /// @param onDestroyFn is a function that should be run when an
    ///        instantiated object is destroyed.
    /// @param functions is a list of public functions supported by
    ///        the object.  These functions should be defined within
    ///        the script and those script functions will be called
    ///        as a 'Call' method.  The arguments passed to
    ///        the function are exactly what is passed to the call.
    ///        The global environment is the interperter that defines
    ///        the ExternalObject.
    /// @return a pointer to an ExternalObject.
    static Ptr Create(const std::string &mtName,
		      const std::string &script,
		      const std::string &createFn,
		      const std::string &onDestroyFn,
		      const SSet        &functions);

    /// @brief Destructor
    ~ExternalMT();

    /// @brief MTName returns the name of the ExternalMT.
    /// @return The name
    const std::string &MTName() const;

    /// @brief Script returns the script that represents the object's
    ///        logic.
    /// @return the script
    const std::string &Script() const;

    /// @brief CreateFn will return the function designated as the
    ///        create function.
    /// @return a string naming the create function.
    const std::string &CreateFn() const;

    /// @brief Destroy will return the function designated as the
    ///        destroy function.
    /// @return a string naming the destroy function.
    const std::string &OnDestroyFn() const;

    /// @brief Functions will return a list of functions defined
    ///        by the script that should be considered public.
    /// @return a set of function names
    const SSet        &Functions() const;

    /// @brief IsDefined returns a flag indicating whether the
    ///        passed name is defined by the function.
    /// @return true/false indicating whether or not the function
    ///         is defined by the given ExternalMT.
    bool               IsDefined(const std::string &fnName) const;

    /// @brief GetInfo will retrieve information about the ExternalMT
    ///        object.
    /// @return a MakeFn of information.
    aliLuaCore::MakeFn GetInfo();

  private:

    /// @brief constructor
    ExternalMT();

    std::string                 mtName;       ///< name
    std::string                 script;       ///< Lua script
    std::string                 createFn;     ///< createFn function name
    std::string                 onDestroyFn;  ///< destroy function name
    SSet                        functions;    ///< set of functions
  };

}

#endif

