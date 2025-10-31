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
#include <expected>
#include <future>

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

#ifndef GE_TAB
#define GE_TAB '\t'
#endif

#ifndef GE_SCOPE_DISTANCE
#define GE_SCOPE_DISTANCE "    "
#endif

#ifndef GE_ARRAY_ITEMS_SEPARATOR
#define GE_ARRAY_ITEMS_SEPARATOR ','
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
            static constexpr char TAB = GE_TAB;
            static constexpr std::string_view SCOPE_DISTANCE = GE_SCOPE_DISTANCE;
            static constexpr char ARRAY_ITEMS_SEPARATOR = GE_ARRAY_ITEMS_SEPARATOR;

            struct StringRange
            {
                StringRange(index begin = std::string::npos, index end = std::string::npos);

                template<String ReturnStringType, String ParamStringType = ReturnStringType>
                ReturnStringType GetSubstring(const ParamStringType& string) const
                {
                    if(begin == std::string::npos || end == std::string::npos)
                        return ReturnStringType{};
                    if(begin >= 0 && end >= 0 && begin <= end)
                        return ReturnStringType{ string.data() + begin, string.data() + end + 1 };
                    else
                        throw std::exception{ "invalid begin or end indices" };
                }

                bool IsValid() const noexcept;

                index begin;
                index end;

                friend StringRange operator+(const StringRange& lhs, const StringRange& rhs);
                friend StringRange operator+(const StringRange& lhs, index rhs);
                StringRange& operator+=(index offset);
                StringRange& operator-=(index offset);

                auto operator<=>(const StringRange&) const = default;
            };

            struct NamespaceIndicesInfo
            {
                StringRange keyword;
                StringRange name;
                StringRange scope;

                friend NamespaceIndicesInfo operator+(const NamespaceIndicesInfo& lhs, const NamespaceIndicesInfo& rhs);
                friend NamespaceIndicesInfo operator+(const NamespaceIndicesInfo& lhs, index rhs);
                NamespaceIndicesInfo& operator+=(index offset);

                auto operator<=>(const NamespaceIndicesInfo&) const = default;
            };
            struct VariableIndicesInfo
            {
                StringRange type;
                index equals;
                StringRange name;
                StringRange value;
                index semicolon;
                bool isArray;

                friend VariableIndicesInfo operator+(const VariableIndicesInfo& lhs, const VariableIndicesInfo& rhs);
                friend VariableIndicesInfo operator+(const VariableIndicesInfo& lhs, index rhs);
                VariableIndicesInfo& operator+=(index offset);

                auto operator<=>(const VariableIndicesInfo&) const = default;
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
            static VariableIndicesInfo ReceiveVariableInfo(const std::string_view& scope, const index& variableTypeBeginIndex);
            //throws an error if nothing is found
            static NamespaceIndicesInfo FindNamespace(const std::string_view& scope, const std::string_view& path);
            //throws an error if nothing is found
            static VariableIndicesInfo FindVariableInfo(const std::string_view& scope, const std::string_view& path);
            static Variable FindVariable(const std::string_view& scope, const std::string_view& path);

            //returns the scope after inserting
            static std::string WriteVariable(std::string scope, const Variable& variable, StringRange scopeRange = {});
            static std::string WriteVariables(std::string scope, const std::vector<Variable>& variables, StringRange scopeRange = {});
            /// 
            /// @param scope 
            /// @param variable 
            /// @param after path to variable or namespace
            /// @return 
            static std::string WriteVariableAfter(std::string scope, const Variable& variable, const std::string_view& after, StringRange scopeRange = {});
            /// 
            /// @param scope 
            /// @param variable 
            /// @param after path to variable or namespace
            /// @return 
            static std::string WriteVariablesAfter(std::string scope, const std::vector<Variable>& variables, const std::string_view& after, StringRange scopeRange = {});
            /// 
            /// @param scope 
            /// @param variable 
            /// @param before path to variable or namespace
            /// @return 
            static std::string WriteVariableBefore(std::string scope, const Variable& variable, const std::string_view& before, StringRange scopeRange = {});
            /// 
            /// @param scope 
            /// @param variable 
            /// @param before path to variable or namespace
            /// @return 
            static std::string WriteVariablesBefore(std::string scope, const std::vector<Variable>& variables, const std::string_view& before, StringRange scopeRange = {});

            //may throw an exception
            static std::string DeleteNamespace(std::string scope, const std::string_view& path);
            //may throw an exception
            static std::string DeleteVariable(std::string scope, const std::string_view& path);
            //makes beauty
            static std::string FormatScope(std::string scope, StringRange range = {});

            //makes from <f""fff> -> <f\"\"fff> so it is usable for config and variable value string.
            //This assumes that variableValue is not already populated with SPECIAL_CHAR_SIGH, so if variableValue == "\\", then output will be "\\\\", but NOT "\\"
            static std::string AddSpecialChars(std::string variableValue);

            static size_t DetermineReserveSize(const Variable& variable);

            //returns <"1", "2", "3"> with NO BRACES
            template<typename T>
                requires
                requires(std::stringstream& ss, T value) { ss << value; }
            &&
                (
                    requires(T value) { std::to_string(value); }
            ||
                String<T>
                )
                static std::string CreateArrayVariableValue(const std::vector<T>& array)
            {
                //TODO: maybe reserve?
                std::stringstream result;

                const size_t size = array.size();
                size_t i = 0;

                for(const auto& item : array)
                {
                    i++;

                    result << VARIABLE_VALUE_SCOPE << std::to_string(item) << VARIABLE_VALUE_SCOPE;

                    if(i < size)
                        result << ARRAY_ITEMS_SEPARATOR;
                }

                return result.str();
            }

        private:
            //this func basically needs an outer index of the scope, and those bools. This func finds out whether current char is about namespace or variable or other shit
            static ParsingDataType DetermineParsingDataType(const std::string_view& scope, index currentCharIndex, bool& wasCommentScopeClosed, bool& wasValueScopeClosed);
            static bool IsArray(const std::string_view& variableValue);

            static index FormatScope(std::string& scope, StringRange& namespaceScope, index scopesOpened, bool wasNewLine = false);

            static StringRange CorrectStringRange(index scopeEnd, StringRange stringRange);
            static StringRange CorrectStringRange(const std::string_view& scope, const StringRange& stringRange);
        };
    public:
        ConfigFile(std::filesystem::path configFilePath, bool createOrOpen = true);
        ~ConfigFile() = default;

        ConfigFile(const ConfigFile& other) = default;
        ConfigFile(ConfigFile&& other) noexcept = default;
        ConfigFile& operator=(const ConfigFile& other) = default;
        ConfigFile& operator=(ConfigFile&& other) noexcept = default;

        bool operator==(const ConfigFile& other) const;

        void Reopen();

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

        //may throw an error
        void Format(const Parser::StringRange& range = {}) const;

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
        template<>
        std::string GetValue() const
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
                            std::string value{ m_Value.cbegin() + valueBegin + 1, m_Value.cbegin() + i };

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
        //this one probably can't be stopped, so I'm making a windows version of this one
        template<typename InChar = char, typename OutChar = InChar, uint32_t bufferSize = 128, String String = std::basic_string<InChar>>
        static std::expected<std::vector<std::basic_string<OutChar>>, std::string> ExecuteCommand(const String& command, uint32_t outputs = std::numeric_limits<uint32_t>::max())
        {
            using OutString = std::basic_string<OutChar>;

            auto PipeOpen = GetPOpen<InChar>();
            auto FGets = GetFGets<OutChar>();

            const InChar* mode;
            if constexpr(std::is_same_v<InChar, char>)
                mode = "r";
            else
                mode = L"r";

            const std::unique_ptr<FILE, decltype(&_pclose)> cmd(PipeOpen(command.data(), mode), _pclose);

            if(!cmd)
                return std::unexpected{ "Failed to execute command." };

            std::array<OutChar, bufferSize> buffer{};
            std::vector<OutString> result;

            if(outputs != std::numeric_limits<uint32_t>::max())
                result.reserve(outputs);

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

#ifdef WIN32
        struct Handle;
        struct ProcessInfo;
        //if outputs == std::numeric_limits<uint32_t>::max() then all outputs will be received
        //if exception occurs, it throws a DWORD. if pipe ends with bad exit code then it returns std::unexpected
        template<typename Char = char, uint32_t bufferSize = 128, String String = std::basic_string<Char>>
        static std::expected<std::vector<std::string>, int> ExecuteCommandWin(const String& command, uint32_t outputs = std::numeric_limits<uint32_t>::max(), DWORD msDelay = 100)
        {
            using OutString = std::string;

            ProcessInfo processInfo;
            Handle readPipe;

            {
                Handle writePipe;

                SECURITY_ATTRIBUTES securityAttributes{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
                if(!CreatePipe(&readPipe.handle, &writePipe.handle, &securityAttributes, 0))
                    throw GetLastError();

                SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

                auto startupInfo = GetSTARTUPINFO<Char>();
                startupInfo.cb = sizeof(startupInfo);
                startupInfo.dwFlags = STARTF_USESTDHANDLES;
                startupInfo.hStdOutput = writePipe;
                startupInfo.hStdError = writePipe;
                startupInfo.hStdInput = nullptr;

                if constexpr(std::is_same_v<Char, wchar_t>)
                {
                    if(!CreateProcessW(nullptr, const_cast<wchar_t*>(command.c_str()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo.processInfo))
                        throw GetLastError();
                }
                else if constexpr(std::is_same_v<Char, char>)
                {
                    if(!CreateProcessA(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo.processInfo))
                        throw GetLastError();
                }
                else
                    static_assert(0, "Unsupported type for char");
            }

            std::array<OutString::value_type, bufferSize> buffer{};
            std::vector<OutString> result;

            if(outputs != std::numeric_limits<uint32_t>::max())
                result.reserve(outputs);

            OutString output;
            DWORD bytesRead;

            while(outputs > 0)
            {
                if(!ReadFile(readPipe, buffer.data(), buffer.size() - 1, &bytesRead, nullptr) || bytesRead == 0)
                {
                    DWORD status = WaitForSingleObject(processInfo.processInfo.hProcess, msDelay);
                    if(status == WAIT_TIMEOUT)
                        continue;
                    else
                        break;
                }

                buffer[buffer.size() - 1] = '\0';

                auto newlinePos = std::find(buffer.begin(), buffer.end(), '\n');
                output.append(buffer.begin(), newlinePos - 1 * (newlinePos == buffer.end() ? 1 : 0));//cuz buffer.end() - 1 is '\0'

                if(newlinePos != buffer.end())
                {
                    result.push_back(std::move(output));
                    output.clear();

                    outputs--;
                }
            }

            DWORD exitCode = 0;
            if(GetExitCodeProcess(processInfo.processInfo.hProcess, &exitCode))
            {
                if(exitCode != STILL_ACTIVE && exitCode != 0)
                    return std::unexpected(exitCode);
            }
            else
                throw GetLastError();

            return result;
        }

        struct ProcessAsync;

        //if outputs == std::numeric_limits<uint32_t>::max() then all outputs will be received
        //if exception occurs, it throws a DWORD. if pipe ends with bad exit code then it returns std::unexpected
        template<typename Char = char, uint32_t bufferSize = 128, String String = std::basic_string<Char>>
        static ProcessAsync ExecuteCommandWinAsync(const String& command, uint32_t outputs = std::numeric_limits<uint32_t>::max(), DWORD msDelay = 100)
        {
            using OutString = std::string;

            ProcessInfo processInfo;
            Handle readPipe;

            {
                Handle writePipe;

                SECURITY_ATTRIBUTES securityAttributes{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
                if(!CreatePipe(&readPipe.handle, &writePipe.handle, &securityAttributes, 0))
                    throw GetLastError();

                SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

                auto startupInfo = GetSTARTUPINFO<Char>();
                startupInfo.cb = sizeof(startupInfo);
                startupInfo.dwFlags = STARTF_USESTDHANDLES;
                startupInfo.hStdOutput = writePipe;
                startupInfo.hStdError = writePipe;
                startupInfo.hStdInput = nullptr;

                if constexpr(std::is_same_v<Char, wchar_t>)
                {
                    if(!CreateProcessW(nullptr, const_cast<wchar_t*>(command.c_str()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo.processInfo))
                        throw GetLastError();
                }
                else if constexpr(std::is_same_v<Char, char>)
                {
                    if(!CreateProcessA(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo.processInfo))
                        throw GetLastError();
                }
                else
                    static_assert(0, "Unsupported type for char");
            }

            auto processInfoTmp = processInfo.processInfo;
            processInfo.processInfo.hProcess = nullptr;
            processInfo.processInfo.hThread = nullptr;

            return
            ProcessAsync{
                std::async([outputs, msDelay, _processInfo = std::move(processInfo), processInfoTmp, _readPipe = std::move(readPipe)] mutable -> std::expected<std::vector<std::string>, int>
                {
                    _processInfo = processInfoTmp;

                    std::array<OutString::value_type, bufferSize> buffer{};
                    std::vector<OutString> result;

                    if(outputs != std::numeric_limits<uint32_t>::max())
                        result.reserve(outputs);

                    OutString output;
                    DWORD bytesRead;

                    while(outputs > 0)
                    {
                        if(!ReadFile(_readPipe, buffer.data(), buffer.size() - 1, &bytesRead, nullptr) || bytesRead == 0)
                        {
                            DWORD status = WaitForSingleObject(_processInfo.processInfo.hProcess, msDelay);
                            if(status == WAIT_TIMEOUT)
                                continue;
                            else
                                break;
                        }

                        buffer[buffer.size() - 1] = '\0';

                        auto newlinePos = std::find(buffer.begin(), buffer.end(), '\n');
                        output.append(buffer.begin(), newlinePos - 1 * (newlinePos == buffer.end() ? 1 : 0));//cuz buffer.end() - 1 is '\0'

                        if(newlinePos != buffer.end())
                        {
                            result.push_back(std::move(output));
                            output.clear();

                            outputs--;
                        }
                    }

                    DWORD exitCode = 0;
                    if(GetExitCodeProcess(_processInfo.processInfo.hProcess, &exitCode))
                    {
                        if(exitCode != STILL_ACTIVE && exitCode != 0)
                            return std::unexpected(exitCode);
                    }
                    else
                        throw GetLastError();

                    return result;
                }),
                processInfoTmp
            };
        }

        struct ProcessAsync
        {
            ProcessAsync(std::future<std::expected<std::vector<std::string>, int>>&& future, PROCESS_INFORMATION processInfo)
                : future(std::move(future)), m_ProcessInfo(processInfo) {
            }

            //may throw a DWORD
            void TerminateProcess() const
            {
                if(!::TerminateProcess(m_ProcessInfo.hProcess, 1))
                    throw GetLastError();
            }

            std::future<std::expected<std::vector<std::string>, int>> future;

        private:
            PROCESS_INFORMATION m_ProcessInfo;
        };
#endif

        static std::string ReceiveFileSource(const std::filesystem::path& filePath);

        static void AppendToFile(const std::filesystem::path& filePath, const std::string_view& append);
        static void WriteToFile(const std::filesystem::path& filePath, const std::string_view& content);
        //almost useless, use better first Rea
        static void WriteToFile(const std::filesystem::path& filePath, ConfigFile::Parser::index index, const std::string_view& content);

        std::filesystem::path GetFullPathToRelativeFile(const std::filesystem::path& relativePath) const;

        const std::filesystem::path& GetPath() const;

    private:
        std::filesystem::path m_Path;

#ifdef WIN32
        struct Handle
        {
            Handle(HANDLE other = nullptr)
                : handle(other) {}
            Handle(Handle&& other) noexcept
                : handle(other.handle)
            {
                other.handle = nullptr;
            }
            Handle& operator=(Handle&& other) noexcept
            {
                handle = other.handle;

                other.handle = nullptr;

                return *this;
            }
            Handle& operator=(const HANDLE& other)
            {
                handle = other;

                return *this;
            }
            ~Handle()
            {
                if(handle)
                    CloseHandle(handle);
            }

            operator HANDLE& ()
            {
                return handle;
            }

            HANDLE handle;
        };
        struct ProcessInfo
        {
            ProcessInfo(PROCESS_INFORMATION processInfo = PROCESS_INFORMATION{})
                : processInfo(processInfo) {}
            ProcessInfo(ProcessInfo&& other) noexcept
                : processInfo(other.processInfo)
            {
                other.processInfo = {};
            }
            ProcessInfo& operator=(ProcessInfo&& other) noexcept
            {
                processInfo = other.processInfo;

                other.processInfo = {};

                return *this;
            }
            ProcessInfo& operator=(const PROCESS_INFORMATION& other)
            {
                processInfo = other;

                return *this;
            }
            ~ProcessInfo()
            {
                if(processInfo.hProcess)
                    CloseHandle(processInfo.hProcess);
                if(processInfo.hThread)
                    CloseHandle(processInfo.hThread);
            }

            operator PROCESS_INFORMATION& ()
            {
                return processInfo;
            }

            PROCESS_INFORMATION processInfo;
        };

        template<typename Char>
        static constexpr auto GetSTARTUPINFO()
        {
            if constexpr(std::is_same_v<Char, char>)
                return STARTUPINFOA{};
            else if(std::is_same_v<Char, wchar_t>)
                return STARTUPINFOW{};
        }
#endif
    };
}