#pragma once
#ifndef SUPABASE_SESSION_H
#define SUPABASE_SESSION_H

#include <Arduino.h>

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>
#endif

/**
 * @brief Represents a Supabase Auth User Session.
 */
struct SupabaseSession {
    String accessToken;
    String refreshToken;
    String tokenType;
    uint32_t expiresIn;  // Duration in seconds
    uint64_t expiresAt;  // UNIX timestamp in seconds
    String userId;

    SupabaseSession()
        : tokenType("bearer"), expiresIn(0), expiresAt(0) {}

    bool isValid() const {
        return accessToken.length() > 0;
    }

    bool isExpired(uint32_t marginSeconds = 60, uint64_t nowSeconds = 0) const {
        if (!isValid()) return true;
        if (expiresAt == 0) return false;
        if (nowSeconds == 0) {
            // If nowSeconds not passed, check using millis() approximation if set
            nowSeconds = (uint64_t)time(NULL);
            if (nowSeconds < 1600000000ULL) return false; // Invalid NTP time, don't report expired based on time
        }
        return (nowSeconds + marginSeconds) >= expiresAt;
    }

    void clear() {
        accessToken = "";
        refreshToken = "";
        tokenType = "bearer";
        expiresIn = 0;
        expiresAt = 0;
        userId = "";
    }
};

/**
 * @brief Abstract interface for session persistence.
 */
class ISupabaseSessionStorage {
public:
    virtual ~ISupabaseSessionStorage() {}
    virtual bool load(SupabaseSession& session) = 0;
    virtual bool save(const SupabaseSession& session) = 0;
    virtual bool clear() = 0;
};

/**
 * @brief In-memory session storage (RAM only, default).
 */
class MemorySessionStorage : public ISupabaseSessionStorage {
public:
    MemorySessionStorage() {}
    
    bool load(SupabaseSession& session) override {
        session = _session;
        return session.isValid();
    }

    bool save(const SupabaseSession& session) override {
        _session = session;
        return true;
    }

    bool clear() override {
        _session.clear();
        return true;
    }

private:
    SupabaseSession _session;
};

/**
 * @brief Persistent session storage using ESP32 Preferences (NVS).
 */
class PreferencesSessionStorage : public ISupabaseSessionStorage {
public:
    PreferencesSessionStorage(const char* namespaceName = "supabase")
        : _namespace(namespaceName) {}

    bool load(SupabaseSession& session) override {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
        Preferences prefs;
        if (!prefs.begin(_namespace.c_str(), true)) {
            return false;
        }
        session.accessToken = prefs.getString("acc_tok", "");
        session.refreshToken = prefs.getString("ref_tok", "");
        session.tokenType = prefs.getString("tok_typ", "bearer");
        session.expiresIn = prefs.getUInt("exp_in", 0);
        session.expiresAt = (uint64_t)prefs.getULongLong("exp_at", 0);
        session.userId = prefs.getString("usr_id", "");
        prefs.end();
        return session.isValid();
#else
        return false;
#endif
    }

    bool save(const SupabaseSession& session) override {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
        Preferences prefs;
        if (!prefs.begin(_namespace.c_str(), false)) {
            return false;
        }
        prefs.putString("acc_tok", session.accessToken);
        prefs.putString("ref_tok", session.refreshToken);
        prefs.putString("tok_typ", session.tokenType);
        prefs.putUInt("exp_in", session.expiresIn);
        prefs.putULongLong("exp_at", (unsigned long long)session.expiresAt);
        prefs.putString("usr_id", session.userId);
        prefs.end();
        return true;
#else
        return false;
#endif
    }

    bool clear() override {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
        Preferences prefs;
        if (!prefs.begin(_namespace.c_str(), false)) {
            return false;
        }
        prefs.clear();
        prefs.end();
        return true;
#else
        return true;
#endif
    }

private:
    String _namespace;
};

#endif // SUPABASE_SESSION_H
