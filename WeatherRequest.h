#pragma once

#include <Arduino.h>
#include <vector>
#include "KeyValuePair.h"


class WeatherRequest {
public:
    const String Date_Utc = "dateutc";

    String httpMethod;
    String httpVersion;
    String httpRequest;
    bool isWeatherRequest;

    std::vector<KeyValuePair> Properties;

    // Constructor: receives full request, parses and fills fields
    WeatherRequest(const String& request) {
        httpRequest = request;
        String paramString = parseRequest(request);
        Properties = toVector(paramString);
    }

    String GetParamValue(const String& name) const
    {
        for (const auto& param : Properties)
        {
            if (param.Name == name)
            {
                return param.Value;
            }
        }

        return "";
    }

    String ToJson() const
    {
        String returnJson = "[";

        bool first = true;

        for (const auto& param : Properties)
        {
            if (!first)
            {
                returnJson += ",";
            }

            returnJson += param.ToJson();
            first = false;
        }

        returnJson += "]";

        return returnJson;
    }

    String GetDateString()
    {
        String ambientDateString = GetParamValue(Date_Utc);

        ambientDateString.replace('+', 'T');

        return ambientDateString;
    }

private:
    // Extracts method and version, returns parameter substring
    String parseRequest(const String& request) {
        // Method: first word
        int firstSpace = request.indexOf(' ');
        httpMethod = (firstSpace > 0) ? request.substring(0, firstSpace) : "";

        // Version: last word
        int lastSpace = request.lastIndexOf(' ');
        httpVersion = (lastSpace > 0) ? request.substring(lastSpace + 1) : "";

        isWeatherRequest = request.indexOf("data/report/") > -1;

        // Properties: after first '&' and before " HTTP/"
        int paramStart = request.indexOf('&');
        int paramEnd = request.indexOf(" HTTP/", paramStart);
        String params = (paramStart > 0 && paramEnd > paramStart)
            ? request.substring(paramStart + 1, paramEnd)
            : "";

        return params;
    }

    // Parses key=value pairs into KeyValuePair vector
    std::vector<KeyValuePair> toVector(const String& Properties) {
        std::vector<KeyValuePair> result;
        int start = 0;
        while (start < Properties.length()) {
            int end = Properties.indexOf('&', start);
            if (end == -1) end = Properties.length();
            String pair = Properties.substring(start, end);
            int eqIdx = pair.indexOf('=');
            if (eqIdx > 0) {
                String name = pair.substring(0, eqIdx);
                String value = pair.substring(eqIdx + 1);
                result.push_back(KeyValuePair(name, value));
            }
            start = end + 1;
        }
        return result;
    }
};