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

#ifdef WIN32
#include <Windows.h>
#endif

//idk #define NOMINMAX doesn't work
//#undef max

//converters
namespace GuelderResourcesManager
{
    template<typename T>
    concept IsNumber = std::integral<T> || std::floating_point<T>;

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
    enum class DataType : uint8_t
    {
        Invalid = 0,
        Var,
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
        String,
        WString
    };

    inline DataType StringToDataType(const std::string_view& str)
    {
        static const std::unordered_map<std::string_view, DataType> lookup =
        {
            {"Invalid", DataType::Invalid},
            {"Var", DataType::Var},
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
            {"WString", DataType::WString}
        };

        if(const auto it = lookup.find(str); it != lookup.end())
            return it->second;

        return DataType::Invalid;
    }
    inline std::string_view DataTypeToString(DataType type)
    {
        switch(type)
        {
        case DataType::Var: return "Var";
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
        case DataType::WString: return "WString";
        default: return "Invalid";
        }
    }

    struct Variable
    {
    public:
        Variable(const std::string_view& name, const std::string_view& value = "", const DataType& type = DataType::Invalid)
            : m_Name(name), m_Type(type), m_Value(value) {}

        bool operator==(const Variable& other) const
        {
            return m_Type == other.m_Type && m_Value == other.m_Value;
        }

        template<typename T>
        T GetValue() const
        {
            throw std::exception("Failed to find any suitable method GetValue<T>(), probably the GetValue<T>() was called with invalid template param.");
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
                throw std::invalid_argument("The variable's type is not bool.");

            return StringToBool(m_Value);
        }

        /**
         * \brief WARNING: this method doesn't check whether the m_Type == DataType::String
         */
        template<>
        const std::string& GetValue() const
        {
            return m_Value;
        }
        template<>
        std::string_view GetValue() const
        {
            return m_Value;
        }
        template<>
        std::wstring GetValue() const
        {
            if(m_Type != DataType::WString)
                throw std::invalid_argument("The variable's type is not std::wstring.");

            return StringToWString(m_Value);
        }

        bool SameType(const Variable& other) const
        {
            return m_Type == other.m_Type;
        }
        bool SameType(const DataType& type) const
        {
            return m_Type == type;
        }

        bool IsNumeral() const
        {
            switch(m_Type)
            {
            case DataType::Int:
            case DataType::UInt:
            case DataType::Long:
            case DataType::ULong:
            case DataType::LongLong:
            case DataType::ULongLong:
            case DataType::Short:
            case DataType::UShort:
            case DataType::Char:
            case DataType::UChar:
            case DataType::Float:
            case DataType::Double:
            case DataType::LongDouble:
                return true;
            default:
                return false;
            }
        }

        DataType GetType() const noexcept { return m_Type; }
        const std::string& GetName() const noexcept { return m_Name; }

    private:
        std::string m_Name;
        DataType m_Type;
        std::string m_Value;
    };
}

namespace GuelderResourcesManager
{
    template<typename T>
    concept IsString = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>;

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
        ResourcesManager(const std::string_view& executablePath, const std::string_view& resourcesFolderPath = "Resources", const std::string_view& configPath = "Config.txt");
        ~ResourcesManager() = default;

        ResourcesManager(const ResourcesManager& other) = default;
        ResourcesManager(ResourcesManager&& other) noexcept = default;
        ResourcesManager& operator=(const ResourcesManager& other) = default;
        ResourcesManager& operator=(ResourcesManager&& other) noexcept = default;

        //if outputs == std::numeric_limits<uint32_t>::max() then all outputs will be received
        template<typename InChar = char, typename OutChar = InChar, uint32_t bufferSize = 128, IsString String = std::string>
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
                output.append(buffer.begin(), newlinePos - 1 * (newlinePos == buffer.end() ? 1 : 0 ));//cuz buffer.end() - 1 is '\0'

                if(newlinePos != buffer.end())
                {
                    result.push_back(std::move(output));
                    output.clear();

                    outputs--;
                }
            }

            return result;
        }

        static std::string GetFileSource(const std::string_view& filePath);
        std::string GetRelativeFileSource(const std::string_view& relativeFilePath) const;
        /*
         *@brief Finds variable content, for example: "var i = "staff";" then it will return "staff"
        */
        const Variable& GetVariable(const std::string_view& name) const;
        /*
         *@brief Finds file content by variable content in resources.txt
        */
        std::string GetFileSourceByVariable(const std::string_view& name) const;

        std::string GetFullPathToRelativeFile(const std::string_view& relativePath) const;
        std::string GetFullPathToRelativeFileByVariable(const std::string_view& varName) const;

        static std::vector<Variable> GetVariablesFromFile(const std::string_view& configSource);

        const std::vector<Variable>& GetVariables() const noexcept;

        const std::string& GetPath() const;
        const std::string& GetResourcesFolderPath() const;
        const std::string& GetConfigPath() const;

    private:
        std::vector<Variable> m_Vars;

        std::string m_Path;
        std::string m_ResourcesFolderPath;
        std::string m_ConfigPath;
    };
}