#pragma once
#include <Arduino.h>

class Hole
{
public:

    String VendorDeviceKey;
    String FromUtc;
    String ToUtc;
    bool PartiallyFilled;
    uint32_t ByteOffset;

    Hole()
        : VendorDeviceKey(""), FromUtc(""), ToUtc(""), PartiallyFilled(false), ByteOffset(0)
    {
    }

    Hole(const String& vendorDeviceKey, const String& fromUtc, const String& toUtc)
        : VendorDeviceKey(vendorDeviceKey), FromUtc(fromUtc), ToUtc(toUtc), PartiallyFilled(false), ByteOffset(0)
    {
    }

    Hole(const String& jsonHole)
        : VendorDeviceKey(""), FromUtc(""), ToUtc(""), PartiallyFilled(false), ByteOffset(0)
    {
        VendorDeviceKey = ExtractJsonValue(jsonHole, "vendorDeviceKey");
        FromUtc         = ExtractJsonValue(jsonHole, "fromUtc");
        ToUtc           = ExtractJsonValue(jsonHole, "toUtc");
    }

    String ToPackBoxName() const
    {
        if (FromUtc.length() < 10)
        {
            return "unknown.box";
        }

        return FromUtc.substring(0, 10) + ".box";
    }

    String ToJson() const
    {
        return "{\"vendorDeviceKey\":\"" + VendorDeviceKey +
            "\",\"fromUtc\":\"" + FromUtc +
            "\",\"toUtc\":\"" + ToUtc + "\"}";
    }

    private:

    static String ExtractJsonValue(const String& json, const String& key)
    {
        String search = "\"" + key + "\":\"";

        int start = json.indexOf(search);
        if (start < 0)
        {
            return "";
        }

        start += search.length();

        int end = json.indexOf('"', start);
        if (end < 0)
        {
            return "";
        }

        return json.substring(start, end);
    }
};