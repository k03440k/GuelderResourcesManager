#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <limits>
#include <vector>

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
        static std::vector<std::string> ExecuteCommand(const std::string_view& command, uint32_t outputs = std::numeric_limits<uint32_t>::max());//for some reason std::numeric_limits<uint32_t>::max() causes issues

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