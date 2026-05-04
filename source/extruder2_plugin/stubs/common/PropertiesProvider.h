/// @file  PropertiesProvider.h
/// @brief Stub replacement for the Oksygen Common::Data::PropertiesProvider.
///        Provides a minimal Properties class and LoadPropertyTree function
///        sufficient for the extruder2_plugin to compile and run standalone.

#ifndef COMMON_PROPERTIES_PROVIDER_H
#define COMMON_PROPERTIES_PROVIDER_H

#include <map>
#include <mutex>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace Common
{
    /// Minimal INI-style properties class (stub for Oksygen Common::Properties).
    /// Supports [Section] / Key = Value format.
    class Properties
    {
    public:
        Properties() = default;

        // Copy constructor
        Properties(const Properties& other)
        {
            std::lock_guard<std::mutex> lock(other.mutex_);
            data_ = other.data_;
        }

        explicit Properties(const std::string& filename)
        {
            Open(filename);
        }

        void Open(const std::string& filename)
        {
            std::ifstream file(filename);
            if (!file.is_open())
                return;

            std::string line;
            std::string current_section;

            while (std::getline(file, line))
            {
                // Trim whitespace
                auto start = line.find_first_not_of(" \t\r\n");
                if (start == std::string::npos)
                    continue;
                line = line.substr(start);

                // Skip comments
                if (line[0] == ';' || line[0] == '#')
                    continue;

                // Section header
                if (line[0] == '[')
                {
                    auto end = line.find(']');
                    if (end != std::string::npos)
                        current_section = line.substr(1, end - 1);
                    continue;
                }

                // Key = Value
                auto eq = line.find('=');
                if (eq != std::string::npos)
                {
                    std::string key = line.substr(0, eq);
                    std::string val = line.substr(eq + 1);

                    // Trim
                    auto trim = [](std::string& s) {
                        auto a = s.find_first_not_of(" \t\r\n");
                        auto b = s.find_last_not_of(" \t\r\n");
                        s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
                    };
                    trim(key);
                    trim(val);

                    std::lock_guard<std::mutex> lock(mutex_);
                    data_[{current_section, key}] = val;
                }
            }
        }

        template <typename T>
        T GetProperty(const std::string& section, const std::string& key, T default_value) const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = data_.find({section, key});
            if (it == data_.end())
                return default_value;

            std::istringstream ss(it->second);
            T result;
            if (ss >> result)
                return result;
            return default_value;
        }

    private:
        std::map<std::pair<std::string, std::string>, std::string> data_;
        mutable std::mutex mutex_;
    };

    // Specialisation for std::string to handle spaces
    template <>
    inline std::string Properties::GetProperty<std::string>(
        const std::string& section, const std::string& key, std::string default_value) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find({section, key});
        if (it == data_.end())
            return default_value;
        return it->second;
    }

    namespace Data
    {
        /// Stub LoadPropertyTree - just opens a single INI file (no recursive defaults).
        inline Properties LoadPropertyTree(const std::string& filename)
        {
            return Properties(filename);
        }
    }
}

#endif // COMMON_PROPERTIES_PROVIDER_H
