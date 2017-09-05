#ifndef INCLUDED_ALI_SYSTEM_UTIL
#define INCLUDED_ALI_SYSTEM_UTIL

#include <functional>
#include <string>

namespace aliSystem {
  namespace Util {

    /// \brief DoNothing is an empty function used to provide a no-op function.
    void     DoNothing();

    /// \brief NotifyFn is merely a utility type that can be used to
    /// articulate single argument handlers.
    /// The notify type is left unadorned so that a user may define
    /// it as a pointer, reference, const or non-constant.
    /// @note One should consider the life cycle of a notification functoin and
    ///       any object contained by the notification function.  In many
    ///       cases, one might prefer to maintain weak pointers within the
    ///       function object to avoid circular references that will prevent
    ///       proper object destruction.
    template<typename NotifyType>
    using NotifyFn = std::function<void(NotifyType)>;

    /// \brief The SelectFn type is a convenience type that is intended
    /// to be used as a funciton argument for filtering the function's
    /// processing.
    template<typename InfoType>
    using SelectFn = std::function<bool(const InfoType&)>;

    /// \brief MatchAll always returns true
    /// \param comp is ignored
    /// \returns true
    template<typename InfoType>
    bool MatchAll(const InfoType &) {
      return true;
    }

    /// \brief The StrSelectFn is a specialization of the SelectFn
    /// intended for string related comparisons.
    using StrSelectFn = SelectFn<std::string>;
    
    /// \brief GetMatchShortestFn returns a select function that will only match
    /// values that match the shortest comp/value length.
    /// \param comp is a string to match with values passed to the select Fn
    StrSelectFn GetMatchShortestFn(const std::string &comp);
    
    /// \brief GetMatchExactFn returns a select function that will only match the
    /// exact string passed the select function.
    /// \param comp - string to match in the returned select function.
    StrSelectFn GetMatchExactFn(const std::string &comp);

    /// \brief GetMatchPrefixFn returns a select function that will match any
    /// string beginning with the given prefix.
    /// \param prefix - prefix to match in the returned select function.
    StrSelectFn GetMatchPrefixFn(const std::string &prefix);

    /// \brief CommitFn names a type that is intended to be used as a boolean evaluation
    /// function.
    using CommitFn = std::function<bool()>;

    /// \brief Always is a trivail commit function that always returns true.
    bool     Always();

    /// \brief Never is a trival commit function that always returns false.
    bool     Never();
    
  }
}

#endif
