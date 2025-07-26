#include "../include/GuelderResourcesManager.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <format>
#include <string>
#include <string_view>

#include "../../GuelderConsoleLog/include/GuelderConsoleLog.hpp"

//Variable
namespace GuelderResourcesManager
{
    Variable::Variable(std::string variablePath, std::string value, DataType type, bool isArray)
        : m_Path(std::move(variablePath)), m_Type(type), m_Value(std::move(value)), m_IsArray(isArray) {
    }

    bool Variable::operator==(const Variable& other) const
    {
        return m_Type == other.m_Type && m_Value == other.m_Value;
    }

    bool Variable::IsValidVariableChar(char ch)
    {
        return std::isalnum(ch) || ch == '_';
    }

    bool Variable::IsNumeral() const
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

    const std::string& Variable::GetRawValue() const
    {
        return m_Value;
    }
    DataType Variable::GetType() const noexcept
    {
        return m_Type;
    }
    std::string_view Variable::GetName() const
    {
        const size_t lastSlashIndex = m_Path.find_last_of('/');

        return (lastSlashIndex == std::string::npos ? m_Path : std::string_view{ m_Path.begin() + lastSlashIndex + 1, m_Path.end() });
    }
    const std::string& Variable::GetPath() const
    {
        return m_Path;
    }
    bool Variable::IsArray() const
    {
        return m_IsArray;
    }
}
//ConfigFile
namespace GuelderResourcesManager
{
    ConfigFile::ConfigFile(std::filesystem::path configFilePath)
        : m_Path(std::move(configFilePath))
    {
        m_Variables = ExtractVariablesFromFile(m_Path);
    }

    bool ConfigFile::operator==(const ConfigFile& other) const
    {
        return m_Path == other.GetPath();
    }

    void ConfigFile::WriteVariable(Variable variable)
    {
        {
            const auto foundIt = std::ranges::find_if(m_Variables, [&variable](const Variable& v) { return v.GetPath() == variable.GetPath(); });

            if(foundIt != m_Variables.end())
                throw std::invalid_argument{ "Cannot add variable with the already existing path" };
        }

        m_Variables.push_back(std::move(variable));

        const Variable& variableToInsert = m_Variables[m_Variables.size() - 1];

        const std::string configSource = ResourcesManager::ReceiveFileSource(m_Path);

        ResourcesManager::WriteToFile(m_Path, Parser::WriteVariable(configSource, variableToInsert));
    }

    void ConfigFile::DeleteVariable(const std::string_view& path)
    {
        const auto it = std::ranges::find_if(m_Variables, [&path](const Variable& variable) { return variable.GetPath() == path; });

        if(it == m_Variables.end())
            throw std::invalid_argument{ "Failed to find variable" };

        m_Variables.erase(it);

        Parser::DeleteVariable(GetConfigFileSource(), path);
    }

    std::vector<Variable> ConfigFile::ExtractVariablesFromString(const std::string_view& configSource)
    {
        std::string path;

        std::vector<Variable> variables;

        Parser::ProcessNamespace(variables, path, configSource);

        return variables;
    }
    std::vector<Variable> ConfigFile::ExtractVariablesFromFile(const std::filesystem::path& configFilePath)
    {
        return ExtractVariablesFromString(ResourcesManager::ReceiveFileSource(configFilePath));
    }

    std::string ConfigFile::GetConfigFileSource() const
    {
        return ResourcesManager::ReceiveFileSource(m_Path);
    }
    const std::filesystem::path& ConfigFile::GetPath() const
    {
        return m_Path;
    }
    const Variable& ConfigFile::GetVariable(const std::string_view& variablePath) const
    {
        const auto foundIt = std::ranges::find_if(m_Variables, [&variablePath](const Variable& variable) { return variable.GetPath() == variablePath; });

        if(foundIt == m_Variables.end())
            throw std::out_of_range("Failed to find variable with path " + std::string{ variablePath });

        return *foundIt;
    }
    const std::vector<Variable>& ConfigFile::GetVariables() const
    {
        return m_Variables;
    }
}
//ConfigFile::Parser is garbage
namespace GuelderResourcesManager
{
    bool ConfigFile::Parser::IsFullSubstringSame(const std::string_view& string, ConfigFile::Parser::index stringIndexPosition, const std::string_view& substring)
    {
        bool result = true;
        for(index k = 0; stringIndexPosition < string.size() && k < substring.size(); stringIndexPosition++, k++)
            if(string[stringIndexPosition] != substring[k])
            {
                result = false;
                break;
            }

        if(result && stringIndexPosition > 0)
            stringIndexPosition--;

        return result;
    }

