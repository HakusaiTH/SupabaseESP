#ifndef SUPABASE_HTTP_H
#define SUPABASE_HTTP_H

#include <Arduino.h>
#include <map>
#include <vector>

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#endif

#include "SupabaseResponse.h"
#include "SupabaseError.h"

struct SupabaseHeader {
    String name;
    String value;
};

/**
 * @brief HTTP Client transport manager for Supabase REST, Auth, and Storage APIs.
 */
class SupabaseHttp {
public:
    SupabaseHttp();
    ~SupabaseHttp();

    void begin(const String& baseUrl, const String& apiKey);
    
    const String& baseUrl() const;
    const String& apiKey() const;

    void setAccessToken(const String& token);
    const String& accessToken() const;

    void setHeader(const String& name, const String& value);
    void clearCustomHeaders();

    void setTimeout(uint32_t timeoutMs);
    uint32_t timeout() const;

    void setCACert(const char* caCert);
    void setInsecureTLS(bool insecure);
    bool isInsecureTLS() const;

    void setMaxResponseSize(size_t bytes);
    size_t maxResponseSize() const;

    void setDebug(bool debug);
    bool isDebug() const;

    // HTTP method execution
    SupabaseResponse execute(
        const String& method,
        const String& endpointOrUrl,
        const std::vector<SupabaseHeader>& requestHeaders = {},
        const String& payload = ""
    );

    // Streaming HTTP upload/download execution
    SupabaseResponse executeStream(
        const String& method,
        const String& endpointOrUrl,
        const std::vector<SupabaseHeader>& requestHeaders,
        Stream* inputStream,
        size_t inputStreamSize,
        Stream* outputStream
    );

private:
    String _baseUrl;
    String _apiKey;
    String _accessToken;
    std::vector<SupabaseHeader> _customHeaders;
    
    uint32_t _timeoutMs;
    const char* _caCert;
    bool _insecureTLS;
    size_t _maxResponseSize;
    bool _debug;

    String sanitizeHeaderForLog(const String& name, const String& value) const;
    String buildFullUrl(const String& endpointOrUrl) const;
};

#endif // SUPABASE_HTTP_H
