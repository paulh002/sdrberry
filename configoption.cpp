// Copyright (C) 2014-2016 Eric Hebert (ayebear)
// This code is licensed under MIT, see LICENSE.txt for details.

#include "configoption.h"

namespace cfg
{

Option::OptionVector Option::emptyVector;

Option::Option(const std::string& data)
{
    setString(data);
}

Option::Option(const Option& data)
{
    operator=(data);
}

void Option::reset()
{
    removeRange();
    options.reset();
    operator=(0);
}

bool Option::operator=(const char* data)
{
    return setString(data);
}

bool Option::operator=(const std::string& data)
{
    return setString(data);
}

Option& Option::operator=(const Option& data)
{
    text = data.text;
    integer = data.integer;
    decimal = data.decimal;
    boolean = data.boolean;
    quotes = data.quotes;
    minEnabled = data.minEnabled;
    maxEnabled = data.maxEnabled;
    rangeMin = data.rangeMin;
    rangeMax = data.rangeMax;
    if (data.options)
        options = std::make_unique<OptionVector>(*data.options);
    else
        options.reset();
    return *this;
}

bool Option::setString(const std::string& data)
{
    std::istringstream stream(data);
    double value{};

    // Try to parse a value from the string
    bool success = (stream >> value) && (static_cast<size_t>(stream.gcount()) == data.size());

    // Only set the value if it's in range
    if (isInRange(value))
    {
        decimal = value;
        integer = decimal;
        text = data;

        // Check for a boolean value
        auto lowerStr = strlib::toLower(data);
        bool isTrue = (lowerStr == "true");
        bool isFalse = (lowerStr == "false");

        // Determine if quotes are necessary
        quotes = !(success || isFalse || isTrue);

        // Convert to a boolean ("true" means true, or any non-zero value)
        boolean = (success ? (decimal != 0) : isTrue);

        return true;
    }
    return false;
}

const std::string& Option::toString() const
{
    return text;
}

std::string Option::toStringWithQuotes() const
{
    // Automatically append quotes to the string if it originally had them
    return (quotes ? ('"' + text + '"') : text);
}

int Option::toInt() const
{
    return integer;
}

long Option::toLong() const
{
    return static_cast<long>(integer);
}

float Option::toFloat() const
{
    return static_cast<float>(decimal);
}

double Option::toDouble() const
{
    return decimal;
}

bool Option::toBool() const
{
    return boolean;
}

char Option::toChar() const
{
    return static_cast<char>(integer);
}

void Option::get(std::string& val) const
{
    val = text;
}

void Option::get(long& val) const
{
    val = integer;
}

void Option::get(double& val) const
{
    val = decimal;
}

void Option::get(bool& val) const
{
    val = boolean;
}

Option::operator const std::string&() const
{
    return text;
}

void Option::setQuotes(bool setting)
{
    quotes = setting;
}

bool Option::hasQuotes()
{
    return quotes;
}

void Option::setMin(double minimum)
{
    rangeMin = minimum;
    minEnabled = true;
}

void Option::setMax(double maximum)
{
    rangeMax = maximum;
    maxEnabled = true;
}

void Option::setRange(double minimum, double maximum)
{
    setMin(minimum);
    setMax(maximum);
}

void Option::removeRange()
{
    minEnabled = false;
    maxEnabled = false;
}

Option& Option::push(const Option& opt)
{
    if (!options)
        options = std::make_unique<OptionVector>();
    options->push_back(opt);
    return options->back();
}

void Option::pop()
{
    if (options && !options->empty())
        options->pop_back();
}

Option& Option::operator[](unsigned pos)
{
    return ((*options)[pos]);
}

Option& Option::back()
{
    return options->back();
}

unsigned Option::size() const
{
    return (options ? options->size() : 0);
}

void Option::clear()
{
    options.reset();
}

Option::OptionVector::iterator Option::begin()
{
    if (options)
        return options->begin();
    return emptyVector.begin();
}

Option::OptionVector::iterator Option::end()
{
    if (options)
        return options->end();
    return emptyVector.end();
}

Option::OptionVector::const_iterator Option::cbegin() const
{
    if (options)
        return options->cbegin();
    return emptyVector.cbegin();
}

Option::OptionVector::const_iterator Option::cend() const
{
    if (options)
        return options->cend();
    return emptyVector.cend();
}

std::string Option::buildArrayString(const std::string& indentStr) const
{
    // Continue building array strings until the option is just a single element and not an array
    if (options)
    {
        std::string nextIndentStr(indentStr + '\t');

        // Build the array string
        std::string arrayStr("{\n");
        unsigned arraySize = options->size();
        for (unsigned i = 0; i < arraySize; ++i)
        {
            arrayStr += nextIndentStr;
            arrayStr += (*options)[i].buildArrayString(nextIndentStr);
            if (i < arraySize - 1)
                arrayStr += ",\n";
        }
        arrayStr += '\n' + indentStr + '}';
        return arrayStr;
    }
    else
        return toStringWithQuotes();
}

bool Option::isInRange(double num)
{
    return ((!minEnabled || num >= rangeMin) &&
            (!maxEnabled || num <= rangeMax));
}

std::ostream& operator<<(std::ostream& stream, const Option& option)
{
    stream << option.toString();
    return stream;
}

}