    void ConfigFile::Parser::ProcessNamespace(std::vector<Variable>& variables, std::string& path, const std::string_view& scope)
    {
        //omfg the code is such shit

        bool isCommentScopeClosed = true;
        bool isValueScopeClosed = true;

        for(index i = 0; i < scope.size(); i++)
        {
            const ParsingDataType parsingDataType = DetermineParsingDataType(scope, i, isCommentScopeClosed, isValueScopeClosed);

            //ALL indices are INCLUSIVE

            if(parsingDataType == ParsingDataType::Namespace)
            {
                const NamespaceIndicesInfo namespaceInfo = ReceiveNamespaceInfo(scope, i);

                //const std::string_view namespaceName{ scope.cbegin() + namespaceInfo.nameBegin, scope.cbegin() + namespaceInfo.nameEnd };
                //const std::string_view namespaceScope{ scope.cbegin() + namespaceInfo.scopeBegin, scope.cbegin() + namespaceInfo.scopeEnd };
                const std::string_view namespaceName = namespaceInfo.name.GetSubstring<std::string_view>(scope);
                const std::string_view namespaceScope = namespaceInfo.scope.GetSubstring<std::string_view>(scope);

                path += std::format("{}/", namespaceName);

                //recursion
                ProcessNamespace(variables, path, namespaceScope);

                if(std::ranges::count(path, PATH_SEPARATOR) > 1)
                    path.erase(std::find(path.rbegin() + 1, path.rend(), PATH_SEPARATOR).base(), path.rbegin().base());
                else
                    path.clear();

                i = namespaceInfo.scope.end;
            }
            else if(parsingDataType == ParsingDataType::Variable)
            {
                const VariableInfo variableInfo = ReceiveVariableInfo(scope, i);

                //const std::string_view variableType{ scope.cbegin() + variableInfo.typeBegin, scope.cbegin() + variableInfo.typeEnd + 1 };
                //const std::string variableName{ scope.cbegin() + variableInfo.nameBegin, scope.cbegin() + variableInfo.nameEnd };
                //std::string variableValue{ scope.cbegin() + variableInfo.valueBegin + 1, scope.cbegin() + variableInfo.valueEnd };

                const std::string_view variableType = variableInfo.type.GetSubstring<std::string_view>(scope);
                std::string variableName = variableInfo.name.GetSubstring<std::string>(scope);
                const std::string_view variableValueRaw = variableInfo.value.GetSubstring<std::string_view>(scope);
                std::string variableValue{ variableValueRaw };

                if(!variableInfo.isArray)
                    for(char specialChar : SPECIAL_CHARS)
                        for(index j = 0; j < variableValue.size(); j++)
                            if(j > 0 && variableValue[j] == specialChar && variableValue[j - 1] == SPECIAL_CHAR_SIGN)
                                variableValue.erase(j - 1, 1);

                variables.emplace_back(path + std::move(variableName), std::move(variableValue), StringToDataType(variableType), variableInfo.isArray);

                i = variableInfo.semicolon;
            }
        }
    }

