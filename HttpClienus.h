#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>

//----------------------------------------------
// HTTP response
//----------------------------------------------

class HttpResponsum
{
public:
    String StatusLine;
    int StatusCode;
    String Body;
    bool Succeeded;
    String ErrorMessage;

    HttpResponsum()
        : StatusLine(""),
          StatusCode(0),
          Body(""),
          Succeeded(false),
          ErrorMessage("")
    {
    }
};

//----------------------------------------------
// HTTP methods
//----------------------------------------------

static const String HttpMethods[] =
{
    "GET",
    "POST",
    "PUT",
    "DELETE",
    "PATCH",
    "HEAD",
    "OPTIONS"
};

static const int HttpMethodCount =
    sizeof(HttpMethods) /
    sizeof(HttpMethods[0]);

//----------------------------------------------
// Limits and timeouts
//----------------------------------------------

static const unsigned long HttpResponseTimeoutMs = 10000;
static const unsigned long HttpDataTimeoutMs = 5000;

static const size_t HttpMaxStatusLineLength = 256;
static const size_t HttpMaxHeaderLineLength = 1024;
static const size_t HttpMaxBodyLength = 32768;

//----------------------------------------------
// Prototypes
//----------------------------------------------

inline HttpResponsum HttpRequest(
    const String& host,
    int port,
    const String& method,
    const String& url,
    const String& token,
    const String& userAgent,
    const String& payload);

inline bool IsAllowedHttpMethod(
    const String& method);

inline bool WaitForHttpData(
    WiFiClient& client,
    unsigned long timeoutMs);

inline bool ReadHttpLine(
    WiFiClient& client,
    String& line,
    size_t maximumLength,
    unsigned long timeoutMs);

inline int ParseHttpStatusCode(
    const String& statusLine);

inline bool ReadHttpHeaders(
    WiFiClient& client,
    long& contentLength,
    bool& chunked,
    unsigned long timeoutMs,
    String& errorMessage);

inline bool ReadHttpBody(
    WiFiClient& client,
    String& body,
    long contentLength,
    bool chunked,
    unsigned long timeoutMs,
    String& errorMessage);

inline bool ReadChunkedHttpBody(
    WiFiClient& client,
    String& body,
    unsigned long timeoutMs,
    String& errorMessage);

inline bool ReadExactHttpBytes(
    WiFiClient& client,
    String& destination,
    size_t byteCount,
    unsigned long timeoutMs,
    String& errorMessage);

inline String BuildHeaderAuth(
    const String& token);

inline String BuildUserAgent(
    const String& userAgent);

//----------------------------------------------
// Implementation
//----------------------------------------------

