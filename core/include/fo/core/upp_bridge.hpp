#pragma once

/**
 * @file upp_bridge.hpp
 * @brief Ultimate++ (U++) Abstraction Layer
 * 
 * This header conditionally maps standard C++ types to their U++ equivalents
 * when the FO_USE_ULTIMATEPP CMake flag is enabled. This allows the core
 * engine to slowly adopt U++ memory allocators and highly optimized containers
 * while remaining fully compatible with the standard library.
 */

#ifdef FO_USE_ULTIMATEPP
#include <Core/Core.h>

namespace fo::core {
    using String = Upp::String;
    using WString = Upp::WString;
    
    template<typename T>
    using Vector = Upp::Vector<T>;
    
    template<typename T>
    using Array = Upp::Array<T>;

    template<typename K, typename V>
    using Map = Upp::VectorMap<K, V>;

    // Type casting helpers
    inline std::string to_std_string(const String& u_str) {
        return std::string(u_str.Begin(), u_str.End());
    }

    inline String to_upp_string(const std::string& std_str) {
        return String(std_str.c_str(), std_str.length());
    }
} // namespace fo::core

#else // Use Standard Library equivalents
#include <string>
#include <vector>
#include <map>

namespace fo::core {
    using String = std::string;
    using WString = std::wstring;
    
    template<typename T>
    using Vector = std::vector<T>;
    
    // Note: std::vector is used here as a fallback for U++ Array since 
    // std::array requires a compile-time size.
    template<typename T>
    using Array = std::vector<T>;

    template<typename K, typename V>
    using Map = std::map<K, V>;

    // No-op type casting helpers for std mode
    inline std::string to_std_string(const std::string& str) { return str; }
    inline std::string to_upp_string(const std::string& str) { return str; }
} // namespace fo::core

#endif