    //idk it is better to make code of those two func clearer but how?
    ConfigFile::Parser::NamespaceIndicesInfo ConfigFile::Parser::ReceiveNamespaceInfo(const std::string_view& scope, const index& namespaceKeywordBeginIndex)
    {
        bool isValueScopeClosed = true;
        bool isCommentScopeClosed = true;

        const index namespaceKeywordEndIndex = namespaceKeywordBeginIndex + NAMESPACE_KEYWORD.size() - 1;

        index namespaceNameBeginIndex = namespaceKeywordEndIndex + 1;

        //
        for(bool isCommented = false; namespaceNameBeginIndex < scope.size(); namespaceNameBeginIndex++)
        {
            char currentChar = scope[namespaceNameBeginIndex];

            if(isCommented && currentChar == NEWLINE)
                isCommented = false;
            else if(!isCommented)
            {
                if(IsFullSubstringSame(scope, namespaceNameBeginIndex, COMMENT_SCOPE_LINE))
                    isCommented = true;
                else if(Variable::IsValidVariableChar(currentChar))
                    break;
            }
        }

        index namespaceNameEndIndex = namespaceNameBeginIndex + 1;//

        index namespaceScopeBeginIndex = namespaceNameEndIndex + 1;

        //
        for(bool isCommented = false; namespaceScopeBeginIndex < scope.size(); namespaceScopeBeginIndex++)
        {
            char currentChar = scope[namespaceScopeBeginIndex];

            if(isCommented && currentChar == NEWLINE)
                isCommented = false;
            else if(!isCommented)
            {
                if(IsFullSubstringSame(scope, namespaceScopeBeginIndex, COMMENT_SCOPE_LINE))
                    isCommented = true;
                else if(Variable::IsValidVariableChar(currentChar))
                    namespaceNameEndIndex = namespaceScopeBeginIndex;
                else if(currentChar == SCOPE_OPEN)
                    break;
            }
        }

        index namespaceScopeEndIndex = namespaceScopeBeginIndex + 1;

        for(size_t openScopeCount = 1, closeScopeCount = 0; namespaceScopeEndIndex < scope.size() && openScopeCount != closeScopeCount; namespaceScopeEndIndex++)
        {
            const char currentChar = scope[namespaceScopeEndIndex];

            if(isCommentScopeClosed && IsFullSubstringSame(scope, namespaceScopeEndIndex, COMMENT_SCOPE_LINE))
                isCommentScopeClosed = false;
            else if(currentChar == NEWLINE)
                isCommentScopeClosed = true;

            if(isCommentScopeClosed)
            {
                if(currentChar == VARIABLE_VALUE_SCOPE)
                {
                    if(!isValueScopeClosed && namespaceScopeEndIndex > 0 && scope[namespaceScopeEndIndex - 1] != SPECIAL_CHAR_SIGN)
                        isValueScopeClosed = true;
                    else
                        isValueScopeClosed = false;
                }
                else if(isValueScopeClosed)
                {
                    if(currentChar == SCOPE_OPEN)
                        openScopeCount++;
                    else if(currentChar == SCOPE_CLOSE)
                        closeScopeCount++;
                }
            }
        }

        namespaceScopeEndIndex--;

        return
        {
            {namespaceKeywordBeginIndex, namespaceKeywordEndIndex},
            {namespaceNameBeginIndex, namespaceNameEndIndex},
            {namespaceScopeBeginIndex, namespaceScopeEndIndex}
        };
    }
    ConfigFile::Parser::VariableInfo ConfigFile::Parser::ReceiveVariableInfo(const std::string_view& scope, const index& variableTypeBeginIndex)
    {
        index variableTypeEndIndex = variableTypeBeginIndex + 1;

        char currentChar;

        for(; variableTypeEndIndex < scope.size(); variableTypeEndIndex++)
        {
            currentChar = scope[variableTypeEndIndex];

            if(!Variable::IsValidVariableChar(currentChar))
                break;
        }

        variableTypeEndIndex--;

        index variableNameBeginIndex = variableTypeEndIndex + 1;

        for(bool isCommented = false; variableNameBeginIndex < scope.size(); variableNameBeginIndex++)
        {
            currentChar = scope[variableNameBeginIndex];

            if(isCommented && currentChar == NEWLINE)
                isCommented = false;
            else if(!isCommented)
            {
                if(IsFullSubstringSame(scope, variableNameBeginIndex, COMMENT_SCOPE_LINE))
                {
                    isCommented = true;
                    variableNameBeginIndex += COMMENT_SCOPE_LINE.size() - 1;
                }
                else if(Variable::IsValidVariableChar(currentChar))
                    break;
            }
        }

        index variableNameEndIndex = variableNameBeginIndex + 1;
        for(; variableNameEndIndex < scope.size(); variableNameEndIndex++)
        {
            currentChar = scope[variableNameEndIndex];

            if(!Variable::IsValidVariableChar(currentChar))
            {
                if(variableNameEndIndex > 0)
                    variableNameEndIndex--;

                break;
            }
        }

        index equalsIndex = variableNameEndIndex + 1;
        for(bool isCommented = false; equalsIndex < scope.size(); equalsIndex++)
        {
            currentChar = scope[equalsIndex];

            if(isCommented && currentChar == NEWLINE)
                isCommented = false;
            else if(!isCommented)
            {
                if(IsFullSubstringSame(scope, equalsIndex, COMMENT_SCOPE_LINE))
                {
                    isCommented = true;
                    equalsIndex += COMMENT_SCOPE_LINE.size() - 1;
                }
                else if(currentChar == EQUALS)
                    break;
            }
        }

        bool isArray = false;

        index variableValueBeginIndex = equalsIndex + 1;
        for(bool isCommented = false; variableValueBeginIndex < scope.size(); variableValueBeginIndex++)
        {
            currentChar = scope[variableValueBeginIndex];

            if(isCommented && currentChar == NEWLINE)
                isCommented = false;
            else if(!isCommented)
            {
                if(IsFullSubstringSame(scope, variableValueBeginIndex, COMMENT_SCOPE_LINE))
                {
                    isCommented = true;
                    variableValueBeginIndex += COMMENT_SCOPE_LINE.size() - 1;
                }
                else if(currentChar == VARIABLE_VALUE_SCOPE)
                    break;
                else if(currentChar == SCOPE_OPEN)
                {
                    isArray = true;
                    break;
                }
            }
        }

        index variableValueEndIndex = variableValueBeginIndex + 1;
        if(isArray)
            for(bool isCommented = false, isString = false; variableValueEndIndex < scope.size(); variableValueEndIndex++)
            {
                currentChar = scope[variableValueEndIndex];

                if(isCommented && currentChar == NEWLINE)
                    isCommented = false;
                else if(!isCommented)
                {
                    if(IsFullSubstringSame(scope, variableValueEndIndex, COMMENT_SCOPE_LINE))
                    {
                        isCommented = true;
                        variableValueEndIndex += COMMENT_SCOPE_LINE.size() - 1;
                    }
                    //else if(currentChar == VARIABLE_VALUE_SCOPE && (variableValueEndIndex > 0 && scope[variableValueEndIndex - 1] != SPECIAL_CHAR_SIGN || variableValueEndIndex > 1 && scope[variableValueEndIndex - 1] == SPECIAL_CHAR_SIGN && scope[variableValueEndIndex - 2] == SPECIAL_CHAR_SIGN))
                    else if(currentChar == VARIABLE_VALUE_SCOPE)
                    {
                        size_t specialCharSignCount = 0;
                        for(index i = variableValueEndIndex - 1; i > 0; i--)
                        {
                            char _currentChar = scope[i];
                            if(_currentChar == SPECIAL_CHAR_SIGN)
                                specialCharSignCount++;
                            else
                                break;
                        }

                        if(specialCharSignCount % 2 == 0)
                            isString = !isString;
                    }
                    else if(!isString && currentChar == SCOPE_CLOSE)
                        break;
                }
            }
        else
            for(; variableValueEndIndex < scope.size(); variableValueEndIndex++)
            {
                currentChar = scope[variableValueEndIndex];
                auto ___ = scope.cbegin() + variableValueEndIndex;

                //if(currentChar == VARIABLE_VALUE_SCOPE && (variableValueEndIndex > 0 && scope[variableValueEndIndex - 1] != SPECIAL_CHAR_SIGN || variableValueEndIndex > 1 && scope[variableValueEndIndex - 1] == SPECIAL_CHAR_SIGN && scope[variableValueEndIndex - 2] == SPECIAL_CHAR_SIGN))
                if(currentChar == VARIABLE_VALUE_SCOPE)
                {
                    size_t specialCharSignCount = 0;
                    for(index i = variableValueEndIndex - 1; i > 0; i--)
                    {
                        char _currentChar = scope[i];
                        if(_currentChar == SPECIAL_CHAR_SIGN)
                            specialCharSignCount++;
                        else
                            break;
                    }

                    if(specialCharSignCount % 2 == 0)
                        break;
                }
            }

        index variableValueSemicolon = variableValueEndIndex + 1;
        for(bool isCommented = false; variableValueSemicolon < scope.size(); variableValueSemicolon++)
        {
            currentChar = scope[variableValueSemicolon];

            if(isCommented && currentChar == NEWLINE)
                isCommented = false;
            else if(!isCommented)
            {
                if(IsFullSubstringSame(scope, variableValueSemicolon, COMMENT_SCOPE_LINE))
                {
                    isCommented = true;
                    variableValueSemicolon += COMMENT_SCOPE_LINE.size() - 1;
                }
                else if(currentChar == SEMICOLON)
                    break;
            }
        }

        if(variableValueBeginIndex == variableValueEndIndex - 1)
        {
            variableValueBeginIndex = std::string::npos;
            variableValueEndIndex = std::string::npos;
        }
        else
        {
            if(!isArray)
            {
                variableValueBeginIndex++;
                variableValueEndIndex--;
            }
        }

        return
        {
            {variableTypeBeginIndex, variableTypeEndIndex},
            equalsIndex,
            {variableNameBeginIndex, variableNameEndIndex},
            {variableValueBeginIndex, variableValueEndIndex},
            variableValueSemicolon,
            isArray
        };
    }

