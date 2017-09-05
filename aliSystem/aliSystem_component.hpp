#ifndef INCLUDED_COMPONENT
#define INCLUDED_COMPONENT

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <string>

namespace aliSystem {

  /// @brief A Component is an element of the system that defines an
  ///        initialization and/or finalization routine.
  ///
  /// Components are registered to a CompoentRegistry, which manages
  /// the execution of their initializtion and finalization routines.
  ///
  /// Use of the ComponentRegistry is not required, though it is the
  /// most common approach.
  ///
  /// Once a component is created, the other component that it depends
  /// on may be added, which can be used to determine an appropriate
  /// or of initialization/finalization for a collection of components.
  ///
  /// The component's name is the key that other components should
  /// use to declare a dependency.  Dependences are defined as a set
  /// of strings rather than other Component pointers or references
  /// so that they can be defined in any order and only when an
  /// initialization routine runs does it need to have valid references
  /// to the collection of components.
  ///
  /// Components expose a method of avoiding explicit initialization
  /// and finalization (shutdown) logic, instead allowing C++ modules
  /// that require initialization to register themselves as well as
  /// other compoenents that they depend upon to allow a fixed algorithm
  /// to generically order and execute the initialization and  shutdown
  /// logic.
  /// 
  struct Component {
    using Ptr  = std::shared_ptr<Component>;      ///< shared pointer
    using WPtr = std::weak_ptr<Component>;        ///< weak pointer
    using Fn   = std::function<void()>;           ///< init/fini signature
    using SSet = std::unordered_set<std::string>; ///< string set

    /// @brief Create a Component.
    /// @param name is a name for the component.  This
    ///        name must be unique within a ComponentRegistry
    ///        if the component is registered.
    /// @param initFn is a function that defines some initialization
    ///        logic.
    /// @param finiFn is a function that defines some finalization
    ///        logic.
    /// @return A shared pointer to the created Component.
    static Ptr Create(const std::string &name,
		      const Fn          &initFn,
		      const Fn          &finiFn);

    /// @brief destructor.
    ~Component();

    /// @brief Return the name of the Component
    /// @return name of Component
    const std::string &Name() const;
    
    /// @brief Return the init function.
    /// @return function
    /// @note The returned function will assert if
    ///       it is run more than once.
    const Fn          &GetInitFn() const;

    /// @brief Return the fini function.
    /// @return function
    /// @note The returned function will assert if
    ///       it is run more than once.
    const Fn          &GetFiniFn() const;

    /// @brief Return a flag indicating the state of the Component.
    /// @return a flag indicating whether the Component is
    ///         frozen.
    bool               IsFrozen() const;

    /// @brief Return a flag indicating the state of the Component.
    /// @return a flag indicating whether the Component's init
    ///         function has been run.
    bool               WasInit() const;

    /// @brief Return a flag indicating the state of the Component.
    /// @return a flag indicating whether the Component's fini
    ///         function has been run.
    bool               WasFini() const;
    
    /// @brief Return the number of dependencies defined for
    ///        the Component.
    /// @return Number of dependencies.
    size_t             NumDep();

    /// @brief Retrieve the Components on which this Component
    ///        depends.
    /// @return a set of Component names.
    /// @note This function will throw an exception if the
    ///       Component is not frozen.
    const SSet &GetDependencies() const;

    /// @brief Retrieve the Components on which this Component
    ///        depends.
    /// @param sSet is a string set to which the Component names
    ///        should be written.
    void GetDependencies(SSet &sSet);

    /// @brief Freeze a component.
    /// @note A ComponentRegistry will freeze components before
    ///       beginning ordering components for their dependencies.
    void Freeze();

    /// @brief Add a reference to a Compnent to which this component
    ///        depends.
    /// @param str is the name of another component.
    /// @note This function will throw an exception if the Component
    ///       is frozen.
    /// @note This function will throw an excepton if the passed str
    ///       is the same as the Component's name.
    void AddDependency(const std::string &str);

    /// @brief Serialize a Component
    /// @param out is the stream to which the component should be serialized
    /// @param o is the Component to serialize
    /// @return the passed stream
    friend std::ostream &operator<<(std::ostream &out, const Component &o);
    
  private:

    /// @brief constructor
    Component();

    std::mutex  lock;      ///< mutex for serializing updates
    std::string name;      ///< name of the component
    Fn          initFn;    ///< component's init function
    Fn          finiFn;    ///< component's fini function
    bool        isFrozen;  ///< frozen flag
    bool        wasInit;   ///< init flag
    bool        wasFini;   ///< fini flag
    SSet        depSet;    ///< set of strings refering to other components
  };

}

#endif
