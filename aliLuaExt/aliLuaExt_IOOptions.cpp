#include <aliLuaExt_IOOptions.hpp>
#include <aliSystem.hpp>

namespace aliLuaExt {
  using SPtr = aliSystem::Codec::Serialize::Ptr; ///< serialize ptr
  
  IOOptions::IOOptions(int    index_,
		       size_t count_)
    : index(index_),
      count(count_),
      indentSize(2),
      separator(", "),
      rootName("root"),
      enableNewLines(true),
      showTableAddress(false) {
  }
  
  int                IOOptions::GetIndex           () const { return index;            }
  size_t             IOOptions::GetCount           () const { return count;            }
  size_t             IOOptions::GetIndentSize      () const { return indentSize;       }
  const std::string &IOOptions::GetSeparator       () const { return separator;        }      
  const std::string &IOOptions::GetRootName        () const { return rootName;         }      
  bool               IOOptions::GetEnableNewLines  () const { return enableNewLines;   }
  bool               IOOptions::GetShowTableAddress() const { return showTableAddress; }
  SPtr               IOOptions::GetSerialize       () const { return serialize;        }
  
  void IOOptions::SetIndex           (int    val_) { index            = val_; }
  void IOOptions::SetCount           (size_t val_) { count            = val_; }
  void IOOptions::SetIndentSize      (size_t val_) { indentSize       = val_; }
  void IOOptions::SetSeparator       (CStr  &val_) { separator        = val_; }
  void IOOptions::SetRootName        (CStr  &val_) { rootName         = val_; }
  void IOOptions::SetEnableNewLines  (bool   val_) { enableNewLines   = val_; }
  void IOOptions::SetShowTableAddress(bool   val_) { showTableAddress = val_; }
  void IOOptions::SetSerialize       (const SPtr &val_) {
    serialize = val_;
  }


  std::ostream &operator<<(std::ostream &out, const IOOptions &o) {
    out << "IOOptions"
	<< "\n   index            = " << o.index
	<< "\n   count            = " << o.count
      	<< "\n   indentSize       = " << o.indentSize
      	<< "\n   separator        = " << o.separator
      	<< "\n   rootName         = " << o.rootName
      	<< "\n   enableNewLines   = " << YES_NO(o.enableNewLines)
      	<< "\n   showTableAddress = " << YES_NO(o.showTableAddress)
      	<< "\n   serialoze        = " << YES_NO(o.serialize);
      
    return out;
  }

}
