#ifndef INCLUDED_ALI_SYSTEM_LOGGING
#define INCLUDED_ALI_SYSTEM_LOGGING

#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <aliSystem_time.hpp>

//
// This module defines some trivial logging functions for the aliSystem library
// and any application that leverages the library.

namespace aliSystem {
  namespace Logging {
    /// @brief function for flagging wheather or not to print debug messages
    /// @return true if a debug  message should be printed
    bool IsDebug();
    std::string CurTime();
    
  }
}


/// @brief returns a log record prefix (time, file, line, function)
#define PREFIX(X) aliSystem::Logging::CurTime() << " " << X << " (" << __FILE__ << " " << __LINE__ << " " << __func__ << ") "

/// @brief logs the the passed output chain to stdout while also collecting the message.
/// @param X data to log (and collect)
#define BASE( X)  std::ostringstream ss; ss << X; std::cout << ss.str() << std::endl;

/// @brief generate a debug log line, only logged if debug logs are enabled
/// @param X data to log
#define DEBUG(X) do { if (aliSystem::Logging::IsDebug()) { std::cout << PREFIX("DEBUG") << X << std::endl; } } while (0)

/// @brief generate an info log line
/// @param X data to log
#define INFO( X) do { std::cout << PREFIX("INFO ") << X << std::endl; } while (0)

/// @brief generate an warning log line
/// @param X data to log
#define WARN( X) do { std::cout << PREFIX("WARN ") << X << std::endl; } while (0)

/// @brief generate an error log line
/// @param X data to log
#define ERROR(X) do { std::cout << PREFIX("ERROR") << X << std::endl; } while (0)

/// @brief generate a debug log line if debugging is enabled and flagged to log
/// @param cond condition indicating whether or not to log a line
/// @param X data to log
#define DEBUG_IF(cond, X) do { if (cond) DEBUG(X); } while (0)

/// @brief generate an info log line if flagged to log
/// @param cond condition indicating whether or not to log a line
/// @param X data to log
#define INFO_IF( cond, X) do { if (cond) INFO(X);  } while (0)

/// @brief generate a warning log line if flagged to log
/// @param cond condition indicating whether or not to log a line
/// @param X data to log
#define WARN_IF( cond, X) do { if (cond) WARN(X);  } while (0)

/// @brief generate an error log line if flagged to log
/// @param cond condition indicating whether or not to log a line
/// @param X data to log
#define ERROR_IF(cond, X) do { if (cond) ERROR(X); } while (0)

/// @brief generate a "throw" log line and throw a std::exception with the log message.
/// @param X data to log
#define THROW(X)          do {			\
    BASE(PREFIX("THROW") << X);			\
    throw std::runtime_error(ss.str());		\
  } while(0)

/// @brief generate a "throw" log line and throw a std::exception with the log message if flagged to log.
/// @param cond condition indicating whether or not to log the error (and throw the exception)
/// @param X data to log
#define THROW_IF(cond, X) do { if (cond) { THROW(X); } } while (0)

/// @brief sipmle yes/no generator
/// @param cond if true, return yes, if false return no
/// @retruns the string literal yes or no
#define YES_NO(cond) ((cond) ? "yes"  : "no"   )

/// @brief sipmle true/false generator
/// @param cond if true, return true, if false return false
/// @retruns the string literal true or false
#define TRUE_FALSE(cond)    ((cond) ? "true" : "false")

#endif
