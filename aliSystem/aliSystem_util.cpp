#include <aliSystem_util.hpp>
#include <algorithm>
#include <cstring>

namespace aliSystem {
  namespace Util {

    void DoNothing() {}

    StrSelectFn GetMatchShortestFn(const std::string &comp) {
      return [=](const std::string &str) -> bool {
	const char   *aPtr = comp.c_str();
	const size_t  aLen = comp.size();
	const char   *bPtr = str.c_str();
	const size_t  bLen = str.size();
	if (aLen<=bLen) {
	  return strncmp(aPtr,bPtr,aLen)==0;
	} else {
	  return strncmp(aPtr,bPtr,bLen)==0;
	}
      };
    }
    StrSelectFn GetMatchExactFn(const std::string &comp) {
      return [=](const std::string &str) -> bool {
	return comp==str;
      };
    }
    StrSelectFn GetMatchPrefixFn(const std::string &prefix) {
      return [=](const std::string &str) -> bool {
	const char   *aPtr = prefix.c_str();
	const size_t  aLen = prefix.size();
	const char   *bPtr = str.c_str();
	const size_t  bLen = str.size();
	return aLen<=bLen && strncmp(aPtr,bPtr,std::min(aLen,bLen))==0;
      };
    }

    bool Always() { return true;  }
    bool Never()   { return false; }

  }
}
