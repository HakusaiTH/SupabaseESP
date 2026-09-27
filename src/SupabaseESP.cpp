#include "SupabaseESP.h"

SupabaseESP::SupabaseESP()
    : _database(&_http),
      _auth(&_http),
      _storage(&_http) {}

SupabaseESP::~SupabaseESP() {}

void SupabaseESP::begin(const String& url, const String& apiKey) {
    _http.begin(url, apiKey);
    _realtime.begin(url, apiKey);
    _database.setHttp(&_http);
    _auth.setHttp(&_http);
    _storage.setHttp(&_http);
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
