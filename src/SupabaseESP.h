#pragma once
#ifndef SUPABASE_ESP_H
#define SUPABASE_ESP_H

#include <Arduino.h>
#include <ArduinoJson.h>

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

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

    // WiFi + Supabase Initialization
    bool begin(
        const char* ssid,
        const char* password,
        const char* supabaseUrl,
        const char* apiKey,
        uint32_t wifiTimeoutMs = 15000
    );

    bool begin(
        const String& ssid,
        const String& password,
        const String& supabaseUrl,
        const String& apiKey,
        uint32_t wifiTimeoutMs = 15000
    );

    // Initialization when WiFi is already connected
    bool begin(
        const char* supabaseUrl,
        const char* apiKey
    );

    bool begin(
        const String& supabaseUrl,
        const String& apiKey
    );

    // Connection status
    bool connected() const;
    bool wifiConnected() const;

    // Database REST API shortcuts
    String select(const char* table);
    String select(const char* table, const char* columns);
    String select(const String& table);
    String select(const String& table, const String& columns);

    bool insert(const char* table, const String& json);
    bool insert(const String& table, const String& json);

    bool update(const char* table, const String& json, const char* filter);
    bool update(const String& table, const String& json, const String& filter);

    bool remove(const char* table, const char* filter);
    bool remove(const String& table, const String& filter);

    // Auth shortcuts
    bool signUp(const char* email, const char* password);
    bool signUp(const String& email, const String& password);

    bool signIn(const char* email, const char* password);
    bool signIn(const String& email, const String& password);

    bool signOut();

    // Error handling
    int lastError() const;
    String lastErrorMessage() const;
    int lastStatusCode() const;

    // Submodule accessors
    SupabaseDatabase& db();
    SupabaseAuth& auth();
    SupabaseStorage& storage();
    SupabaseRealtime& realtime();

    // Direct Fluent Query builder shortcuts
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
    int _lastErrorCode;
    String _lastErrorMessage;
    int _lastStatusCode;

    void updateLastErrorFromResponse(const SupabaseResponse& resp);
};

#endif // SUPABASE_ESP_H