    ConfigFile::Parser::NamespaceIndicesInfo ConfigFile::Parser::FindNamespace(const std::string_view& scope, const std::string_view& path)
    {
        if(path.empty())
            throw std::invalid_argument{ "namespace path is empty" };

        bool isCommentScopeClosed = true;
        bool isValueScopeClosed = true;

        for(index i = 0; i < scope.size(); i++)
        {
            const ParsingDataType parsingDataType = DetermineParsingDataType(scope, i, isCommentScopeClosed, isValueScopeClosed);

            if(parsingDataType == ParsingDataType::Namespace)
            {
                const NamespaceIndicesInfo namespaceIndicesInfo = ReceiveNamespaceInfo(scope, i);

                //const std::string_view foundNamespaceName{ scope.cbegin() + namespaceIndicesInfo.nameBegin, scope.cbegin() + namespaceIndicesInfo.nameEnd + 1 };
                const std::string_view foundNamespaceName = namespaceIndicesInfo.name.GetSubstring<std::string_view>(scope);

                const index pathSeparatorIndex = path.find('/');

                //current namespace path name so namespace1/namespace2/namespace3
                //                               ^ starts from the beginning, cuz you need to think
                const std::string_view currentPath{ path.cbegin(), path.cbegin() + (pathSeparatorIndex == std::string::npos ? path.size() : pathSeparatorIndex) };

                if(currentPath == foundNamespaceName)
                {
                    if(pathSeparatorIndex == std::string::npos)
                        return namespaceIndicesInfo;
                    else
                    {
                        //but there could be multiple namespaces with the same name
                        const std::string_view nextPath{ path.cbegin() + currentPath.size() + 1, path.cend() };

                        //const std::string_view nextScope{ scope.cbegin() + namespaceIndicesInfo.scopeBegin, scope.cbegin() + namespaceIndicesInfo.scopeEnd };
                        const std::string_view nextScope = namespaceIndicesInfo.scope.GetSubstring<std::string_view>(scope);

                        //wtf is wrong with returning indices?
                        auto n = FindNamespace(nextScope, nextPath);

                        return n + namespaceIndicesInfo.scope.begin;
                    }
                }

                i = namespaceIndicesInfo.scope.end + 1;
            }
        }

        throw std::out_of_range{ "Failed to find namespace" };
    }

