#pragma once
#ifndef SUPABASE_AUTH_H
#define SUPABASE_AUTH_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SupabaseHttp.h"
#include "SupabaseSession.h"
#include "SupabaseResponse.h"

/**
 * @brief Submodule for Supabase GoTrue Authentication.
 */
class SupabaseAuth {
public:
    SupabaseAuth();
    explicit SupabaseAuth(SupabaseHttp* http);
    ~SupabaseAuth();

    void setHttp(SupabaseHttp* http);
    void setStorage(ISupabaseSessionStorage* storage);

    SupabaseResponse signUp(const String& email, const String& password);
    SupabaseResponse signIn(const String& email, const String& password);
    SupabaseResponse signInWithPassword(const String& email, const String& password);

    SupabaseResponse refreshSession();
    SupabaseResponse refreshSession(const String& refreshToken);

    SupabaseResponse getUser();
    SupabaseResponse signOut();

    void setAutoRefresh(bool autoRefresh);
    bool isAutoRefresh() const;

    void setRefreshMargin(uint32_t seconds);
    uint32_t refreshMargin() const;

    const SupabaseSession& session() const;
    bool isAuthenticated() const;

    // Checks and performs auto-refresh if token near expiration
    bool checkAutoRefresh();

private:
    SupabaseHttp* _http;
    ISupabaseSessionStorage* _storage;
    MemorySessionStorage _defaultMemoryStorage;
    SupabaseSession _session;

    bool _autoRefresh;
    uint32_t _refreshMarginSeconds;

    void updateSessionFromResponse(const JsonObjectConst& json);
    void saveSession();
    void loadSession();
    void clearSession();
};

#endif // SUPABASE_AUTH_H
