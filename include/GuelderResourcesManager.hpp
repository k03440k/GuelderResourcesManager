#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <limits>
#include <memory>
#include <vector>
#include <array>

namespace GuelderResourcesManager
{
    //you can define your own path
#ifdef GE_CUSTOM_RESOURCE_FOLDER_PATH
    constexpr const std::string_view g_ResourcesFolderPath = GE_CUSTOM_RESOURCE_FOLDER_PATH;
#else
    constexpr const std::string_view g_ResourcesFolderPath = "Resources";
#endif

#ifdef GE_CUSTOM_RESOURCE_FILE_PATH
    //relatively to g_ResourcesFolderPath, must be inside the g_ResourcesFolderPath
    constexpr const std::string_view g_ResourcesFilePath = GE_CUSTOM_RESOURCE_FILE_PATH;
#else
    //relatively to g_ResourcesFolderPath, must be inside the g_ResourcesFolderPath
    constexpr const std::string_view g_ResourcesFilePath = "Resources.txt";
#endif

    template<typename T>
    concept IsString = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>;

    template<typename Char>
    constexpr auto GetPOpen()
    {
        if constexpr (std::is_same_v<Char, char>)
            return _popen;
        else
            return _wpopen;
    }
    template<typename Char>
    constexpr auto GetFGets()
    {
        if constexpr (std::is_same_v<Char, char>)
            return fgets;
        else
            return fgetws;
    }

    class ResourcesManager
    {
    public:
        using vars = std::unordered_map<std::string, std::string>;
    public:
        ResourcesManager(const std::string_view& executablePath);
        ~ResourcesManager() = default;

        ResourcesManager(const ResourcesManager& other) = delete;
        ResourcesManager(ResourcesManager&& other) = delete;
        ResourcesManager& operator=(const ResourcesManager& other) = delete;
        ResourcesManager& operator=(ResourcesManager&& other) = delete;

        //if outputs == std::numeric_limits<uint32_t>::max() then all outputs will be received
        template<typename InChar = char, typename OutChar = InChar, IsString String = std::string>
        static std::vector<std::basic_string<OutChar>> ExecuteCommand(const String& command, uint32_t outputs = std::numeric_limits<uint32_t>::max())
        {
            using outString = std::basic_string<OutChar>;

            auto PipeOpen = GetPOpen<InChar>();
            auto FGets = GetFGets<OutChar>();

            std::array<OutChar, 128> buffer{};
            std::vector<outString> result;

            const InChar* mode;
            if constexpr (std::is_same_v<InChar, char>)
                mode = "r";
            else
                mode = L"r";

            const std::unique_ptr<FILE, decltype(&_pclose)> cmd(PipeOpen(command.data(), mode), _pclose);

            if(!cmd)
                throw std::exception("Failed to create std::unique_ptr<FILE, decltype(&pclose)>");

            outString output;

            while(FGets(buffer.data(), buffer.size(), cmd.get()) != nullptr && outputs > 0)
            {
                output.insert(output.end(), buffer.begin(), buffer.end() - (buffer.end() - buffer.begin() > 1 ? 1 : 0));

                if(output.find('\n') != outString::npos)
                {
                    auto it = output.find('\n');

                    if(it != outString::npos)
                        output.erase(it);

                    result.push_back(output);
                    output.clear();

                    outputs--;
                }
            }

            return result;
        }

        //get file string
        std::string GetRelativeFileSource(const std::string_view& relativeFilePath) const;
        static std::string GetFileSource(const std::string_view& filePath);
        /*
         *@brief Finds variable content, for example: "var i = "staff";" then it will return "staff"
        */
        std::string_view GetResourcesVariableContent(const std::string_view& name) const;
        /*
         *@brief Finds file content by variable content in resources.txt
        */
        std::string GetResourcesVariableFileContent(const std::string_view& name) const;
        std::string GetFullPathToRelativeFile(const std::string_view& relativePath) const;
        std::string GetFullPathToRelativeFileViaVar(const std::string_view& varName) const;

        static vars GetAllResourcesVariables(const std::string_view& resSource);

        const vars& GetResourcesVariables() const noexcept;

        const std::string path;

    private:
        //variables from resources.txt
        const vars m_Vars;
    };
}