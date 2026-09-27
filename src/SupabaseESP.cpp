#include "SupabaseESP.h"

SupabaseESP::SupabaseESP()
    : _database(&_http),
      _auth(&_http),
      _storage(&_http),
      _lastErrorCode(0),
      _lastErrorMessage(""),
      _lastStatusCode(0) {}

SupabaseESP::~SupabaseESP() {}

bool SupabaseESP::begin(const char* ssid, const char* password, const char* supabaseUrl, const char* apiKey, uint32_t wifiTimeoutMs) {
    if (ssid != nullptr && strlen(ssid) > 0) {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid, password != nullptr ? password : "");
            uint32_t startMs = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - startMs < wifiTimeoutMs)) {
                delay(250);
            }
        }
        if (WiFi.status() != WL_CONNECTED) {
            _lastErrorCode = static_cast<int>(SupabaseErrorCode::WIFI_NOT_CONNECTED);
            _lastErrorMessage = "WiFi connection timeout";
            _lastStatusCode = 0;
            return false;
        }
#endif
    }

    if (supabaseUrl == nullptr || strlen(supabaseUrl) == 0) {
        _lastErrorCode = static_cast<int>(SupabaseErrorCode::INVALID_URL);
        _lastErrorMessage = "Supabase URL is invalid";
        _lastStatusCode = 0;
        return false;
    }

    _http.begin(supabaseUrl, apiKey != nullptr ? apiKey : "");
    _realtime.begin(supabaseUrl, apiKey != nullptr ? apiKey : "");
    _database.setHttp(&_http);
    _auth.setHttp(&_http);
    _storage.setHttp(&_http);

    _lastErrorCode = static_cast<int>(SupabaseErrorCode::NONE);
    _lastErrorMessage = "";
    _lastStatusCode = 200;
    return true;
}

bool SupabaseESP::begin(const String& ssid, const String& password, const String& supabaseUrl, const String& apiKey, uint32_t wifiTimeoutMs) {
    return begin(ssid.c_str(), password.c_str(), supabaseUrl.c_str(), apiKey.c_str(), wifiTimeoutMs);
}

bool SupabaseESP::begin(const char* supabaseUrl, const char* apiKey) {
    return begin(nullptr, nullptr, supabaseUrl, apiKey);
}

bool SupabaseESP::begin(const String& supabaseUrl, const String& apiKey) {
    return begin(nullptr, nullptr, supabaseUrl.c_str(), apiKey.c_str());
}

bool SupabaseESP::wifiConnected() const {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    return (WiFi.status() == WL_CONNECTED);
#else
    return true;
#endif
}

bool SupabaseESP::connected() const {
    return wifiConnected() && (_http.baseUrl().length() > 0) && (_http.apiKey().length() > 0);
}

void SupabaseESP::updateLastErrorFromResponse(const SupabaseResponse& resp) {
    _lastStatusCode = resp.statusCode();
    _lastErrorCode = static_cast<int>(resp.errorCode());
    if (resp.isSuccess()) {
        _lastErrorMessage = "";
    } else {
        String msg = "";
        int status = resp.statusCode();
        if (status == 401) {
            msg = "HTTP 401 Unauthorized";
        } else if (status == 404) {
            msg = "HTTP 404 Not Found";
        } else if (status == 500) {
            msg = "HTTP 500 Server Error";
        } else if (status > 0) {
            msg = "HTTP " + String(status);
            if (resp.payload().length() > 0 && resp.payload().startsWith("{")) {
                StaticJsonDocument<256> doc;
                if (deserializeJson(doc, resp.payload()) == DeserializationError::Ok) {
                    if (doc.containsKey("message")) {
                        msg += ": " + String(doc["message"].as<const char*>());
                    } else if (doc.containsKey("error_description")) {
                        msg += ": " + String(doc["error_description"].as<const char*>());
                    }
                }
            }
        } else {
            switch (resp.errorCode()) {
                case SupabaseErrorCode::WIFI_NOT_CONNECTED:
                    msg = "WiFi connection timeout";
                    break;
                case SupabaseErrorCode::INVALID_URL:
                    msg = "Supabase URL is invalid";
                    break;
                case SupabaseErrorCode::HTTP_ERROR:
                    msg = "HTTP connection failed";
                    break;
                case SupabaseErrorCode::TIMEOUT:
                    msg = "Request timeout";
                    break;
                default:
                    msg = supabaseErrorCodeToString(resp.errorCode());
                    break;
            }
        }
        _lastErrorMessage = msg;
    }
}

String SupabaseESP::select(const char* table) {
    return select(table, "*");
}

String SupabaseESP::select(const char* table, const char* columns) {
    if (table == nullptr) {
        _lastErrorCode = static_cast<int>(SupabaseErrorCode::INVALID_CONFIG);
        _lastErrorMessage = "Table name cannot be null";
        _lastStatusCode = 0;
        return "";
    }
    SupabaseResponse resp = _database.from(table).select(columns != nullptr ? columns : "*").execute();
    updateLastErrorFromResponse(resp);
    if (resp.isSuccess()) {
        return resp.payload();
    }
    return "";
}

String SupabaseESP::select(const String& table) {
    return select(table.c_str(), "*");
}

