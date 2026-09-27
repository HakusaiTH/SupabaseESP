#include "SupabaseHttp.h"

SupabaseHttp::SupabaseHttp()
    : _timeoutMs(10000),
      _caCert(nullptr),
      _insecureTLS(false),
      _maxResponseSize(16384),
      _debug(false) {}

SupabaseHttp::~SupabaseHttp() {}

void SupabaseHttp::begin(const String& baseUrl, const String& apiKey) {
    _baseUrl = baseUrl;
    // Trim trailing slashes from base URL
    while (_baseUrl.endsWith("/")) {
        _baseUrl.remove(_baseUrl.length() - 1);
    }
    _apiKey = apiKey;
}

const String& SupabaseHttp::baseUrl() const {
    return _baseUrl;
}

const String& SupabaseHttp::apiKey() const {
    return _apiKey;
}

void SupabaseHttp::setAccessToken(const String& token) {
    _accessToken = token;
}

const String& SupabaseHttp::accessToken() const {
    return _accessToken;
}

void SupabaseHttp::setHeader(const String& name, const String& value) {
    for (auto& h : _customHeaders) {
        if (h.name.equalsIgnoreCase(name)) {
            h.value = value;
            return;
        }
    }
    _customHeaders.push_back({name, value});
}

void SupabaseHttp::clearCustomHeaders() {
    _customHeaders.clear();
}

void SupabaseHttp::setTimeout(uint32_t timeoutMs) {
    _timeoutMs = timeoutMs;
}

uint32_t SupabaseHttp::timeout() const {
    return _timeoutMs;
}

void SupabaseHttp::setCACert(const char* caCert) {
    _caCert = caCert;
}

void SupabaseHttp::setInsecureTLS(bool insecure) {
    _insecureTLS = insecure;
    if (_insecureTLS && _debug) {
        Serial.println("[SupabaseESP][WARNING] Insecure TLS mode enabled! Certificate verification disabled.");
    }
}

bool SupabaseHttp::isInsecureTLS() const {
    return _insecureTLS;
}

void SupabaseHttp::setMaxResponseSize(size_t bytes) {
    _maxResponseSize = bytes;
}

size_t SupabaseHttp::maxResponseSize() const {
    return _maxResponseSize;
}

void SupabaseHttp::setDebug(bool debug) {
    _debug = debug;
}

bool SupabaseHttp::isDebug() const {
    return _debug;
}

String SupabaseHttp::sanitizeHeaderForLog(const String& name, const String& value) const {
    String lower = name;
    lower.toLowerCase();
    if (lower == "apikey" || lower == "authorization" || lower == "x-api-key") {
        return "[REDACTED]";
    }
    return value;
}

String SupabaseHttp::buildFullUrl(const String& endpointOrUrl) const {
    if (endpointOrUrl.startsWith("http://") || endpointOrUrl.startsWith("https://")) {
        return endpointOrUrl;
    }
    String url = _baseUrl;
    if (!endpointOrUrl.startsWith("/")) {
        url += "/";
    }
    url += endpointOrUrl;
    return url;
}

SupabaseResponse SupabaseHttp::execute(
    const String& method,
    const String& endpointOrUrl,
    const std::vector<SupabaseHeader>& requestHeaders,
    const String& payload
) {
    return executeStream(method, endpointOrUrl, requestHeaders, nullptr, 0, nullptr);
}

