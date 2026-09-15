#pragma once

#include <Arduino.h>
#include <vector>
#include "KeyValuePair.h"

class ApiResponse
{
public:
    bool IsValid;
    bool IsArray;
    String Json;

    std::vector<std::vector<KeyValuePair>> Objects;

    ApiResponse(const String& responseBody)
        : IsValid(false),
          IsArray(false),
          Json("")
    {
        Json = ExtractJson(responseBody);

        if (Json.length() == 0)
        {
            return;
        }

        IsArray = Json.charAt(0) == '[';

        if (IsArray)
        {
            Objects = ParseArray(Json);
        }
        else
        {
            std::vector<KeyValuePair> objectProperties =
                ParseObject(Json);

            if (!objectProperties.empty())
            {
                Objects.push_back(objectProperties);
            }
        }

        IsValid = !Objects.empty() || IsArray;
    }

    String GetValue(
        int objectIndex,
        const String& propertyName) const
    {
        if (objectIndex < 0 ||
            objectIndex >= (int)Objects.size())
        {
            return "";
        }

        for (const KeyValuePair& parameter :
             Objects[objectIndex])
        {
            if (parameter.Name == propertyName)
            {
                return parameter.Value;
            }
        }

        return "";
    }

private:
    String ExtractJson(const String& responseBody)
    {
        int arrayStart = responseBody.indexOf('[');

        if (arrayStart >= 0)
        {
            int arrayEnd = responseBody.lastIndexOf(']');

            if (arrayEnd > arrayStart)
            {
                return responseBody.substring(
                    arrayStart,
                    arrayEnd + 1);
            }

            return "";
        }

        int objectStart = responseBody.indexOf('{');

        if (objectStart >= 0)
        {
            int objectEnd = responseBody.lastIndexOf('}');

            if (objectEnd > objectStart)
            {
                return responseBody.substring(
                    objectStart,
                    objectEnd + 1);
            }
        }

        return "";
    }

    std::vector<std::vector<KeyValuePair>>
    ParseArray(const String& jsonArray)
    {
        std::vector<std::vector<KeyValuePair>> result;

        bool insideString = false;
        bool escaped = false;
        int objectDepth = 0;
        int objectStart = -1;

        for (int i = 0; i < jsonArray.length(); i++)
        {
            char current = jsonArray.charAt(i);

            if (insideString)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (current == '\\')
                {
                    escaped = true;
                }
                else if (current == '"')
                {
                    insideString = false;
                }

                continue;
            }

            if (current == '"')
            {
                insideString = true;
                continue;
            }

            if (current == '{')
            {
                if (objectDepth == 0)
                {
                    objectStart = i;
                }

                objectDepth++;
            }
            else if (current == '}')
            {
                if (objectDepth > 0)
                {
                    objectDepth--;
                }

                if (objectDepth == 0 &&
                    objectStart >= 0)
                {
                    String jsonObject =
                        jsonArray.substring(
                            objectStart,
                            i + 1);

                    std::vector<KeyValuePair>
                        objectProperties =
                            ParseObject(jsonObject);

                    if (!objectProperties.empty())
                    {
                        result.push_back(
                            objectProperties);
                    }

                    objectStart = -1;
                }
            }
        }

        return result;
    }

    std::vector<KeyValuePair>
    ParseObject(const String& jsonObject)
    {
        std::vector<KeyValuePair> result;

        int position = 0;

        SkipWhitespaceAndDelimiters(
            jsonObject,
            position);

        if (position >= jsonObject.length() ||
            jsonObject.charAt(position) != '{')
        {
            return result;
        }

        position++;

        while (position < jsonObject.length())
        {
            SkipWhitespaceAndDelimiters(
                jsonObject,
                position);

            if (position >= jsonObject.length() ||
                jsonObject.charAt(position) == '}')
            {
                break;
            }

            String propertyName =
                ReadJsonValue(
                    jsonObject,
                    position);

            SkipWhitespace(
                jsonObject,
                position);

            if (position >= jsonObject.length() ||
                jsonObject.charAt(position) != ':')
            {
                return std::vector<KeyValuePair>();
            }

            position++;

            SkipWhitespace(
                jsonObject,
                position);

            String propertyValue =
                ReadJsonValue(
                    jsonObject,
                    position);

            result.push_back(
                KeyValuePair(
                    propertyName,
                    propertyValue));

            SkipWhitespaceAndDelimiters(
                jsonObject,
                position);
        }

        return result;
    }

    String ReadJsonValue(
        const String& json,
        int& position)
    {
        SkipWhitespace(json, position);

        if (position >= json.length())
        {
            return "";
        }

        if (json.charAt(position) == '"')
        {
            return ReadQuotedString(
                json,
                position);
        }

        int start = position;

        while (position < json.length())
        {
            char current = json.charAt(position);

            if (current == ',' ||
                current == '}' ||
                current == ']')
            {
                break;
            }

            position++;
        }

        String value =
            json.substring(start, position);

        value.trim();

        return value;
    }

    String ReadQuotedString(
        const String& json,
        int& position)
    {
        String result = "";

        if (json.charAt(position) != '"')
        {
            return result;
        }

        position++;

        bool escaped = false;

        while (position < json.length())
        {
            char current = json.charAt(position);
            position++;

            if (escaped)
            {
                switch (current)
                {
                    case '"':
                        result += '"';
                        break;

                    case '\\':
                        result += '\\';
                        break;

                    case '/':
                        result += '/';
                        break;

                    case 'n':
                        result += '\n';
                        break;

                    case 'r':
                        result += '\r';
                        break;

                    case 't':
                        result += '\t';
                        break;

                    default:
                        result += current;
                        break;
                }

                escaped = false;
            }
            else if (current == '\\')
            {
                escaped = true;
            }
            else if (current == '"')
            {
                break;
            }
            else
            {
                result += current;
            }
        }

        return result;
    }

    void SkipWhitespace(
        const String& text,
        int& position)
    {
        while (position < text.length())
        {
            char current = text.charAt(position);

            if (current != ' ' &&
                current != '\r' &&
                current != '\n' &&
                current != '\t')
            {
                break;
            }

            position++;
        }
    }

    void SkipWhitespaceAndDelimiters(
        const String& text,
        int& position)
    {
        while (position < text.length())
        {
            char current = text.charAt(position);

            if (current != ' ' &&
                current != '\r' &&
                current != '\n' &&
                current != '\t' &&
                current != ',')
            {
                break;
            }

            position++;
        }
    }
};