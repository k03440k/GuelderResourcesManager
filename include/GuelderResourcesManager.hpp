#pragma once

#define NOMINMAX

#include <string>
#include <string_view>
#include <limits>
#include <vector>
#include <memory>
#include <charconv>
#include <stdexcept>
#include <format>
#include <concepts>
#include <unordered_map>
#include <array>
#include <filesystem>

#ifdef WIN32
#include <Windows.h>
#endif

//idk #define NOMINMAX doesn't work
//#undef max

#ifndef GE_SCOPE_OPEN
#define GE_SCOPE_OPEN '{'
#endif

#ifndef GE_SCOPE_CLOSE
#define GE_SCOPE_CLOSE '}'
#endif

#ifndef GE_NAMESPACE_KEYWORD
#define GE_NAMESPACE_KEYWORD "ns"
#endif

#ifndef GE_VARIABLE_VALUE_SCOPE
#define GE_VARIABLE_VALUE_SCOPE '\"'
#endif

#ifndef GE_COMMENT_SCOPE_LINE
#define GE_COMMENT_SCOPE_LINE "//"
#endif

#ifndef GE_SPECIAL_CHAR_SIGN
#define GE_SPECIAL_CHAR_SIGN '\\'
#endif

#ifndef GE_SPECIAL_CHARS
#define GE_SPECIAL_CHARS "\"\\"
#endif

#ifndef GE_NEWLINE
#define GE_NEWLINE '\n'
#endif

#ifndef GE_WHITESPACE
#define GE_WHITESPACE ' '
#endif

#ifndef GE_PATH_SEPARATOR
#define GE_PATH_SEPARATOR '/'
#endif

#ifndef GE_EQUALS
#define GE_EQUALS '='
#endif

#ifndef GE_SEMICOLON
#define GE_SEMICOLON ';'
#endif

//converters
namespace GuelderResourcesManager
{
    template<typename T>
    concept IsNumber = std::integral<T> || std::floating_point<T>;

    template<typename StringType>
    concept String = requires(StringType str)
    {
        typename StringType::value_type;
        str.data();
        str.size();
    };

    template<IsNumber Integer>
    Integer StringToNumber(const std::string_view& str)
    {
        Integer res = 0;

        auto [wrongChar, errorCode] = std::from_chars(str.data(), str.data() + str.size(), res);

#ifndef GE_SOFT
        if(static_cast<int>(errorCode) != 0)
            throw std::invalid_argument(std::format("Failed to convert {} to {}", str, typeid(Integer).name()));
#endif

        return res;
    }
    inline bool StringToBool(const std::string_view& str)
    {
        if(str == "true" || str == "1")
            return true;
        else if(str == "false" || str == "0")
            return false;

#ifndef GE_SOFT
        throw std::invalid_argument(std::format("Failed to convert {} to bool.", str));
#endif
        return false;
    }
    /**
    * \brief WARNING: This func only works properly with windows, because I don't give a fuck about Linux or MacOS.
    */
    inline std::wstring StringToWString(const std::string_view& str)
    {
#ifdef WIN32
        const int neededSize = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, nullptr, 0);
        std::wstring wString(neededSize, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, wString.data(), neededSize);
#else
        std::wstring wString{};
#endif

        return wString;
    }
    /**
     * \brief WARNING: This func only works properly with windows, because I don't give a fuck about Linux or MacOS.
     */
    inline std::string WStringToString(const std::wstring_view& wStr)
    {
#ifdef WIN32
        const int neededSize = WideCharToMultiByte(CP_UTF8, 0, wStr.data(), wStr.size(), nullptr, 0, nullptr, nullptr);
        std::string string(neededSize, L'\0');
        WideCharToMultiByte(CP_UTF8, 0, wStr.data(), wStr.size(), string.data(), neededSize, nullptr, nullptr);
#else
        std::string string;
#endif

        return string;
    }
}

