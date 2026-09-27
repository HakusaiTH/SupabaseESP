#pragma once
#ifndef SUPABASE_REALTIME_H
#define SUPABASE_REALTIME_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <vector>
#include <map>

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <WebSocketsClient.h>
#endif

enum class RealtimeState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    JOINING,
    JOINED,
    RECONNECTING,
    ERROR
};

enum class SupabaseChangeEvent {
    Insert,
    Update,
    Delete,
    All
};

struct SupabasePostgresChange {
    SupabaseChangeEvent event;
    String schema;
    String table;
    JsonDocument record;
    JsonDocument oldRecord;
    String commitTimestamp;
};

typedef std::function<void(const JsonObjectConst& payload)> BroadcastCallback;
typedef std::function<void(const SupabasePostgresChange& change)> PostgresChangeCallback;
typedef std::function<void(const JsonObjectConst& state)> PresenceCallback;
typedef std::function<void(RealtimeState state)> RealtimeStateCallback;

struct BroadcastHandler {
    String event;
    BroadcastCallback callback;
};

struct PostgresChangeHandler {
    SupabaseChangeEvent event;
    String schema;
    String table;
    String filter;
    PostgresChangeCallback callback;
};

class SupabaseRealtime;

/**
 * @brief Represents a Supabase Realtime channel (Broadcast, Presence, Postgres Changes).
 */
class SupabaseRealtimeChannel {
public:
    SupabaseRealtimeChannel(SupabaseRealtime* manager, const String& topic);

    const String& topic() const;

    void join();
    void leave();

    // Broadcast
    void sendBroadcast(const String& event, JsonVariantConst payload);
    void onBroadcast(const String& event, BroadcastCallback callback);

    // Postgres Changes
    void onPostgresChange(
        SupabaseChangeEvent event,
        const String& schema,
        const String& table,
        PostgresChangeCallback callback
    );

    void onPostgresChange(
        SupabaseChangeEvent event,
        const String& schema,
        const String& table,
        const String& filter,
        PostgresChangeCallback callback
    );

    // Presence
    void enablePresence(const String& key);
    void track(JsonVariantConst state);
    void onPresenceState(PresenceCallback callback);
    void onPresenceJoin(PresenceCallback callback);
    void onPresenceLeave(PresenceCallback callback);

    // Internal dispatchers
    void handleIncomingMessage(const String& eventName, const JsonObjectConst& payload);
    bool isJoined() const;
    void setJoined(bool joined);

private:
    SupabaseRealtime* _manager;
    String _topic;
    bool _isJoined;
    String _presenceKey;

    std::vector<BroadcastHandler> _broadcastHandlers;
    std::vector<PostgresChangeHandler> _postgresHandlers;
    
    PresenceCallback _presenceStateCallback;
    PresenceCallback _presenceJoinCallback;
    PresenceCallback _presenceLeaveCallback;

    String changeEventToString(SupabaseChangeEvent evt);
};

/**
 * @brief Main Realtime Manager class handling WebSocket connections, heartbeat, and reconnects.
 */
class SupabaseRealtime {
public:
    SupabaseRealtime();
    ~SupabaseRealtime();

    void begin(const String& baseUrl, const String& apiKey);
    void setAccessToken(const String& token);
    const String& accessToken() const;

    SupabaseRealtimeChannel& channel(const String& name);
    void removeChannel(const String& name);

    void onStateChange(RealtimeStateCallback callback);
    RealtimeState state() const;

    void setHeartbeatInterval(uint32_t seconds);
    void setReconnectMaxDelay(uint32_t ms);

    void loop();

    // Low level message sender
    void sendPhxMessage(const String& topic, const String& event, JsonVariantConst payload, const String& ref = "");

private:
    String _baseUrl;
    String _apiKey;
    String _accessToken;

    RealtimeState _state;
    RealtimeStateCallback _stateCallback;

    uint32_t _heartbeatIntervalMs;
    uint32_t _lastHeartbeatMs;
    
    uint32_t _reconnectMaxDelayMs;
    uint32_t _currentBackoffMs;
    uint32_t _lastReconnectAttemptMs;

    uint32_t _messageRef;

    std::map<String, SupabaseRealtimeChannel*> _channels;

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    WebSocketsClient _webSocket;
    static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
    static SupabaseRealtime* _instance;
#endif

    void updateState(RealtimeState newState);
    void sendHeartbeat();
    void processWebSocketMessage(const String& msg);
    String nextRef();
    String buildWsUrl() const;
};

#endif // SUPABASE_REALTIME_H
