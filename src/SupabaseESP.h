#ifndef SUPABASE_ESP_H
#define SUPABASE_ESP_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SupabaseError.h"
#include "SupabaseResponse.h"
#include "SupabaseUrlEncoder.h"
#include "SupabaseSession.h"
#include "SupabaseHttp.h"
#include "SupabaseQuery.h"
#include "SupabaseDatabase.h"
#include "SupabaseAuth.h"
#include "SupabaseStorage.h"
#include "SupabaseRealtime.h"

/**
 * @brief Primary entry point for SupabaseESP client library on ESP32.
 */
class SupabaseESP {
public:
    SupabaseESP();
    ~SupabaseESP();

    // Initialization
    void begin(const String& url, const String& apiKey);

    // Submodule accessors
    SupabaseDatabase& db();
    SupabaseAuth& auth();
    SupabaseStorage& storage();
    SupabaseRealtime& realtime();

    // Direct Database REST shortcuts
    SupabaseQuery from(const String& table);
    SupabaseQuery from(const String& schema, const String& table);
    SupabaseQuery rpc(const String& functionName, JsonVariantConst params = JsonVariantConst());

    // Low-level HTTP raw escape hatch
    SupabaseHttp& raw();

    // Global configuration options
    void setTimeout(uint32_t timeoutMs);
    void setCACert(const char* caCert);
    void setInsecureTLS(bool insecure);
    void setMaxResponseSize(size_t maxBytes);
    void setDebug(bool debug);
    void setRequireMutationFilter(bool require);
    void setHeader(const String& name, const String& value);

    // Device identity helpers
    void setDeviceId(const String& deviceId);
    const String& deviceId() const;

private:
    SupabaseHttp _http;
    SupabaseDatabase _database;
    SupabaseAuth _auth;
    SupabaseStorage _storage;
    SupabaseRealtime _realtime;

    String _deviceId;
};

#endif // SUPABASE_ESP_H
