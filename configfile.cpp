// Copyright (C) 2014-2016 Eric Hebert (ayebear)
// This code is licensed under MIT, see LICENSE.txt for details.

#include "configfile.h"
#include <fstream>
#include <iostream>
#include "strlib.h"

namespace cfg
{

File::File()
{
    setFlags();
}

File::File(const std::string& filename, int newFlags)
{
    setFlags(newFlags);
    loadFromFile(filename);
}

File::File(const ConfigMap& defaultOptions, int newFlags)
{
    setFlags(newFlags);
    setDefaultOptions(defaultOptions);
}

File::File(const std::string& filename, const ConfigMap& defaultOptions, int newFlags)
{
    setFlags(newFlags);
    setDefaultOptions(defaultOptions);
    loadFromFile(filename);
}

File::~File()
{
    if (flags & Autosave)
        writeToFile();
}

bool File::loadFromFile(const std::string& filename)
{
    configFilename = filename;
    std::vector<std::string> lines;
    fileIoSuccessful = strlib::readLinesFromFile(configFilename, lines);
    if (fileIoSuccessful)
        parseLines(lines);
    else if (flags & Verbose)
        std::cout << "Error loading \"" << configFilename << "\"\n";
    return fileIoSuccessful;
}

void File::loadFromString(const std::string& str)
{
    auto lines = strlib::getLinesFromString(str);
    parseLines(lines);
}

bool File::writeToFile(std::string filename) const
{
    if (filename.empty())
        filename = configFilename;
    // Write the std::string to the output file
    fileIoSuccessful = strlib::writeStringToFile(filename, buildString());
    if (!fileIoSuccessful && (flags & Verbose))
        std::cout << "Error writing \"" << configFilename << "\"\n";
    return fileIoSuccessful;
}

void File::writeToString(std::string& str) const
{
    for (const auto& section: options) // Go through all of the sections
    {
        if (!section.first.empty())
            str += '[' + section.first + "]\n"; // Add the section line if it is not blank
        for (const auto& o: section.second) // Go through all of the options in this section
            str += o.first + " = " + o.second.buildArrayString() + '\n';
        str += '\n';
    }
    if (!str.empty() && str.back() == '\n')
        str.pop_back(); // Strip the extra new line at the end
}

std::string File::buildString() const
{
    std::string configStr;
    writeToString(configStr);
    return configStr;
}

File::operator bool() const
{
    return fileIoSuccessful;
}

bool File::getStatus() const
{
    return fileIoSuccessful;
}

void File::setFlag(int flag, bool state)
{
    if (state)
        flags |= flag;
    else
        flags &= ~flag;
}

void File::setFlags(int newFlags)
{
    flags = newFlags;
}

Option& File::operator()(const std::string& name, const std::string& section)
{
    return options[section][name];
}

Option& File::operator()(const std::string& name)
{
    return options[currentSection][name];
}

bool File::optionExists(const std::string& name, const std::string& section) const
{
    auto sectionFound = options.find(section);
    return (sectionFound != options.end() && sectionFound->second.find(name) != sectionFound->second.end());
}

bool File::optionExists(const std::string& name) const
{
    return optionExists(name, currentSection);
}

void File::setDefaultOptions(const ConfigMap& defaultOptions)
{
    options.insert(defaultOptions.begin(), defaultOptions.end());
}

void File::useSection(const std::string& section)
{
    currentSection = section;
}

File::ConfigMap::iterator File::begin()
{
    return options.begin();
}

File::ConfigMap::iterator File::end()
{
    return options.end();
}

File::Section& File::getSection(const std::string& section)
{
    return options[section];
}

File::Section& File::getSection()
{
    return options[currentSection];
}

bool File::sectionExists(const std::string& section) const
{
    return (options.find(section) != options.end());
}

bool File::sectionExists() const
{
    return sectionExists(currentSection);
}

bool File::eraseOption(const std::string& name, const std::string& section)
{
    bool status = false;
    auto sectionFound = options.find(section);
    if (sectionFound != options.end()) // If the section exists
        status = (sectionFound->second.erase(name) > 0); // Erase the option
    return status;
}

bool File::eraseOption(const std::string& name)
{
    return eraseOption(name, currentSection);
}

bool File::eraseSection(const std::string& section)
{
    return (options.erase(section) > 0);
}

bool File::eraseSection()
{
    return eraseSection(currentSection);
}

void File::clear()
{
    options.clear();
}

void File::parseLines(std::vector<std::string>& lines)
{
    currentArrayStack.clear();
    arrayOptionName.clear();
    std::string section;
    bool multiLineComment = false;
    Comment commentType = Comment::None;
    for (std::string& line: lines) // Iterate through the std::vector of strings
    {
        strlib::trimWhitespace(line);
        commentType = stripComments(line, multiLineComment);

        if (commentType == Comment::Start)
            multiLineComment = true;

        if (!multiLineComment && !line.empty()) // Don't process a comment or an empty line
        {
            if (isSection(line))
                parseSectionLine(line, section); // Example: "[Section]"
            else
                parseOptionLine(line, section); // Example: "Option = Value"
        }

        if (commentType == Comment::End)
            multiLineComment = false;
    }
}

bool File::isSection(const std::string& section) const
{
    return (section.size() >= 2 && section.front() == '[' && section.back() == ']');
}

void File::parseSectionLine(const std::string& line, std::string& section)
{
    section = line.substr(1, line.size() - 2); // Set the current section
    options[section]; // Add that section to the map
}

void File::parseOptionLine(const std::string& line, const std::string& section)
{
    if (!currentArrayStack.empty())
    {
        // Process another line in the array
        // This could be 1 of 3 things:
        // 1. { (array start)
        // 2. X (another value)
        // 3. } (array end)
        std::string value = line;
        strlib::trimWhitespace(value);
        if (value.back() == ',')
            value.pop_back();
        strlib::trimWhitespace(value); // In case there was whitespace before the comma
        if (!value.empty())
        {
            Option& option = getArrayOption(section, arrayOptionName);
	        if ((value == "{") || (value[0] == '{'))
	        {
		        startArray(option);
	        }
            else if ((value == "}") || (value[0] == '}'))
                currentArrayStack.pop_back();
            else
                setOption(option.push(), value);
        }
    }
    else
    {
        // Process a regular option line (or the start of a new array)
        size_t equalPos = line.find("="); // Find the position of the "=" symbol
        if (equalPos != std::string::npos && equalPos >= 1) // Ignore the line if there is no "=" symbol
        {
            std::string name, value;
            // Extract the name and value
            name = line.substr(0, equalPos);
            value = line.substr(equalPos + 1);
            // Trim any whitespace around the name and value
            strlib::trimWhitespace(name);
            strlib::trimWhitespace(value);
            // Check if this is the start of an array
            Option& option = options[section][name];
            if ((value == "{") || (value[0] == '{'))
            {
	            bool bline = false;
	            arrayOptionName = name;
                currentArrayStack.assign(1, 0);
	            if (value.find(',') > 0)
	            {
		            Option& option = getArrayOption(section, arrayOptionName);
		            startArray(option);
		            bline = true;
		            int n = value.find('}');
		            if (n != -1)
		                value.erase(n);		         
	            }
	            while (value.find(',') != -1)
	            {   
		            int n = value.find(',');
		            std::string s = value.substr(1, n);
		            setOption(option.push(), s);
		            value.erase(1, n);
	            }
	            if (bline)
	            {
		            currentArrayStack.clear();
		            arrayOptionName.clear();
	            }
            }
            else
            {
                if (!setOption(option, value) && (flags & Verbose))
                {
                    std::cout << "Warning: Option \"" << name << "\" in [" << section << "] was out of range.\n";
                    std::cout << "    Using default value: " << option.toStringWithQuotes() << std::endl;
                }
            }
        }
    }
}

bool File::setOption(Option& option, const std::string& value)
{
    std::string trimmedValue = value;
    bool trimmedQuotes = trimQuotes(trimmedValue); // Remove quotes if any
    bool optionSet = (option = trimmedValue); // Try to set the option
    if (trimmedQuotes) // If quotes were removed
        option.setQuotes(true); // Add quotes to the option
    return optionSet;
}

Option& File::getArrayOption(const std::string& section, const std::string& name)
{
    Option* currentOption = &options[section][name];
    // Start at 1, because the 0th element represents the current option above
    for (unsigned i = 1; i < currentArrayStack.size(); ++i)
        currentOption = &((*currentOption)[currentArrayStack[i]]);
    return *currentOption;
}

void File::startArray(Option& option)
{
    option.push(); // This new option will be holding the array starting with this "{"
    currentArrayStack.push_back(option.size() - 1);
}

bool File::areQuotes(char c1, char c2)
{
    // Both characters must be the same, either single quotes or double quotes
    return ((c1 == c2) && (c1 == '"' || c1 == '\''));
}

bool File::trimQuotes(std::string& str)
{
    bool status = false;
    if (str.size() >= 2 && areQuotes(str.front(), str.back()))
    {
        // Remove the quotes
        str.pop_back();
        str.erase(str.begin());
        status = true;
    }
    return status;
}

bool File::isEndComment(const std::string& str) const
{
    return (str.find("*/") != std::string::npos);
}

File::Comment File::getCommentType(const std::string& str, bool checkEnd) const
{
    // Comment symbols and types
    static const std::vector<std::string> commentSymbols = {"/*", "//", "#", "::", ";"};
    static const std::vector<Comment> commentTypes = {Comment::Start, Comment::Single, Comment::Single, Comment::Single, Comment::Single};

    Comment commentType = Comment::None;

    // This is optional so the function returns the correct result
    if (checkEnd && isEndComment(str))
        commentType = Comment::End;

    // Find out which comment type the string is, if any
    for (unsigned int i = 0; (commentType == Comment::None && i < commentSymbols.size()); i++)
    {
        if (str.compare(0, commentSymbols[i].size(), commentSymbols[i]) == 0)
            commentType = commentTypes[i];
    }

    // This check is required for comments using the multi-line symbols on a single line
    if (!checkEnd && commentType == Comment::Start && isEndComment(str))
        commentType = Comment::Single;

    return commentType;
}

File::Comment File::stripComments(std::string& str, bool checkEnd)
{
    Comment commentType = getCommentType(str, checkEnd);
    if (commentType == Comment::Single)
        str.clear();
    return commentType;
}

}