    ConfigFile::Parser::VariableInfo ConfigFile::Parser::FindVariableInfo(const std::string_view& scope, const std::string_view& path)
    {
        const index lastPathSeparatorIndex = path.find_last_of('/');

        const std::string_view namespacePath = (lastPathSeparatorIndex == std::string::npos ? "" : std::string_view{ path.cbegin(), path.cbegin() + lastPathSeparatorIndex });
        const std::string_view toFindVariableName{ path.cbegin() + (lastPathSeparatorIndex == std::string::npos ? 0 : lastPathSeparatorIndex + 1), path.cend() };

        if(lastPathSeparatorIndex == std::string::npos)
        {
            bool isCommentScopeClosed = true;
            bool isValueScopeClosed = true;

            for(index j = 0; j < scope.size(); j++)
            {
                const ParsingDataType parsingDataType = DetermineParsingDataType(scope, j, isCommentScopeClosed, isValueScopeClosed);

                if(parsingDataType == ParsingDataType::Variable)
                {
                    const VariableInfo variableInfo = ReceiveVariableInfo(scope, j);

                    const std::string_view variableName = variableInfo.name.GetSubstring<std::string_view>(scope);

                    if(variableName == toFindVariableName)
                        return variableInfo;

                    j = variableInfo.semicolon + 1;
                }
                else if(parsingDataType == ParsingDataType::Namespace)
                {
                    const NamespaceIndicesInfo namespaceInfo = ReceiveNamespaceInfo(scope, j);

                    j = namespaceInfo.scope.end;
                }
            }
        }
        else
        {
            std::string_view nextScope{ scope };
            index offset = 0;

            for(NamespaceIndicesInfo namespaceInfo = FindNamespace(nextScope, namespacePath); namespaceInfo.scope.end < scope.size(); namespaceInfo = FindNamespace(nextScope, namespacePath))
            {
                const std::string_view namespaceScope = namespaceInfo.scope.GetSubstring<std::string_view>(nextScope);

                bool isCommentScopeClosed = true;
                bool isValueScopeClosed = true;

                for(index j = 0; j < namespaceScope.size(); j++)
                {
                    const ParsingDataType parsingDataType = DetermineParsingDataType(namespaceScope, j, isCommentScopeClosed, isValueScopeClosed);

                    if(parsingDataType == ParsingDataType::Variable)
                    {
                        const VariableInfo variableInfo = ReceiveVariableInfo(namespaceScope, j);

                        const std::string_view variableName = variableInfo.name.GetSubstring<std::string_view>(namespaceScope);

                        if(variableName == toFindVariableName)
                            return variableInfo + offset + namespaceInfo.scope.begin;

                        j = variableInfo.semicolon + 1;
                    }
                }

                nextScope = std::string_view{ nextScope.cbegin() + namespaceInfo.scope.end + 1, nextScope.cend() };
                offset += namespaceInfo.scope.end + 1;
            }
        }

        throw std::out_of_range{ "failed to find variable" };
    }
    Variable ConfigFile::Parser::FindVariable(const std::string_view& scope, const std::string_view& path)
    {
        const VariableInfo info = FindVariableInfo(scope, path);

        const std::string_view variableValueRaw = info.value.GetSubstring<std::string_view>(scope);
        std::string variableValue{ variableValueRaw };

        for(char specialChar : SPECIAL_CHARS)
            for(index j = 0; j < variableValue.size(); j++)
                if(j > 0 && variableValue[j] == specialChar && variableValue[j - 1] == SPECIAL_CHAR_SIGN)
                    variableValue.erase(j - 1, 1);

        return Variable{ path.data(), std::move(variableValue), StringToDataType(info.type.GetSubstring<std::string_view>(scope)), IsArray(variableValueRaw) };
    }

