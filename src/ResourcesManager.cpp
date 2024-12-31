#include "../include/ResourcesManager.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <format>

namespace GuelderResourcesManager
{
    ResourcesManager::ResourcesManager(const std::string_view& executablePath)
        : path(executablePath.substr(0, executablePath.find_last_of("/\\"))), m_Vars(GetAllVariables(GetRelativeFileSource(resourcesPath))) {}

    ResourcesManager::vars ResourcesManager::GetAllVariables(const std::string_view& resSource)
    {
        vars varsMap;

        const std::regex pattern(R"||(\s*var\s+(\w+)\s*=\s*"(.+)"\s*;)||"); // Regular expression pattern
        std::match_results<std::string_view::const_iterator> matches;

        auto it = resSource.cbegin();
        while(std::regex_search(it, resSource.cend(), matches, pattern))
        {
            varsMap[matches[1]] = matches[2];
            it = matches.suffix().first;
        }

        return varsMap;
    }

    std::string ResourcesManager::GetRelativeFileSource(const std::string_view& relativeFilePath) const
    {
        std::ifstream file;
        file.open(path + "/" + relativeFilePath.data(), std::ios::in | std::ios::binary);

        if(!file.is_open())
            throw std::exception(std::format("Failed to open file at location: {}/{}", path, relativeFilePath).c_str());

        std::stringstream source;//istringstream or stringstream?
        source << file.rdbuf();

        file.close();

        return source.str();
    }
    std::string ResourcesManager::GetFileSource(const std::string_view& filePath)
    {
        std::ifstream file;
        file.open(filePath.data(), std::ios::in | std::ios::binary);

        if(!file.is_open())
            throw std::exception(std::format("Failed to open file at location: {}", filePath).c_str());

        std::stringstream source;//istringstream or stringstream?
        source << file.rdbuf();

        file.close();

        return source.str();
    }
    std::string_view ResourcesManager::GetResourcesVariableContent(const std::string_view& name) const
    {
        const auto found = m_Vars.find(name.data());
        //GE_ASSERT(found != m_Vars.end(), "cannot find \"", name, "\" variable, in \"", path, '/', resourcesPath, "\"");
        if(found == m_Vars.end())
            throw std::exception(std::format("Failed to find \"{}\" variable, in \"{}/{}\"", name, path, resourcesPath).c_str());

        return found->second;
    }
    std::string ResourcesManager::GetResourcesVariableFileContent(const std::string_view& name) const
    {
        return GetRelativeFileSource(GetResourcesVariableContent(name));
    }
    std::string ResourcesManager::GetFullPathToRelativeFile(const std::string_view& relativePath) const
    {
        std::string filePath(path);
        filePath.append(relativePath);
        return filePath;
    }
    std::string ResourcesManager::GetFullPathToRelativeFileViaVar(const std::string_view& varName) const
    {
        return GetFullPathToRelativeFile(GetResourcesVariableContent(varName));
    }
    const ResourcesManager::vars& ResourcesManager::GetResourcesVariables() const noexcept
    {
        return m_Vars;
    }
}