//variables
namespace GuelderResourcesManager
{
    struct Variable;
    enum class DataType : uint8_t
    {
        Invalid = 0,
        Int,
        UInt,
        Long,
        ULong,
        LongLong,
        ULongLong,
        Short,
        UShort,
        Char,
        UChar,
        Float,
        Double,
        LongDouble,
        Bool,
        String
    };

    inline DataType StringToDataType(const std::string_view& str)
    {
        static const std::unordered_map<std::string_view, DataType> lookup =
        {
            {"Invalid", DataType::Invalid},
            {"Int", DataType::Int},
            {"UInt", DataType::UInt},
            {"Long", DataType::Long},
            {"ULong", DataType::ULong},
            {"LongLong", DataType::LongLong},
            {"ULongLong", DataType::ULongLong},
            {"Short", DataType::Short},
            {"UShort", DataType::UShort},
            {"Char", DataType::Char},
            {"UChar", DataType::UChar},
            {"Float", DataType::Float},
            {"Double", DataType::Double},
            {"LongDouble", DataType::LongDouble},
            {"Bool", DataType::Bool},
            {"String", DataType::String},
        };

        if(const auto it = lookup.find(str); it != lookup.end())
            return it->second;

        return DataType::Invalid;
    }
    inline std::string_view DataTypeToString(DataType type)
    {
        switch(type)
        {
        case DataType::Int: return "Int";
        case DataType::UInt: return "UInt";
        case DataType::Long: return "Long";
        case DataType::ULong: return "ULong";
        case DataType::LongLong: return "LongLong";
        case DataType::ULongLong: return "ULongLong";
        case DataType::Short: return "Short";
        case DataType::UShort: return "UShort";
        case DataType::Char: return "Char";
        case DataType::UChar: return "UChar";
        case DataType::Float: return "Float";
        case DataType::Double: return "Double";
        case DataType::LongDouble: return "LongDouble";
        case DataType::Bool: return "Bool";
        case DataType::String: return "String";
        default: return "Invalid";
        }
    }

    struct ConfigFile
    {
    public:
        ConfigFile(std::filesystem::path configFilePath);
        ~ConfigFile() = default;

        ConfigFile(const ConfigFile& other) = default;
        ConfigFile(ConfigFile&& other) noexcept = default;
        ConfigFile& operator=(const ConfigFile& other) = default;
        ConfigFile& operator=(ConfigFile&& other) noexcept = default;

        bool operator==(const ConfigFile& other) const;

        void WriteVariable(Variable variable);

        void DeleteVariable(const std::string_view& path);

        static std::vector<Variable> ExtractVariablesFromString(const std::string_view& configSource);
        static std::vector<Variable> ExtractVariablesFromFile(const std::filesystem::path& configFilePath);

        /// @brief This method opens file, e.g. operates with disk.
        /// @return Gets full raw source of the config file
        std::string GetConfigFileSource() const;
        const std::filesystem::path& GetPath() const;

        /// @param variablePath The namespace path to the variable. Syntax: namespace/namespace/variablePath or variablePath if there are no any namespaces.
        /// @returns The variable that is saved in m_Variables.
        const Variable& GetVariable(const std::string_view& variablePath) const;
        const std::vector<Variable>& GetVariables() const;

    public:
        struct Parser
        {
        public:
            using index = int;

            static constexpr char SCOPE_OPEN = GE_SCOPE_OPEN;
            static constexpr char SCOPE_CLOSE = GE_SCOPE_CLOSE;
            static constexpr std::string_view NAMESPACE_KEYWORD = GE_NAMESPACE_KEYWORD;
            static constexpr char VARIABLE_VALUE_SCOPE = GE_VARIABLE_VALUE_SCOPE;
            static constexpr std::string_view COMMENT_SCOPE_LINE = GE_COMMENT_SCOPE_LINE;
            static constexpr char SPECIAL_CHAR_SIGN = GE_SPECIAL_CHAR_SIGN;
            static constexpr std::string_view SPECIAL_CHARS = GE_SPECIAL_CHARS;
            static constexpr char NEWLINE = GE_NEWLINE;
            static constexpr char WHITESPACE = GE_WHITESPACE;
            static constexpr char PATH_SEPARATOR = GE_PATH_SEPARATOR;
            static constexpr char EQUALS = GE_EQUALS;
            static constexpr char SEMICOLON = GE_SEMICOLON;