    std::string ConfigFile::Parser::WriteVariable(std::string scope, const Variable& variable)
    {
        bool doesScopeExist = true;

        const std::string_view path = variable.GetPath();
        std::string_view currentScope = scope;

        index prevPathSeparatorOffset;
        index pathSeparatorOffset = path.find(PATH_SEPARATOR);

        std::string_view currentNamespace;

        index insertOffset = pathSeparatorOffset != std::string::npos;//why two? cuz something is about indices

        //finding the scope wee need
        try
        {
            NamespaceIndicesInfo namespaceIndicesInfo;
            for(prevPathSeparatorOffset = 0; pathSeparatorOffset != std::string::npos; prevPathSeparatorOffset = pathSeparatorOffset + 1, pathSeparatorOffset = path.find(PATH_SEPARATOR, prevPathSeparatorOffset))
            {
                currentNamespace = { path.cbegin() + prevPathSeparatorOffset, path.cbegin() + pathSeparatorOffset };

                namespaceIndicesInfo = FindNamespace(currentScope, currentNamespace);

                currentScope = namespaceIndicesInfo.scope.GetSubstring<std::string_view>(currentScope);
                insertOffset += namespaceIndicesInfo.scope.begin;
            }
        }
        catch(...)
        {
            doesScopeExist = false;
        }

        //check for comments offset
        if(!insertOffset)
            for(bool isCommented = false; insertOffset < currentScope.size(); insertOffset++)
            {
                const char currentChar = currentScope[insertOffset];

                if(isCommented && currentChar == NEWLINE)
                {
                    isCommented = false;
                    break;
                }
                else if(!isCommented)
                {
                    if(IsFullSubstringSame(currentScope, insertOffset, COMMENT_SCOPE_LINE))
                    {
                        isCommented = true;
                        insertOffset += COMMENT_SCOPE_LINE.size() - 1;
                    }
                }
            }

        if(doesScopeExist)
        {
            std::stringstream variableToInsertStringStream;

            variableToInsertStringStream << NEWLINE << DataTypeToString(variable.GetType()) << ' ' << variable.GetName() << ' ' << EQUALS << ' ';
            if(variable.IsArray())
                variableToInsertStringStream << SCOPE_OPEN << variable.GetRawValue() << SCOPE_CLOSE;
            else
                variableToInsertStringStream << VARIABLE_VALUE_SCOPE << variable.GetRawValue() << VARIABLE_VALUE_SCOPE;

            variableToInsertStringStream << SEMICOLON << NEWLINE;

            //ahhh... copying
            scope.insert(insertOffset, variableToInsertStringStream.str());
        }
        else
        {
            //the scope doesn't exist, so we need to create it
            std::stringstream newScope;

            size_t scopesOpenedCount = 0;

            //newScope << NAMESPACE_KEYWORD << ' ' << currentNamespace << NEWLINE << SCOPE_OPEN;

            for(; pathSeparatorOffset != std::string::npos; prevPathSeparatorOffset = pathSeparatorOffset + 1, pathSeparatorOffset = path.find(PATH_SEPARATOR, prevPathSeparatorOffset), scopesOpenedCount++)
            {
                newScope << NEWLINE << NAMESPACE_KEYWORD << ' ' << currentNamespace << NEWLINE << SCOPE_OPEN << NEWLINE;

                currentNamespace = { path.cbegin() + prevPathSeparatorOffset, path.cbegin() + pathSeparatorOffset };
            }

            newScope << DataTypeToString(variable.GetType()) << ' ' << variable.GetName() << ' ' << EQUALS << ' ';
            if(variable.IsArray())
                newScope << SCOPE_OPEN << variable.GetRawValue() << SCOPE_CLOSE;
            else
                newScope << VARIABLE_VALUE_SCOPE << variable.GetRawValue() << VARIABLE_VALUE_SCOPE;

            newScope << SEMICOLON << NEWLINE;

            for(size_t i = 0; i < scopesOpenedCount; i++)
                newScope << SCOPE_CLOSE << NEWLINE;

            //ahhh... copying
            scope.insert(insertOffset, newScope.str());
        }

        return scope;
    }

