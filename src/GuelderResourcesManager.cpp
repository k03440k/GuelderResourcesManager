#include "../include/GuelderResourcesManager.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <format>
#include <string>
#include <string_view>

namespace GuelderResourcesManager
{
    ResourcesManager::ResourcesManager(const std::string_view& executablePath, const std::string_view& resourcesFolderPath, const std::string_view& configPath)
        : m_Path(executablePath.substr(0, executablePath.find_last_of("/\\"))), m_ResourcesFolderPath(resourcesFolderPath), m_ConfigPath(configPath)
    {
        m_Vars = GetAllResourcesVariables(GetRelativeFileSource(std::format("{}/{}", resourcesFolderPath, configPath)));
    }

    std::string ResourcesManager::GetRelativeFileSource(const std::string_view& relativeFilePath) const
    {
        std::ifstream file;
        file.open(m_Path + "/" + relativeFilePath.data(), std::ios::binary);

        if(!file.is_open())
            throw std::exception(std::format("Failed to open file at location: {}\\{}", m_Path, relativeFilePath).c_str());

        std::stringstream source;//istringstream or stringstream?
        source << file.rdbuf();

        file.close();

        return source.str();
    }
    std::string ResourcesManager::GetFileSource(const std::string_view& filePath)
    {
        std::ifstream file;
        file.open(filePath.data(), std::ios::binary);

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

        if(found == m_Vars.end())
            throw std::exception(std::format(R"(Failed to find "{}" variable, in "{}\\{}")", name, m_Path, m_ConfigPath).c_str());

        return found->second;
    }
    std::string ResourcesManager::GetResourcesVariableFileContent(const std::string_view& name) const
    {
        return GetRelativeFileSource(std::format("{}\\{}", m_ResourcesFolderPath, GetResourcesVariableContent(name)).c_str());
    }
    std::string ResourcesManager::GetFullPathToRelativeFile(const std::string_view& relativePath) const
    {
        std::string filePath(m_Path);
        filePath.append("\\");
        filePath.append(relativePath);
        return filePath;
    }
    std::string ResourcesManager::GetFullPathToRelativeFileViaVar(const std::string_view& varName) const
    {
        return GetFullPathToRelativeFile(std::format("{}\\{}", m_ResourcesFolderPath, GetResourcesVariableContent(varName)));
    }
    ResourcesManager::vars ResourcesManager::GetAllResourcesVariables(const std::string_view& resSource)
    {
        vars varsMap;

        const std::regex pattern(R"||(\s*var\s+(\w+)\s*=\s*"(.+)"\s*;)||");//var = "...";
        std::match_results<std::string_view::const_iterator> matches;

        auto it = resSource.cbegin();
        while(std::regex_search(it, resSource.cend(), matches, pattern))
        {
            varsMap[matches[1]] = matches[2];
            it = matches.suffix().first;
        }

        return varsMap;
    }
    const ResourcesManager::vars& ResourcesManager::GetResourcesVariables() const noexcept
    {
        return m_Vars;
    }
    std::string ResourcesManager::GetPath() const
    {
        return m_Path;
    }
    std::string ResourcesManager::GetResourcesFolderPath() const
    {
        return m_ResourcesFolderPath;
    }
    std::string ResourcesManager::GetConfigPath() const
    {
        return m_ConfigPath;
    }
}