            struct StringRange
            {
                StringRange(index begin = std::string::npos, index end = std::string::npos);

                template<String ReturnStringType, String ParamStringType = ReturnStringType>
                ReturnStringType GetSubstring(const ParamStringType& string) const
                {
                    if(begin == std::string::npos || end == std::string::npos)
                        return ReturnStringType{};
                    if(end > 0 && begin <= end)
                        return ReturnStringType{ string.data() + begin, string.data() + end + 1 };
                    else
                        throw std::exception{ "invalid begin or end indices" };
                }

                index begin;
                index end;

                friend StringRange operator+(const StringRange& lhs, const StringRange& rhs);
                friend StringRange operator+(const StringRange& lhs, index rhs);

                auto operator<=>(const StringRange&) const = default;
            };

            struct NamespaceIndicesInfo
            {
                StringRange keyword;
                StringRange name;
                StringRange scope;

                friend NamespaceIndicesInfo operator+(const NamespaceIndicesInfo& lhs, const NamespaceIndicesInfo& rhs);
                friend NamespaceIndicesInfo operator+(const NamespaceIndicesInfo& lhs, index rhs);

                auto operator<=>(const NamespaceIndicesInfo&) const = default;
            };
            struct VariableInfo
            {
                StringRange type;
                index equals;
                StringRange name;
                StringRange value;
                index semicolon;
                bool isArray;

                friend VariableInfo operator+(const VariableInfo& lhs, const VariableInfo& rhs);
                friend VariableInfo operator+(const VariableInfo& lhs, index rhs);

                auto operator<=>(const VariableInfo&) const = default;
            };

            enum class ParsingDataType : uint8_t
            {
                Invalid = 0,
                Comment,
                VariableValue,
                Variable,
                Namespace
            };

            static bool IsFullSubstringSame(const std::string_view& string, index stringIndexPosition, const std::string_view& substring);

            static void ProcessNamespace(std::vector<Variable>& variables, std::string& path, const std::string_view& scope);

            static NamespaceIndicesInfo ReceiveNamespaceInfo(const std::string_view& scope, const index& namespaceKeywordBeginIndex);
            //if the variable is empty e.g. "", VariableIndicesInfo::value indices will be equal to std::string::npos
            static VariableInfo ReceiveVariableInfo(const std::string_view& scope, const index& variableTypeBeginIndex);
            //throws an error if nothing is found
            static NamespaceIndicesInfo FindNamespace(const std::string_view& scope, const std::string_view& path);
            //throws an error if nothing is found
            static VariableInfo FindVariableInfo(const std::string_view& scope, const std::string_view& path);
            static Variable FindVariable(const std::string_view& scope, const std::string_view& path);

            //returns the scope after inserting
            static std::string WriteVariable(std::string scope, const Variable& variable);
            //may throw an exception
            static std::string DeleteNamespace(std::string scope, const std::string_view& path);
            //may throw an exception
            static std::string DeleteVariable(std::string scope, const std::string_view& path);

        private:
            //this func basically needs an outer index of the scope, and those bools. This func finds out whether current char is about namespace or variable or other shit
            static ParsingDataType DetermineParsingDataType(const std::string_view& scope, index currentCharIndex, bool& wasCommentScopeClosed, bool& wasValueScopeClosed);
            static bool IsArray(const std::string_view& variableValue);
        };
    private:
        std::filesystem::path m_Path;

        std::vector<Variable> m_Variables;
    };

    struct Variable
    {
    public:
        using Bool = char;
        template<typename T>
        using Array = std::vector<T>;
    public:
        Variable(std::string variablePath, std::string value = "", DataType type = DataType::Invalid, bool isArray = false);
        ~Variable() = default;

        Variable(const Variable& other) = default;
        Variable(Variable&& other) noexcept = default;
        Variable& operator=(const Variable& other) = default;
        Variable& operator=(Variable&& other) noexcept = default;