    std::string ConfigFile::Parser::DeleteNamespace(std::string scope, const std::string_view& path)
    {
        NamespaceIndicesInfo namespaceIndicesInfo = FindNamespace(scope, path);

        scope.erase(scope.cbegin() + namespaceIndicesInfo.keyword.begin, scope.cbegin() + namespaceIndicesInfo.scope.end + 1);

        return scope;
    }
    std::string ConfigFile::Parser::DeleteVariable(std::string scope, const std::string_view& path)
    {
        VariableInfo variableIndicesInfo = FindVariableInfo(scope, path);

        scope.erase(scope.cbegin() + variableIndicesInfo.type.begin, scope.cbegin() + variableIndicesInfo.semicolon + 1);

        return scope;
    }

    ConfigFile::Parser::ParsingDataType ConfigFile::Parser::DetermineParsingDataType(const std::string_view& scope, index currentCharIndex, bool& wasCommentScopeClosed, bool& wasValueScopeClosed)
    {
        char currentChar = scope[currentCharIndex];

        if(wasCommentScopeClosed && IsFullSubstringSame(scope, currentCharIndex, COMMENT_SCOPE_LINE))
        {
            wasCommentScopeClosed = false;
            return ParsingDataType::Comment;
        }
        else if(currentChar == NEWLINE)
            wasCommentScopeClosed = true;

        if(wasCommentScopeClosed)
        {
            if(currentChar == VARIABLE_VALUE_SCOPE)
            {
                if(!wasValueScopeClosed && currentCharIndex > 0 && scope[currentCharIndex - 1] != SPECIAL_CHAR_SIGN)
                    wasValueScopeClosed = true;
                else
                {
                    wasValueScopeClosed = false;
                    return ParsingDataType::VariableValue;
                }
            }
            else if(wasValueScopeClosed)
            {
                if(IsFullSubstringSame(scope, currentCharIndex, NAMESPACE_KEYWORD))
                    return ParsingDataType::Namespace;
                else if(Variable::IsValidVariableChar(currentChar))
                    return ParsingDataType::Variable;
            }
        }

        return ParsingDataType::Invalid;
    }