inline HttpResponsum HttpRequest(
    const String& host,
    int port,
    const String& method,
    const String& url,
    const String& token,
    const String& userAgent,
    const String& payload)
{
    HttpResponsum response;
    WiFiClient apiClient;

    String useMethod = method;
    useMethod.trim();
    useMethod.toUpperCase();

    //------------------------------------------
    // Validate request
    //------------------------------------------

    if(host.length() == 0)
    {
        response.ErrorMessage =
            "HTTP host was not provided.";
    }
    else if(port <= 0)
    {
        response.ErrorMessage =
            "HTTP port is invalid.";
    }
    else if(url.length() == 0)
    {
        response.ErrorMessage =
            "HTTP URL was not provided.";
    }
    else if(IsAllowedHttpMethod(useMethod) != true)
    {
        response.ErrorMessage =
            "Unsupported HTTP method: " +
            useMethod;
    }
    else if(apiClient.connect(host.c_str(), port) != true)
    {
        response.ErrorMessage =
            "Unable to connect to " +
            host +
            ":" +
            String(port);
    }
    else
    {
        //--------------------------------------
        // Send request line
        //--------------------------------------

        apiClient.print(useMethod);
        apiClient.print(" ");
        apiClient.print(url);
        apiClient.println(" HTTP/1.1");

        //--------------------------------------
        // Send headers
        //--------------------------------------

        apiClient.print("Host: ");
        apiClient.println(host);

        if(token.length() > 0)
        {
            apiClient.println(
                BuildHeaderAuth(token));
        }

        if(userAgent.length() > 0)
        {
            apiClient.println(
                BuildUserAgent(userAgent));
        }

        apiClient.println("Connection: close");

        //--------------------------------------
        // Send payload headers
        //--------------------------------------

        if(payload.length() > 0)
        {
            apiClient.println(
                "Content-Type: application/json");

            apiClient.print("Content-Length: ");
            apiClient.println(payload.length());
        }

        //--------------------------------------
        // End request headers
        //--------------------------------------

        apiClient.println();

        //--------------------------------------
        // Send payload
        //--------------------------------------

        if(payload.length() > 0)
        {
            apiClient.print(payload);
        }

        //--------------------------------------
        // Wait for response
        //--------------------------------------

        if(WaitForHttpData(apiClient, HttpResponseTimeoutMs) != true)
        {
            response.ErrorMessage =
                "Timed out waiting for HTTP response.";
        }
        else if(ReadHttpLine(apiClient, response.StatusLine, HttpMaxStatusLineLength, HttpDataTimeoutMs) != true)
        {
            response.ErrorMessage =
                "Unable to read HTTP status line.";
        }
        else
        {
            response.StatusCode = ParseHttpStatusCode(response.StatusLine);

            if(response.StatusCode == 0)
            {
                response.ErrorMessage = "Invalid HTTP status line: " + response.StatusLine;
            }
            else
            {
                long contentLength = -1;
                bool chunked = false;

                if(ReadHttpHeaders(apiClient, contentLength, chunked, HttpDataTimeoutMs, response.ErrorMessage) != true)
                {
                    // ErrorMessage set by helper.
                }
                else if(useMethod == "HEAD")
                {
                    response.Succeeded = true;
                }
                else if(ReadHttpBody(apiClient, response.Body, contentLength, chunked, HttpDataTimeoutMs, response.ErrorMessage) != true)
                {
                    // ErrorMessage set by helper.
                }
                else
                {
                    response.Succeeded = true;
                }
            }
        }
    }

    //------------------------------------------
    // Always close connection
    //------------------------------------------

    apiClient.stop();

    return response;
}

//----------------------------------------------
// Validate HTTP method
//----------------------------------------------

inline bool IsAllowedHttpMethod(const String& method)
{
    for(int index = 0; index < HttpMethodCount; index++)
    {
        if(method == HttpMethods[index])
        {
            return true;
        }
    }

    return false;
}

//----------------------------------------------
// Wait until data becomes available
//----------------------------------------------

inline bool WaitForHttpData(WiFiClient& client, unsigned long timeoutMs)
{
    unsigned long startedAt = millis();

    while(client.available() == 0)
    {
        if(client.connected() != true)
        {
            return false;
        }

        if((unsigned long)(millis() - startedAt) >= timeoutMs)
        {
            return false;
        }

        delay(1);
    }

    return true;
}

//----------------------------------------------
// Read one line across multiple packets
//----------------------------------------------

inline bool ReadHttpLine(WiFiClient& client, String& line, size_t maximumLength, unsigned long timeoutMs)
{
    line = "";

    unsigned long lastDataAt = millis();

    while(true)
    {
        while(client.available() > 0)
        {
            int readValue = client.read();

            if(readValue < 0)
            {
                break;
            }

            lastDataAt = millis();

            char currentCharacter = (char)readValue;

            if(currentCharacter == '\n')
            {
                if(line.endsWith("\r"))
                {
                    line.remove(line.length() - 1);
                }

                return true;
            }

            if(line.length() >= maximumLength)
            {
                return false;
            }

            line += currentCharacter;
        }

        if(client.connected() != true && client.available() == 0)
        {
            return false;
        }

        if((unsigned long)(millis() - lastDataAt) >= timeoutMs)
        {
            return false;
        }

        delay(1);
    }
}

