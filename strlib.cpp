// Copyright (C) 2014-2016 Eric Hebert (ayebear)
// This code is licensed under MIT, see LICENSE.txt for details.

#include "strlib.h"
#include <fstream>
#include <algorithm>
#include <cctype>

namespace strlib
{

void trimWhitespace(std::string& str)
{
    while (!str.empty() && std::isspace(str.back())) // Loop until right side contains no whitespace
        str.pop_back(); // Remove last character
    while (!str.empty() && std::isspace(str.front())) // Loop until left side contains no whitespace
        str.erase(str.begin()); // Remove first character
}

void stripNewLines(std::string& str)
{
    if (!str.empty())
    {
        for (char newLineChar: "\r\n")
            str.erase(std::remove(str.begin(), str.end(), newLineChar), str.end());
    }
}

std::string toLower(const std::string& str)
{
    // Make all of the characters lowercase
    std::string tmpStr(str);
    for (char& c: tmpStr)
        c = tolower(c);
    return tmpStr;
}

std::string toUpper(const std::string& str)
{
    // Make all of the characters uppercase
    std::string tmpStr(str);
    for (char& c: tmpStr)
        c = toupper(c);
    return tmpStr;
}

bool mustEndWith(std::string& str, const std::string& endStr)
{
    bool endsWith = true;
    // If the original string is smaller than what it should end with
    // Or if the original string does not end with the ending string
    if (str.size() < endStr.size() || str.compare(str.size() - endStr.size(), endStr.size(), endStr))
    {
        str += endStr; // Then we know it doesn't already end with it!
        endsWith = false;
    }
    return endsWith;
}

size_t replaceAll(std::string& str, const std::string& findStr, const std::string& replaceStr)
{
    size_t count{0};
    size_t pos{0};
    // Keep searching for the string to find
    while ((pos = str.find(findStr, pos)) != std::string::npos)
    {
        // Replace the found string with the replace string
        str.replace(pos, findStr.length(), replaceStr);
        pos += replaceStr.size();
        ++count;
    }
    return count; // Return the number of occurrences that were replaced
}

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    size_t start{0};
    size_t end{0};
    std::vector<std::string> elements;

    // Keep searching for the delimiters to split
    while ((end = str.find(delim, start)) != std::string::npos)
    {
        // Extract the string between the delimiters, and add it to the vector
        elements.push_back(str.substr(start, end - start));
        start = end + delim.size();
    }

    // Get the last part of the string
    if (start < str.size())
        elements.push_back(str.substr(start));

    return elements;
}

std::vector<std::string> getLinesFromString(const std::string& str)
{
    std::string tmpStr(str);

    // First, search and replace all CRLF with LF
    // This needs to be a while loop in case there is something like "\r\r\n"
    while (replaceAll(tmpStr, "\r\n", "\n"));

    // Then, replace CR with LF
    replaceAll(tmpStr, "\r", "\n");

    // Finally, split on LF
    return split(tmpStr, "\n");
}

bool readLinesFromFile(const std::string& filename, std::vector<std::string>& lines)
{
    bool status = false;
    std::ifstream file(filename, std::ifstream::in); // Open the file
    if (file.is_open())
    {
        std::string line;
        while (getline(file, line)) // Read a line
        {
            stripNewLines(line); // Make sure to strip any leftover new line characters
            lines.push_back(line); // Store the line
        }
        status = true;
    }
    return status;
}

bool writeStringToFile(const std::string& filename, const std::string& data)
{
    bool status = false;
    if (!filename.empty())
    {
        std::ofstream outFile(filename, std::ofstream::out | std::ofstream::trunc);
        if (outFile.is_open())
        {
            outFile << data; // Write the string to the file
            outFile.close();
            status = true;
        }
    }
    return status;
}

bool strToBool(const std::string& str)
{
    // Check if the string is "true", or if the parsed value is non-zero
    return (toLower(str) == "true" || fromString<int>(str) != 0);
}

}