        bool operator==(const Variable& other) const;

        static bool IsValidVariableChar(char ch);

        template<typename T>
        T GetValue() const
        {
            throw std::exception{ "The type is invalid" };
        }

        template<IsNumber Numeral>
        Numeral GetValue() const
        {
            if(!IsNumeral())
                throw std::invalid_argument("The variable's type is not numeral.");

            return StringToNumber<Numeral>(m_Value);
        }
        template<>
        bool GetValue() const
        {
            if(m_Type != DataType::Bool)
                throw std::invalid_argument{ "Wrong variable type" };

            return StringToBool(m_Value);
        }
        template<>
        const std::string& GetValue() const
        {
            if(m_Type != DataType::String)
                throw std::invalid_argument{ "Wrong variable type" };

            return m_Value;
        }

        template<typename T>
        Array<T> GetArrayValue() const
        {
            throw std::exception{ "The type is invalid" };
        }
        //yeah the code is dogshit, but I'm too lazy to incapsulate
        template<IsNumber Numeral>
        Array<Numeral> GetArrayValue() const
        {
            using index = ConfigFile::Parser::index;

            if((!IsNumeral() || m_Type == DataType::Bool) && !m_IsArray)
                throw std::invalid_argument{ "Wrong variable type" };

            Array<Numeral> result;

            index valueBegin = 0;
            bool valueScopeClosed = true;
            //parsing
            if(m_Type == DataType::Bool)
                for(index i = 0; i < m_Value.size(); i++)
                {
                    char currentChar = m_Value[i];

                    if(currentChar == ConfigFile::Parser::VARIABLE_VALUE_SCOPE)
                    {
                        size_t specialCharSignCount = 0;
                        for(index j = i - 1; j > 0; j--)
                        {
                            char _currentChar = m_Value[j];
                            if(_currentChar == ConfigFile::Parser::SPECIAL_CHAR_SIGN)
                                specialCharSignCount++;
                            else
                                break;
                        }

                        if(specialCharSignCount % 2 == 0)
                            valueScopeClosed = !valueScopeClosed;

                        if(!valueScopeClosed)
                            valueBegin = i;
                        else
                            result.push_back(StringToBool({ m_Value.cbegin() + valueBegin + 1, m_Value.cbegin() + i }));
                    }
                }
            else
                for(index i = 0; i < m_Value.size(); i++)
                {
                    char currentChar = m_Value[i];

                    if(currentChar == ConfigFile::Parser::VARIABLE_VALUE_SCOPE)
                    {
                        size_t specialCharSignCount = 0;
                        for(index j = i - 1; j > 0; j--)
                        {
                            char _currentChar = m_Value[j];
                            if(_currentChar == ConfigFile::Parser::SPECIAL_CHAR_SIGN)
                                specialCharSignCount++;
                            else
                                break;
                        }

                        if(specialCharSignCount % 2 == 0)
                            valueScopeClosed = !valueScopeClosed;

                        if(!valueScopeClosed)
                            valueBegin = i;
                        else
                            result.push_back(StringToNumber<Numeral>({ m_Value.cbegin() + valueBegin + 1, m_Value.cbegin() + i }));
                    }
                }

            return result;
        }
        template<>
        Array<std::string> GetArrayValue() const
        {
            using index = ConfigFile::Parser::index;

            if(m_Type != DataType::String && !m_IsArray)
                throw std::invalid_argument{ "Wrong variable type" };

            Array<std::string> result;

            index valueBegin = 0;
            bool valueScopeClosed = true;
            //parsing
            for(index i = 0; i < m_Value.size(); i++)
            {
                char currentChar = m_Value[i];

                if(currentChar == ConfigFile::Parser::VARIABLE_VALUE_SCOPE)
                {
                    size_t specialCharSignCount = 0;
                    for(index j = i - 1; j > 0; j--)
                    {
                        char _currentChar = m_Value[j];
                        if(_currentChar == ConfigFile::Parser::SPECIAL_CHAR_SIGN)
                            specialCharSignCount++;
                        else
                            break;
                    }

                    if(specialCharSignCount % 2 == 0)
                    {
                        valueScopeClosed = !valueScopeClosed;

                        if(!valueScopeClosed)
                            valueBegin = i;
                        else
                        {
                            std::string value{m_Value.cbegin() + valueBegin + 1, m_Value.cbegin() + i};

                            for(char specialChar : ConfigFile::Parser::SPECIAL_CHARS)
                                for(index j = 0; j < value.size(); j++)
                                    if(j > 0 && value[j] == specialChar && value[j - 1] == ConfigFile::Parser::SPECIAL_CHAR_SIGN)
                                        value.erase(j - 1, 1);

                            result.push_back(std::move(value));
                        }
                    }
                }
            }

            return result;
        }

