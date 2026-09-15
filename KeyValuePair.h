#pragma once
#include <Arduino.h>

class KeyValuePair
{
public:

    String Name;
    String Value;

    KeyValuePair()
        : Name(""), Value("")
    {
    }

    KeyValuePair(const String& name, const String& value)
        : Name(name), Value(value)
    {
    }

    String ToJson() const
    {
        return "{\"FieldName\":\"" + Name +
            "\",\"FieldValue\":\"" + Value + "\"}";
    }
};