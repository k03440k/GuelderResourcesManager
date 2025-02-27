#include "../include/GuelderResourcesManager.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <format>
#include <string>
#include <string_view>

namespace GuelderResourcesManager
{
    ResourcesManager::ResourcesManager(const std::string_view& executablePath, const std::string_view& resourcesFolderPath, const std::string_view& configPath)
        : m_Path(executablePath.substr(0, executablePath.find_last_of("/\\"))), m_ResourcesFolderPath(resourcesFolderPath), m_ConfigPath(configPath)
    {
        m_Vars = GetVariablesFromFile(GetRelativeFileSource(std::format("{}/{}", resourcesFolderPath, configPath)));
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
    std::string ResourcesManager::GetRelativeFileSource(const std::string_view& relativeFilePath) const
    {
        return GetFileSource(m_Path + "/" + relativeFilePath.data());
    }
    const Variable& ResourcesManager::GetVariable(const std::string_view& name) const
    {
        const auto found = std::ranges::find_if(m_Vars, [&name](const Variable& var) { return var.GetName() == name; });

        if(found == m_Vars.end())
            throw std::exception(std::format(R"(Failed to find "{}" variable, in "{}/{}")", name, m_Path, m_ConfigPath).c_str());

        return *found;
    }
    std::string ResourcesManager::GetFileSourceByVariable(const std::string_view& name) const
    {
        return GetRelativeFileSource(std::format("{}/{}", m_ResourcesFolderPath, GetVariable(name).GetValue<std::string_view>()).c_str());
    }
    std::string ResourcesManager::GetFullPathToRelativeFile(const std::string_view& relativePath) const
    {
        return {m_Path + "\\" + relativePath.data()};
    }
    std::string ResourcesManager::GetFullPathToRelativeFileByVariable(const std::string_view& varName) const
    {
        return GetFullPathToRelativeFile(std::format("{}/{}", m_ResourcesFolderPath, GetVariable(varName).GetValue<std::string_view>()));
    }
    std::vector<Variable> ResourcesManager::GetVariablesFromFile(const std::string_view& configSource)
    {
        std::vector<Variable> vars;

        const std::regex pattern(R"||(\s*(\w+)\s+(\w+)\s*=\s*"(.+)"\s*;)||");//Var = "...";
        std::match_results<std::string_view::const_iterator> matches;

        auto it = configSource.cbegin();
        std::regex_search(it, configSource.cend(), matches, pattern);

        vars.reserve(matches.size());

        while(std::regex_search(it, configSource.cend(), matches, pattern))
        {
            vars.emplace_back(matches[2].str(), matches[3].str(), StringToDataType(matches[1].str()));

            it = matches.suffix().first;
        }

        return vars;
    }

    const std::vector<Variable>& ResourcesManager::GetVariables() const noexcept
    {
        return m_Vars;
    }
    const std::string& ResourcesManager::GetPath() const
    {
        return m_Path;
    }
    const std::string& ResourcesManager::GetResourcesFolderPath() const
    {
        return m_ResourcesFolderPath;
    }
    const std::string& ResourcesManager::GetConfigPath() const
    {
        return m_ConfigPath;
    }
}