//----------------------------------------------
// Parse status code
//----------------------------------------------

inline int ParseHttpStatusCode(
    const String& statusLine)
{
    int firstSpace = statusLine.indexOf(' ');

    if(firstSpace < 0)
    {
        return 0;
    }

    int secondSpace = statusLine.indexOf(' ', firstSpace + 1);

    String statusCodeText;

    if(secondSpace < 0)
    {
        statusCodeText = statusLine.substring(firstSpace + 1);
    }
    else
    {
        statusCodeText = statusLine.substring(firstSpace + 1, secondSpace);
    }

    statusCodeText.trim();

    if(statusCodeText.length() != 3)
    {
        return 0;
    }

    for(int index = 0; index < statusCodeText.length(); index++)
    {
        if(isDigit(statusCodeText.charAt(index)) != true)
        {
            return 0;
        }
    }

    return statusCodeText.toInt();
}

//----------------------------------------------
// Read response headers
//----------------------------------------------

inline bool ReadHttpHeaders(WiFiClient& client, long& contentLength, bool& chunked, unsigned long timeoutMs, String& errorMessage)
{
    contentLength = -1;
    chunked = false;

    while(true)
    {
        String headerLine;

        if(ReadHttpLine(client, headerLine, HttpMaxHeaderLineLength, timeoutMs) != true)
        {
            errorMessage = "Timed out or disconnected while reading HTTP headers.";

            return false;
        }

        if(headerLine.length() == 0)
        {
            return true;
        }

        int delimiter = headerLine.indexOf(':');

        if(delimiter < 0)
        {
            continue;
        }

        String headerName = headerLine.substring(0, delimiter);

        String headerValue = headerLine.substring(delimiter + 1);

        headerName.trim();
        headerValue.trim();

        headerName.toLowerCase();

        if(headerName == "content-length")
        {
            contentLength = headerValue.toInt();

            if(contentLength < 0)
            {
                errorMessage = "Invalid Content-Length header.";

                return false;
            }

            if((size_t)contentLength > HttpMaxBodyLength)
            {
                errorMessage = "HTTP response body exceeds maximum length.";

                return false;
            }
        }
        else if(headerName == "transfer-encoding")
        {
            headerValue.toLowerCase();

            if(headerValue.indexOf("chunked") >= 0)
            {
                chunked = true;
            }
        }
    }
}

//----------------------------------------------
// Read response body
//----------------------------------------------

inline bool ReadHttpBody(WiFiClient& client, String& body, long contentLength, bool chunked, unsigned long timeoutMs, String& errorMessage)
{
    body = "";

    if(chunked)
    {
        return ReadChunkedHttpBody(client, body, timeoutMs, errorMessage);
    }

    if(contentLength == 0)
    {
        return true;
    }

    if(contentLength > 0)
    {
        return ReadExactHttpBytes( client, body, (size_t)contentLength, timeoutMs, errorMessage);
    }

    //------------------------------------------
    // No Content-Length:
    // read until the server closes connection
    //------------------------------------------

    unsigned long lastDataAt = millis();

    while(true)
    {
        while(client.available() > 0)
        {
            int readValue = client.read();

            if(readValue < 0)
            {
                break;
            }

            lastDataAt = millis();

            if(body.length() >= HttpMaxBodyLength)
            {
                errorMessage = "HTTP response body exceeds maximum length.";

                return false;
            }

            body += (char)readValue;
        }

        if(client.connected() != true && client.available() == 0)
        {
            return true;
        }

        if((unsigned long)(millis() - lastDataAt) >= timeoutMs)
        {
            errorMessage = "Timed out while reading HTTP response body.";

            return false;
        }

        delay(1);
    }
}