String SupabaseESP::select(const String& table, const String& columns) {
    return select(table.c_str(), columns.c_str());
}

bool SupabaseESP::insert(const char* table, const String& json) {
    if (table == nullptr) {
        _lastErrorCode = static_cast<int>(SupabaseErrorCode::INVALID_CONFIG);
        _lastErrorMessage = "Table name cannot be null";
        _lastStatusCode = 0;
        return false;
    }
    String endpoint = "/rest/v1/";
    endpoint += table;
    SupabaseResponse resp = _http.execute("POST", endpoint, {}, json);
    updateLastErrorFromResponse(resp);
    return resp.isSuccess();
}

bool SupabaseESP::insert(const String& table, const String& json) {
    return insert(table.c_str(), json);
}

bool SupabaseESP::update(const char* table, const String& json, const char* filter) {
    if (table == nullptr) {
        _lastErrorCode = static_cast<int>(SupabaseErrorCode::INVALID_CONFIG);
        _lastErrorMessage = "Table name cannot be null";
        _lastStatusCode = 0;
        return false;
    }
    String endpoint = "/rest/v1/";
    endpoint += table;
    if (filter != nullptr && strlen(filter) > 0) {
        if (filter[0] != '?') {
            endpoint += "?";
        }
        endpoint += filter;
    }
    SupabaseResponse resp = _http.execute("PATCH", endpoint, {}, json);
    updateLastErrorFromResponse(resp);
    return resp.isSuccess();
}

bool SupabaseESP::update(const String& table, const String& json, const String& filter) {
    return update(table.c_str(), json, filter.c_str());
}

bool SupabaseESP::remove(const char* table, const char* filter) {
    if (table == nullptr) {
        _lastErrorCode = static_cast<int>(SupabaseErrorCode::INVALID_CONFIG);
        _lastErrorMessage = "Table name cannot be null";
        _lastStatusCode = 0;
        return false;
    }
    String endpoint = "/rest/v1/";
    endpoint += table;
    if (filter != nullptr && strlen(filter) > 0) {
        if (filter[0] != '?') {
            endpoint += "?";
        }
        endpoint += filter;
    }
    SupabaseResponse resp = _http.execute("DELETE", endpoint, {}, "");
    updateLastErrorFromResponse(resp);
    return resp.isSuccess();
}

bool SupabaseESP::remove(const String& table, const String& filter) {
    return remove(table.c_str(), filter.c_str());
}

bool SupabaseESP::signUp(const char* email, const char* password) {
    if (email == nullptr || password == nullptr) return false;
    SupabaseResponse resp = _auth.signUp(email, password);
    updateLastErrorFromResponse(resp);
    return resp.isSuccess();
}

bool SupabaseESP::signUp(const String& email, const String& password) {
    return signUp(email.c_str(), password.c_str());
}

bool SupabaseESP::signIn(const char* email, const char* password) {
    if (email == nullptr || password == nullptr) return false;
    SupabaseResponse resp = _auth.signIn(email, password);
    updateLastErrorFromResponse(resp);
    return resp.isSuccess();
}

bool SupabaseESP::signIn(const String& email, const String& password) {
    return signIn(email.c_str(), password.c_str());
}

bool SupabaseESP::signOut() {
    SupabaseResponse resp = _auth.signOut();
    updateLastErrorFromResponse(resp);
    return resp.isSuccess();
}

int SupabaseESP::lastError() const {
    return _lastErrorCode;
}

String SupabaseESP::lastErrorMessage() const {
    return _lastErrorMessage;
}

int SupabaseESP::lastStatusCode() const {
    return _lastStatusCode;
}

SupabaseDatabase& SupabaseESP::db() {
    return _database;
}

SupabaseAuth& SupabaseESP::auth() {
    return _auth;
}

SupabaseStorage& SupabaseESP::storage() {
    return _storage;
}

SupabaseRealtime& SupabaseESP::realtime() {
    return _realtime;
}

SupabaseQuery SupabaseESP::from(const String& table) {
    return _database.from(table);
}

SupabaseQuery SupabaseESP::from(const String& schema, const String& table) {
    return _database.from(schema, table);
}

SupabaseQuery SupabaseESP::rpc(const String& functionName, JsonVariantConst params) {
    return _database.rpc(functionName, params);
}

SupabaseHttp& SupabaseESP::raw() {
    return _http;
}

void SupabaseESP::setTimeout(uint32_t timeoutMs) {
    _http.setTimeout(timeoutMs);
}

void SupabaseESP::setCACert(const char* caCert) {
    _http.setCACert(caCert);
}

void SupabaseESP::setInsecureTLS(bool insecure) {
    _http.setInsecureTLS(insecure);
}

void SupabaseESP::setMaxResponseSize(size_t maxBytes) {
    _http.setMaxResponseSize(maxBytes);
}

void SupabaseESP::setDebug(bool debug) {
    _http.setDebug(debug);
}

void SupabaseESP::setRequireMutationFilter(bool require) {
    _database.setRequireMutationFilter(require);
}

void SupabaseESP::setHeader(const String& name, const String& value) {
    _http.setHeader(name, value);
}

void SupabaseESP::setDeviceId(const String& deviceId) {
    _deviceId = deviceId;
}

const String& SupabaseESP::deviceId() const {
    return _deviceId;
}