SupabaseResponse SupabaseHttp::executeStream(
    const String& method,
    const String& endpointOrUrl,
    const std::vector<SupabaseHeader>& requestHeaders,
    Stream* inputStream,
    size_t inputStreamSize,
    Stream* outputStream
) {
    uint32_t startMs = millis();
    String fullUrl = buildFullUrl(endpointOrUrl);

    if (_debug) {
        Serial.printf("[SupabaseESP] %s -> %s\n", method.c_str(), fullUrl.c_str());
    }

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    WiFiClientSecure client;
    if (_insecureTLS) {
        client.setInsecure();
    } else if (_caCert != nullptr) {
        client.setCACert(_caCert);
    } else {
        // Default: insecure without CA cert unless provided, but user must be aware
        // To be secure by default on ESP32 without root cert bundle, we allow connection
        // or setInsecure if no root set, but print debug warning if TLS insecure
        client.setInsecure(); 
    }

    HTTPClient http;
    http.setTimeout(_timeoutMs);
    if (!http.begin(client, fullUrl)) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_URL, millis() - startMs);
    }

    // Default Supabase Headers
    bool hasApiKey = false;
    bool hasAuth = false;
    bool hasContentType = false;
    bool hasAccept = false;

    // Apply custom client-wide headers
    for (const auto& h : _customHeaders) {
        http.addHeader(h.name, h.value);
        if (h.name.equalsIgnoreCase("apikey")) hasApiKey = true;
        if (h.name.equalsIgnoreCase("Authorization")) hasAuth = true;
        if (h.name.equalsIgnoreCase("Content-Type")) hasContentType = true;
        if (h.name.equalsIgnoreCase("Accept")) hasAccept = true;
    }

    // Apply request-specific headers
    for (const auto& h : requestHeaders) {
        http.addHeader(h.name, h.value);
        if (h.name.equalsIgnoreCase("apikey")) hasApiKey = true;
        if (h.name.equalsIgnoreCase("Authorization")) hasAuth = true;
        if (h.name.equalsIgnoreCase("Content-Type")) hasContentType = true;
        if (h.name.equalsIgnoreCase("Accept")) hasAccept = true;
    }

    // Add apikey if missing
    if (!hasApiKey && _apiKey.length() > 0) {
        http.addHeader("apikey", _apiKey);
    }

    // Add Authorization if missing
    if (!hasAuth) {
        if (_accessToken.length() > 0) {
            http.addHeader("Authorization", "Bearer " + _accessToken);
        } else if (_apiKey.length() > 0) {
            http.addHeader("Authorization", "Bearer " + _apiKey);
        }
    }

    if (!hasContentType && (method.equals("POST") || method.equals("PATCH") || method.equals("PUT"))) {
        http.addHeader("Content-Type", "application/json");
    }

    if (!hasAccept) {
        http.addHeader("Accept", "application/json");
    }

    // Request collect response headers
    const char* headerKeys[] = {"Retry-After", "Content-Length"};
    http.collectHeaders(headerKeys, 2);

    int httpCode = 0;
    if (inputStream != nullptr && inputStreamSize > 0) {
        httpCode = http.sendRequest(method.c_str(), inputStream, inputStreamSize);
    } else {
        httpCode = http.sendRequest(method.c_str(), (uint8_t*)payload.c_str(), payload.length());
    }

    uint32_t elapsed = millis() - startMs;

    if (httpCode <= 0) {
        if (_debug) {
            Serial.printf("[SupabaseESP] Connection failed, error: %s (%d)\n", http.errorToString(httpCode).c_str(), httpCode);
        }
        http.end();
        return SupabaseResponse(httpCode, "", SupabaseErrorCode::HTTP_ERROR, elapsed);
    }

    int contentLength = http.getSize();
    if (_maxResponseSize > 0 && contentLength > (int)_maxResponseSize) {
        if (_debug) {
            Serial.printf("[SupabaseESP] Response body size (%d) exceeds max response limit (%d)\n", contentLength, (int)_maxResponseSize);
        }
        http.end();
        return SupabaseResponse(httpCode, "", SupabaseErrorCode::RESPONSE_TOO_LARGE, elapsed);
    }

    String responseBody = "";
    if (outputStream != nullptr) {
        WiFiClient* stream = http.getStreamPtr();
        uint8_t buff[256] = { 0 };
        int len = contentLength;
        while (http.connected() && (len > 0 || len == -1)) {
            size_t size = stream->available();
            if (size) {
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                outputStream->write(buff, c);
                if (len > 0) {
                    len -= c;
                }
            }
            delay(1);
        }
    } else {
        responseBody = http.getString();
        if (_maxResponseSize > 0 && responseBody.length() > _maxResponseSize) {
            http.end();
            return SupabaseResponse(httpCode, "", SupabaseErrorCode::RESPONSE_TOO_LARGE, elapsed);
        }
    }

    SupabaseResponse resp(httpCode, responseBody, SupabaseErrorCode::NONE, elapsed);

    if (http.hasHeader("Retry-After")) {
        String retryStr = http.header("Retry-After");
        resp.setRetryAfter(retryStr.toInt());
    }

    if (_debug) {
        Serial.printf("[SupabaseESP] Response status: %d (elapsed %d ms, size %d bytes)\n", httpCode, elapsed, responseBody.length());
    }

    http.end();
    return resp;
#else
    // Non-ESP32 fallback (simulation/mock testing layer)
    return SupabaseResponse(200, "{}", SupabaseErrorCode::NONE, millis() - startMs);
#endif
}