//----------------------------------------------
// Read exact body length
//----------------------------------------------

inline bool ReadExactHttpBytes(
    WiFiClient& client,
    String& destination,
    size_t byteCount,
    unsigned long timeoutMs,
    String& errorMessage)
{
    if(destination.length() + byteCount > HttpMaxBodyLength)
    {
        errorMessage = "HTTP response body exceeds maximum length.";

        return false;
    }

    unsigned long lastDataAt = millis();
    size_t bytesRead = 0;

    while(bytesRead < byteCount)
    {
        while(client.available() > 0 && bytesRead < byteCount)
        {
            int readValue = client.read();

            if(readValue < 0)
            {
                break;
            }

            destination += (char)readValue;
            bytesRead++;
            lastDataAt = millis();
        }

        if(bytesRead >= byteCount)
        {
            return true;
        }

        if(client.connected() != true && client.available() == 0)
        {
            errorMessage = "Connection closed before the complete HTTP body was received.";

            return false;
        }

        if((unsigned long)(millis() - lastDataAt) >= timeoutMs)
        {
            errorMessage = "Timed out before the complete HTTP body was received.";

            return false;
        }

        delay(1);
    }

    return true;
}

//----------------------------------------------
// Read chunked response body
//----------------------------------------------

inline bool ReadChunkedHttpBody(WiFiClient& client, String& body, unsigned long timeoutMs, String& errorMessage)
{
    while(true)
    {
        String chunkSizeLine;

        if(ReadHttpLine(client, chunkSizeLine, HttpMaxHeaderLineLength, timeoutMs) != true)
        {
            errorMessage = "Unable to read HTTP chunk size.";

            return false;
        }

        int extensionPosition = chunkSizeLine.indexOf(';');

        if(extensionPosition >= 0)
        {
            chunkSizeLine = chunkSizeLine.substring(0, extensionPosition);
        }

        chunkSizeLine.trim();

        if(chunkSizeLine.length() == 0)
        {
            errorMessage = "Empty HTTP chunk size.";

            return false;
        }

        char* parseEnd = nullptr;

        unsigned long chunkSize = strtoul(chunkSizeLine.c_str(), &parseEnd, 16);

        if(parseEnd == chunkSizeLine.c_str() || *parseEnd != '\0')
        {
            errorMessage = "Invalid HTTP chunk size.";

            return false;
        }

        if(chunkSize == 0)
        {
            //----------------------------------
            // Consume trailer headers
            //----------------------------------

            while(true)
            {
                String trailerLine;

                if(ReadHttpLine(client, trailerLine, HttpMaxHeaderLineLength, timeoutMs) != true)
                {
                    errorMessage = "Unable to read HTTP chunk trailers.";

                    return false;
                }

                if(trailerLine.length() == 0)
                {
                    return true;
                }
            }
        }

        if(body.length() + chunkSize > HttpMaxBodyLength)
        {
            errorMessage = "HTTP response body exceeds maximum length.";

            return false;
        }

        if(ReadExactHttpBytes(client, body, (size_t)chunkSize, timeoutMs, errorMessage) != true)
        {
            return false;
        }

        //--------------------------------------
        // Consume CRLF following chunk data
        //--------------------------------------

        String chunkEnding;

        if(ReadHttpLine(client, chunkEnding, 2, timeoutMs) != true)
        {
            errorMessage = "Unable to read HTTP chunk ending.";

            return false;
        }

        if(chunkEnding.length() != 0)
        {
            errorMessage = "Invalid HTTP chunk ending.";

            return false;
        }
    }
}

//----------------------------------------------
// Build Authorization header
//----------------------------------------------

inline String BuildHeaderAuth(const String& token)
{
    return "Authorization: Bearer " + token;
}

//----------------------------------------------
// Build User-Agent header
//----------------------------------------------

inline String BuildUserAgent(const String& userAgent)
{
    return "User-Agent: " + userAgent;
}

