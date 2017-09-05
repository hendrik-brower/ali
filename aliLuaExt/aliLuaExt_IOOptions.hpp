#ifndef INCLUDED_ALI_EXT_IO_OPTIONS
#define INCLUDED_ALI_EXT_IO_OPTIONS

#include <aliSystem.hpp>
#include <limits>
#include <iostream>
#include <string>


namespace aliLuaExt {

  /// @brief IOOptions defines a set of options to use for the
  ///        IO structure when logging Lua base values.
  /// @note Some possible extensions might include:
  ///       TableSeparator, TableStart, TableEnd
  struct IOOptions {
    using CStr = const std::string;                 ///< const std::string
    using SPtr = aliSystem::Codec::Serialize::Ptr;  ///< serialize ptr


    /// @brief constructor
    /// @param index or starting to process
    /// @param count is the number of indices to process, default to [index,top]
    explicit IOOptions(int    index = 1,
		       size_t count = std::numeric_limits<size_t>::max());
    //
    // accessors
    
    /// @brief GetIndex returns the defined instance
    /// @return the index
    int     GetIndex           () const;

    /// @brief GetCount returns the number of items to
    ///        process.
    /// @return count
    /// @note Processing will terminate by the count or actual
    ///       stack items, whichever comes first, so this value
    ///       does not necessarily reflect how many items will be
    ///       processed.
    size_t  GetCount           () const;
    
    /// @brief GetIndentSize returns the number of spaces to indent output
    /// @return Indent size
    size_t  GetIndentSize      () const;
    
    /// @brief GetSeparator returns the separator to use between
    ///        sequential values.
    /// @return separator
    CStr   &GetSeparator       () const;
    
    /// @brief GetRootName returns the base name to use for repeated
    ///        table references.
    /// @return the root name
    CStr   &GetRootName        () const;
    
    /// @brief GetEnableNewLines returns a flag indicating whether
    ///        or not output should include new lines
    /// @return new line flag
    bool    GetEnableNewLines  () const;
    
    /// @brief GetShowTableAddress returns a flag indicating whether
    ///        or not tables should be preceded by their address
    /// @return show table address flag
    bool    GetShowTableAddress() const;
    
    /// @brief GetSerialize returns a flag indicating whether or not output
    ///        should be serialized in a format that can later be deserialized.
    /// @return serialize flag
    SPtr GetSerialize() const;

    //
    // manipulators

    /// @brief SetIndex allows reassigning the index.
    /// @param val_ the new index
    void SetIndex           (int     val_);
    
    /// @brief SetCount allows reassinging the count
    /// @param val_ the new count
    void SetCount           (size_t  val_);
    
    /// @brief SetIndentSize allows reassinging the indent size
    /// @param val_ the new indent size
    void SetIndentSize      (size_t  val_);
    
    /// @brief SetSepartor allows reassinging the separator
    /// @param val_ the new separator
    void SetSeparator       (CStr   &val_);
    
    /// @brief SetRootName allows reassinging the root name
    /// @param val_ the new root name
    void SetRootName        (CStr   &val_);
    
    /// @brief SetEnableNewLines allows reassinging the enable new lines flag
    /// @param val_ the new new-lines flags
    void SetEnableNewLines  (bool    val_);
    
    /// @brief SetShowTableAddress allows reassinging the show table address flag
    /// @param val_ the new show table address flag
    void SetShowTableAddress(bool    val_);
    
    /// @brief SetSerialize allows reassinging the serialize flag
    /// @param val_ the new serialize flag
    void SetSerialize       (const SPtr &val_);
    
    /// @brief IOOptions serialization operator
    /// @param out output stream to serialize IOOptions
    /// @param o object to serialize
    /// @return output stream
    friend std::ostream &operator<<(std::ostream &out, const IOOptions &o);
    
  private:
    int         index;               ///< index
    size_t      count;               ///< count
    size_t      indentSize;          ///< indent size
    std::string separator;           ///< separator
    std::string rootName;            ///< root name
    bool        enableNewLines;      ///< enable new lines flag
    bool        showTableAddress;    ///< show table address flag
    SPtr        serialize;           ///< serialize flag
  };
  
}

#endif