        [[nodiscard]]
        bool IsNumeral() const;

        const std::string& GetRawValue() const;
        DataType GetType() const noexcept;
        std::string_view GetName() const;
        const std::string& GetPath() const;
        bool IsArray() const;

    private:
        std::string m_Path;
        DataType m_Type;
        std::string m_Value;
        bool m_IsArray : 1;
    };
}

namespace GuelderResourcesManager
{
    template<typename Char>
    constexpr auto GetPOpen()
    {
        if constexpr(std::is_same_v<Char, char>)
            return _popen;
        else
            return _wpopen;
    }
    template<typename Char>
    constexpr auto GetFGets()
    {
        if constexpr(std::is_same_v<Char, char>)
            return fgets;
        else
            return fgetws;
    }

    class ResourcesManager
    {
    public:
        ResourcesManager(std::filesystem::path executablePath = "");
        ~ResourcesManager() = default;

        ResourcesManager(const ResourcesManager& other) = default;
        ResourcesManager(ResourcesManager&& other) noexcept = default;
        ResourcesManager& operator=(const ResourcesManager& other) = default;
        ResourcesManager& operator=(ResourcesManager&& other) noexcept = default;

        //if outputs == std::numeric_limits<uint32_t>::max() then all outputs will be received
        template<typename InChar = char, typename OutChar = InChar, uint32_t bufferSize = 128, String String = std::string>
        static std::vector<std::basic_string<OutChar>> ExecuteCommand(const String& command, uint32_t outputs = std::numeric_limits<uint32_t>::max())
        {
            using OutString = std::basic_string<OutChar>;

            auto PipeOpen = GetPOpen<InChar>();
            auto FGets = GetFGets<OutChar>();

            std::array<OutChar, bufferSize> buffer{};
            std::vector<OutString> result;

            if(outputs != std::numeric_limits<uint32_t>::max())
                result.reserve(outputs);

            const InChar* mode;
            if constexpr(std::is_same_v<InChar, char>)
                mode = "r";
            else
                mode = L"r";

            const std::unique_ptr<FILE, decltype(&_pclose)> cmd(PipeOpen(command.data(), mode), _pclose);

            if(!cmd)
                throw std::exception("Failed to execute command.");

            OutString output;

            while(outputs > 0 && FGets(buffer.data(), buffer.size(), cmd.get()) != nullptr)
            {
                auto newlinePos = std::find(buffer.begin(), buffer.end(), '\n');
                output.append(buffer.begin(), newlinePos - 1 * (newlinePos == buffer.end() ? 1 : 0));//cuz buffer.end() - 1 is '\0'

                if(newlinePos != buffer.end())
                {
                    result.push_back(std::move(output));
                    output.clear();

                    outputs--;
                }
            }

            return result;
        }

        static std::string ReceiveFileSource(const std::filesystem::path& filePath);

        static void WriteToFile(const std::filesystem::path& filePath, const std::string_view& content);
        //almost useless, use better first Rea
        static void WriteToFile(const std::filesystem::path& filePath, ConfigFile::Parser::index index, const std::string_view& content);

        std::filesystem::path GetFullPathToRelativeFile(const std::filesystem::path& relativePath) const;

        const std::filesystem::path& GetPath() const;

    private:
        std::filesystem::path m_Path;
    };
}