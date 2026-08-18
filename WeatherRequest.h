#pragma once
#include <Arduino.h>
#include <vector>
#include "WeatherParam.h"

class WeatherRequest {
public:
    String httpMethod;
    String httpVersion;
    String httpRequest;
    bool isWeatherRequest;

    std::vector<WeatherParam> Parameters;

    // Constructor: receives full request, parses and fills fields
    WeatherRequest(const String& request) {
        httpRequest = request;
        String paramString = parseRequest(request);
        Parameters = toVector(paramString);
    }

    String GetParamValue(const String& name) const
    {
        for (const auto& param : Parameters)
        {
            if (param.Name == name)
            {
                return param.Value;
            }
        }

        return "";
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

        // Parameters: after first '&' and before " HTTP/"
        int paramStart = request.indexOf('&');
        int paramEnd = request.indexOf(" HTTP/", paramStart);
        String params = (paramStart > 0 && paramEnd > paramStart)
            ? request.substring(paramStart + 1, paramEnd)
            : "";

        return params;
    }

    // Parses key=value pairs into WeatherParam vector
    std::vector<WeatherParam> toVector(const String& parameters) {
        std::vector<WeatherParam> result;
        int start = 0;
        while (start < parameters.length()) {
            int end = parameters.indexOf('&', start);
            if (end == -1) end = parameters.length();
            String pair = parameters.substring(start, end);
            int eqIdx = pair.indexOf('=');
            if (eqIdx > 0) {
                String name = pair.substring(0, eqIdx);
                String value = pair.substring(eqIdx + 1);
                result.push_back(WeatherParam(name, value));
            }
            start = end + 1;
        }
        return result;
    }
};