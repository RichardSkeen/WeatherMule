#pragma once

#include <Arduino.h>

class Hole
{
public:
    String VendorDeviceKey;

    String FromUtc;
    String ToUtc;

    String FromDateRaw;
    String ToDateRaw;

    bool PartiallyFilled;
    uint32_t ByteOffset;

    Hole()
        : VendorDeviceKey(""),
          FromUtc(""),
          ToUtc(""),
          FromDateRaw(""),
          ToDateRaw(""),
          PartiallyFilled(false),
          ByteOffset(0)
    {
    }

    Hole(
        const String& vendorDeviceKey,
        const String& fromUtc,
        const String& toUtc,
        const String& fromDateRaw,
        const String& toDateRaw)
        : VendorDeviceKey(vendorDeviceKey),
          FromUtc(fromUtc),
          ToUtc(toUtc),
          FromDateRaw(fromDateRaw),
          ToDateRaw(toDateRaw),
          PartiallyFilled(false),
          ByteOffset(0)
    {
    }

    Hole(const String& jsonHole)
        : VendorDeviceKey(""),
          FromUtc(""),
          ToUtc(""),
          FromDateRaw(""),
          ToDateRaw(""),
          PartiallyFilled(false),
          ByteOffset(0)
    {
        VendorDeviceKey = ExtractJsonValue(
            jsonHole,
            "vendorDeviceKey");

        FromUtc = ExtractJsonValue(
            jsonHole,
            "fromUtc");

        ToUtc = ExtractJsonValue(
            jsonHole,
            "toUtc");

        FromDateRaw = ExtractJsonValue(
            jsonHole,
            "fromDateRaw");

        ToDateRaw = ExtractJsonValue(
            jsonHole,
            "toDateRaw");
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
            "\",\"toUtc\":\"" + ToUtc +
            "\",\"fromDateRaw\":\"" + FromDateRaw +
            "\",\"toDateRaw\":\"" + ToDateRaw + "\"}";
    }

private:
    static String ExtractJsonValue(
        const String& json,
        const String& key)
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