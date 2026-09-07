#pragma once
#include <Arduino.h>

class WeatherParam
{
public:

    String Name;
    String Value;

    WeatherParam()
        : Name(""), Value("")
    {
    }

    WeatherParam(const String& name, const String& value)
        : Name(name), Value(value)
    {
    }

    String ToJson() const
    {
        return "{\"FieldName\":\"" + Name +
            "\",\"FieldValue\":\"" + Value + "\"}";
    }
};