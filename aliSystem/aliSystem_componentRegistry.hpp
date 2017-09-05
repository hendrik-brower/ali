#ifndef INCLUDED_COMPONENT_REGISTRY
#define INCLUDED_COMPONENT_REGISTRY

#include <aliSystem_component.hpp>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace aliSystem {

  /// @brief A component registry defines a container of Components.
  ///
  /// The ComponentRegistry exposes an init/fini function that will
  /// triger the execution fo the init/fini functions for each
  /// registered Component.  The execution of the init functions
  /// will be ordered such that no Component is executed before
  /// its named dependencies.  If it is not possible to order the
  /// Components to satisfy this condition, a call to Init will
  /// throw an exception.
  ///
  /// When calling Fini, registered Components will have their
  /// fini funciton called.  The order of these calls will be the
  /// exact opposite of the order determined for the init sequence.
  struct ComponentRegistry {
    using Vec  = std::vector<Component::Ptr>;     ///< Compnent vector
    using SSet = std::unordered_set<std::string>; ///< string set

    /// @brief constructor
    ComponentRegistry();
    
    /// @brief Copy constructor is deleted
    ComponentRegistry(const ComponentRegistry &) = delete;
    
    /// @brief Assignment operator is deleted
    ComponentRegistry &operator=(const ComponentRegistry &) = delete;

    /// @brief destructor
    ~ComponentRegistry();

    /// @brief Register a Component with the given name with empty
    ///        init and fini functions.
    /// @return Component pointer
    /// @note This is useful for defining higher level "markers"
    ///       for an initialization sequence.  For example, if a library
    ///       defines 50 components, and other Compoennts might want to
    ///       simply name the collection, the library can define an
    ///       abstract component that represents this collection.
    ///       This definition can be done inside or outside of a library.
    ///       However, defining a Component like this within the library
    ///       eables the library definers to ensure that the dependency
    ///       list stays up to date for users of the library.
    /// @note The component registry will retain a reference to the
    ///       returned pointer and it is not necessary for the caller
    ///       to maintain this pointer.  The pointer however can be used
    ///       to manipulate the Component constructed by this call.
    /// @note This function will throw an exception if the specified
    ///       name matches the name of a Component that is already
    ///       registered.
    Component::Ptr Register(const std::string   &name);

    /// @brief Create and Register a Component with the given name, init
    ///        function and fini function.
    /// @param name is the name of the component to define
    /// @param initFn is the init function to use for the newly constructed
    ///        component.
    /// @param finiFn is the fini function to use for the newly constructed
    ///        component.
    /// @note The component registry will retain a reference to the
    ///       returned pointer and it is not necessary for the caller
    ///       to maintain this pointer.  The pointer however can be used
    ///       to manipulate the Component constructed by this call.
    /// @note This function will throw an exception if the specified
    ///       name matches the name of a Component that is already
    ///       registered.
    Component::Ptr Register(const std::string   &name,
			    const Component::Fn &initFn,
			    const Component::Fn &finiFn);

    /// @brief Register the given component.
    /// @return the returned pointer is the same as the passed pointer.
    /// @note This function will throw an exception if the specified
    ///       name matches the name of a Component that is already
    ///       registered.
    /// @note This function will also throw an exception if the passed
    ///       ptr is uninitialized.
    Component::Ptr Register(const Component::Ptr &ptr);

    /// @brief Return a flag indicating the state of the ComponentRegistry
    /// @return a flag indicating whether or not the init sequence has
    ///         be executed.
    bool HasInit() const;

    /// @brief Return a flag indicating the state of the ComponentRegistry
    /// @return a flag indicating whether or not the fini sequence has
    ///         be executed.
    bool HasFini() const;
    
    /// @brief Get a list of the components registered to the given
    ///        ComponentRegistry
    /// @param vec is the vector to write the Components into.
    /// @note vec is not cleared before the components are added
    void GetComponents(Vec &vec);
    
    /// @brief Trigger the execution of the init functions for
    ///        The registered Components.
    /// @note This function will first sort the Components so that
    ///       each component's dependencies will be initialized before
    ///       the component is initialized.   If this order cannot
    ///       be achieved, this function will throw an exception.
    /// @note This function will attempt to define a consistent ordering
    ///       of functions so that initialization tends to be consistent
    ///       from execution to execution.  This might allow one to
    ///       consistently initialize Components even if dependencies
    ///       are not accurately defined; however, it tend to make
    ///       issues related to this repeatable.  To do this, compoents
    ///       are generally sorted by their dependencies and then their
    ///       name.  This stable ordering is not guarenteed and might
    ///       evolve as this class evolves.
    void Init();
    
    /// @brief Trigger the execution of the fini functions for
    ///        The registered Components.
    void Fini();

    /// @brief GetSrcSet will return the current set of registered module
    ///        names.
    /// @param sSet the set to which the Component names should be
    ///        insertered.
    /// @note sSet is not cleared before Component names are inserted.
    /// @note This function will return the current set of Components,
    ///       which may change until the Init funciton is called.
    void GetSrcSet(SSet &sSet);

    /// @brief GetDepSet will return the current set of dependencies
    ///        for all registered Components.
    /// @param sSet the set to which the Component dependencies should be
    ///        insertered.
    /// @note sSet is not cleared before Component names are inserted.
    /// @note This function will return the current set of dependencies,
    ///       which may change until the Init funciton is called.
    void GetDepSet(SSet &sSet);
  
  private:
    
    std::mutex lock;        ///< lock to serialize access to other members
    bool       hasInit;     ///< flag indicating state of the object
    bool       hasFini;     ///< flag indicating state of the object
    Vec        components;  ///< vector of registered components
    SSet       srcSet;      ///< string set of component names
  };  

}

#endif
