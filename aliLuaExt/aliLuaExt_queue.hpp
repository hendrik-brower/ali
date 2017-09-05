#ifndef INCLUDED_ALI_LUA_EXT_QUEUE
#define INCLUDED_ALI_LUA_EXT_QUEUE

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <ostream>

struct lua_State;
namespace aliLuaExt {

  /// @brief The Queue class defines a queue of Lua MakeFn values that
  ///        can be used to asynronously queue data for a Lua interpreter
  ///        or a set of Lua interpreters to process.
  ///
  /// Since Lua is fundamentally a single threaded API, so using an
  /// interpreter to execute work is often challenged by the desire
  /// to allow the c++ code that would generally call the Lua to run
  /// without being blocked.  In order to separate this interaction,
  /// this class provides a mechanism for c++ code to queue work.
  /// One could use a aliLuaCore::Future to retrieve processing results
  /// alternatively one might have their own custom call back mechanism.
  /// In general, when Lua calls to C++, it is often quite easy to enable
  /// synchronize those calls with other C++ threads.  In cases where
  /// the is no need for a call back, interactions between C++ and Lua are
  /// simply that much simpler.  If data needs to be queued to C++, and
  /// the call from Lua will not directly syncronize with the C++ data sink
  /// one might extract a aliLuaCore::MakeFn from the Lua interpreter in
  /// the call back.  Then in a "decoupling" Lua interpreter, that data can
  /// be re-constructed and read directly from the C++ code needing the
  /// data.
  struct Queue {
    using Ptr       = std::shared_ptr<Queue>;             ///< shared pointer
    using WPtr      = std::weak_ptr<Queue>;               ///< weak pointer
    using MVec      = std::deque<aliLuaCore::MakeFn>;     ///< MakeFn queue
    using Listeners = aliSystem::Listeners<const WPtr &>; ///< listener container
    using Listener  = aliSystem::Listener<const WPtr &>;  ///< listener
    using OBJ       = aliLuaCore::StaticObject<Queue>;    ///< utility StaticObject
    using LOBJ      = aliLuaCore::StaticObject<Listener>; ///< utility StaticObject
    
    /// @brief Initialize Queue module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Create a queue.
    /// @param name the name of the queue
    /// @return A queueu
    static Ptr Create(const std::string &name);

    /// @brief Destructor.
    ~Queue();

    /// @brief Name retrieves the name of the queue
    /// @return the name
    const std::string &Name() const;

    /// @brief GetPtr retrieves a shared pointer to the queue
    /// @return "self" pointer
    Ptr GetPtr() const;

    /// @brief PushNext will extract an item from the queue and push
    ///        it onto the given interpreter's stack.
    /// @param L the interpreter to receive the queue item
    /// @return the number of items pushed onto the stack
    /// @note If there are no items in the queue, nothing will be
    ///       pushed.
    int PushNext(lua_State *L);

    /// @brief Append will add the given item to the end of the queue.
    /// @param item MakeFn representing the "item to add"
    /// @note The 'item' may represent more than one stack value.
    ///       One might consider pushing tables into a queue, which
    ///       makes it easy to consistently received the items from a
    ///       queue.  One's strategy in this regard is arbitrary.
    void Append(const aliLuaCore::MakeFn &item);

    /// @brief Append will add the given item to the end of the queue.
    /// @param item MakeFn representing the "item to add"
    /// @note The 'item' may represent more than one stack value.
    ///       One might consider pushing tables into a queue, which
    ///       makes it easy to consistently received the items from a
    ///       queue.  One's strategy in this regard is arbitrary.
    void Insert(const aliLuaCore::MakeFn &item);

    /// @brief Empty checks to see if the queue is empty.
    /// @brief empty status
    bool Empty();

    /// @brief Size returns the number of items in the queue, which
    ///        may be zero.
    /// @return number of items in the queue.
    size_t Size();

    /// @brief Onfirst will return a listering container that will
    ///        be notified when the first item is entered into the
    ///        queue whenever it was empty before the addition.
    /// @return listener container.
    const Listeners::Ptr &OnFirst () const;

    /// @brief OnInsert will return a listering container that will
    ///        be notified whenever an item is added to the
    ///        queue.
    /// @return listener container.
    const Listeners::Ptr &OnInsert() const;

    /// @brief serialization function for a Queue.
    /// @param out is the stream to which the queue should be serialized
    /// @param o is the queue object to serialize
    /// @return the stream passed as out
    friend std::ostream &operator<<(std::ostream &out, const Queue &o);

  private:

    /// @brief Constructor
    Queue();

    /// @brief Notify is an internal function that is used to trigger
    ///        listeners whenever items are added to the queue.
    /// @param sz should be the size of the queue before the item
    ///        trigger the call was added.
    /// @note This call is made within the queue's lock.  Because
    ///       of this listeners should not makes calls to the queue
    ///       that would trigger a lock.
    void Notify(size_t sz);
    
    WPtr           THIS;        ///< weak pointer to self
    std::mutex     lock;        ///< lock to guard the manipulation of items
    std::string    name;        ///< name of the queue
    MVec           items;       ///< items in the queue
    Listeners::Ptr onFirst;     ///< listener container
    Listeners::Ptr onInsert;    ///< listener container
  };

}

#endif
