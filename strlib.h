// Copyright (C) 2014-2016 Eric Hebert (ayebear)
// This code is licensed under MIT, see LICENSE.txt for details.

#ifndef STRLIB_H
#define STRLIB_H

#include <string>
#include <sstream>
#include <vector>
#include <cctype>

namespace strlib
{

/// String manipulation =======================================================

// Trims all whitespace on both sides of the string
void trimWhitespace(std::string& str);

// Removes all new lines and carriage returns from a string
void stripNewLines(std::string& str);

// Creates an all-lowercase version of the passed in string
std::string toLower(const std::string& str);

// Creates an all-uppercase version of the passed in string
std::string toUpper(const std::string& str);

// Appends the second string if the first doesn't end with it
bool mustEndWith(std::string& str, const std::string& endStr);

// Replaces all instances of a sub-string with another string, and returns the number of replaces
size_t replaceAll(std::string& str, const std::string& findStr, const std::string& replaceStr);

// Splits a string into a vector of strings using a delimeter string
std::vector<std::string> split(const std::string& str, const std::string& delim);

// Splits a string using a delimeter, parses each value as the specified type,
// then returns a vector of the elements.
template <typename T>
std::vector<T> split(const std::string& str, const std::string& delim, T defaultValue = 0);

// Joins elements from any container into a string
template <typename T>
std::string join(T&& elements, const std::string& sepStr);

/// File operations ===========================================================

// Splits a string into separate lines using the CR and/or LF characters
std::vector<std::string> getLinesFromString(const std::string& str);

// Reads a file into a vector as separate lines
bool readLinesFromFile(const std::string& filename, std::vector<std::string>& lines);

// Writes a string to a file, will overwrite an existing file
bool writeStringToFile(const std::string& filename, const std::string& data);


/// String converting =========================================================

// Parses a string to determine its boolean value
bool strToBool(const std::string& str);

// Converts most types to strings using a string stream
template <typename T>
std::string toString(T data, unsigned precision = 16)
{
    std::ostringstream tmp;
    tmp.precision(precision);
    tmp << data;
    return tmp.str();
}

// Converts a bool to a string
template <typename T>
std::string toString(bool data)
{
    return (data ? "true" : "false");
}

// Converts a string to most types
template <typename T>
T fromString(const std::string& str, T defaultValue = 0)
{
    T val;
    std::istringstream stream(str);
    if ((stream >> val).fail())
        val = defaultValue;
    return val;
}


/// Template implementations ===================================================

template <typename T>
std::vector<T> split(const std::string& str, const std::string& delim, T defaultValue)
{
    // Split the string into a vector of strings
    auto strs = split(str, delim);

    // Parse the values into a vector of the specified type
    std::vector<T> values;
    values.reserve(strs.size());
    for (auto& s: strs)
        values.push_back(fromString<T>(s, defaultValue));
    return values;
}

template <typename T>
std::string join(T&& elements, const std::string& sepStr)
{
    std::ostringstream stream;
    auto first = &(*elements.begin());
    for (const auto& elem: elements)
    {
        if (&elem != first)
            stream << sepStr;
        stream << elem;
    }
    return stream.str();
}

}

#endif