    bool ConfigFile::Parser::IsArray(const std::string_view& variableValue)
    {
        if(variableValue.empty())
            return false;

        bool isArray = true;

        if(variableValue[0] != SCOPE_OPEN)
            isArray = false;
        if(isArray && variableValue[variableValue.size() - 1] != SCOPE_CLOSE)
            isArray = false;

        if(isArray)
        {
            bool valueScopeClosed = true;
            for(index i = 1; i < variableValue.size(); i++)
            {
                const char currentChar = variableValue[i];

                if(currentChar == VARIABLE_VALUE_SCOPE && variableValue[i - 1] != SPECIAL_CHAR_SIGN)
                    valueScopeClosed = !valueScopeClosed;
            }

            isArray = valueScopeClosed;
        }

        return isArray;
    }
}
//ConfigFile::Parser's indices stuff
namespace GuelderResourcesManager
{

    ConfigFile::Parser::StringRange::StringRange(index begin, index end)
        : begin(begin), end(end)
    {
        if(begin > end)
            throw std::invalid_argument{ "begin cannot be bigger than end" };
    }

    ConfigFile::Parser::StringRange operator+(const ConfigFile::Parser::StringRange& lhs, const ConfigFile::Parser::StringRange& rhs)
    {
        return { lhs.begin + rhs.begin, lhs.end + rhs.end };
    }
    ConfigFile::Parser::StringRange operator+(const ConfigFile::Parser::StringRange& lhs, ConfigFile::Parser::index rhs)
    {
        return { lhs.begin + rhs, lhs.end + rhs };
    }

    ConfigFile::Parser::NamespaceIndicesInfo operator+(const ConfigFile::Parser::NamespaceIndicesInfo& lhs, const ConfigFile::Parser::NamespaceIndicesInfo& rhs)
    {
        return
        {
            lhs.keyword + rhs.keyword,
            lhs.name + rhs.name,
            lhs.scope + rhs.scope,
        };
    }
    ConfigFile::Parser::NamespaceIndicesInfo operator+(const ConfigFile::Parser::NamespaceIndicesInfo& lhs, ConfigFile::Parser::index rhs)
    {
        return
        {
            lhs.keyword + rhs,
            lhs.name + rhs,
            lhs.scope + rhs,
        };
    }

    ConfigFile::Parser::VariableInfo operator+(const ConfigFile::Parser::VariableInfo& lhs, const ConfigFile::Parser::VariableInfo& rhs)
    {
        return
        {
            lhs.type + rhs.type,
            lhs.equals + rhs.equals,
            lhs.name + rhs.name,
            lhs.value + rhs.value,
            lhs.semicolon + rhs.semicolon
        };
    }
    ConfigFile::Parser::VariableInfo operator+(const ConfigFile::Parser::VariableInfo& lhs, ConfigFile::Parser::index rhs)
    {
        return
        {
            lhs.type + rhs,
            lhs.equals + rhs,
            lhs.name + rhs,
            lhs.value + rhs,
            lhs.semicolon + rhs
        };
    }
}
//ResourcesManager
namespace GuelderResourcesManager
{
    ResourcesManager::ResourcesManager(std::filesystem::path executablePath)
        : m_Path(std::move(executablePath)) {
    }

    std::string ResourcesManager::ReceiveFileSource(const std::filesystem::path& filePath)
    {
        std::ifstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);

        file.open(filePath, std::ios::binary);

        std::stringstream source;//istringstream or stringstream?
        source << file.rdbuf();

        file.close();

        return source.str();
    }

    void ResourcesManager::WriteToFile(const std::filesystem::path& filePath, const std::string_view& content)
    {
        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);

        file.open(filePath, std::ios::binary);

        file << content;

        file.close();
    }
    void ResourcesManager::WriteToFile(const std::filesystem::path& filePath, ConfigFile::Parser::index index, const std::string_view& content)
    {
        std::string source = ReceiveFileSource(filePath);

        source.insert(index, content);

        WriteToFile(filePath, source);
    }

    std::filesystem::path ResourcesManager::GetFullPathToRelativeFile(const std::filesystem::path& relativePath) const
    {
        return m_Path / relativePath;
    }

    const std::filesystem::path& ResourcesManager::GetPath() const
    {
        return m_Path;
    }
}