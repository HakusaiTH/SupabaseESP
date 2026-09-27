#include "SupabaseRealtime.h"

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
SupabaseRealtime* SupabaseRealtime::_instance = nullptr;
#endif

// --- SupabaseRealtimeChannel ---

SupabaseRealtimeChannel::SupabaseRealtimeChannel(SupabaseRealtime* manager, const String& topic)
    : _manager(manager), _topic("realtime:" + topic), _isJoined(false) {}

const String& SupabaseRealtimeChannel::topic() const {
    return _topic;
}

bool SupabaseRealtimeChannel::isJoined() const {
    return _isJoined;
}

void SupabaseRealtimeChannel::setJoined(bool joined) {
    _isJoined = joined;
}

String SupabaseRealtimeChannel::changeEventToString(SupabaseChangeEvent evt) {
    switch (evt) {
        case SupabaseChangeEvent::Insert: return "INSERT";
        case SupabaseChangeEvent::Update: return "UPDATE";
        case SupabaseChangeEvent::Delete: return "DELETE";
        case SupabaseChangeEvent::All:    return "*";
        default:                           return "*";
    }
}

void SupabaseRealtimeChannel::join() {
    if (_manager == nullptr) return;

    JsonDocument doc;
    JsonObject config = doc["config"].to<JsonObject>();
    
    // Broadcast config
    JsonObject broadcastConfig = config["broadcast"].to<JsonObject>();
    broadcastConfig["self"] = true;

    // Presence config
    if (_presenceKey.length() > 0) {
        JsonObject presenceConfig = config["presence"].to<JsonObject>();
        presenceConfig["key"] = _presenceKey;
    }

    // Postgres changes config
    if (!_postgresHandlers.empty()) {
        JsonArray pgArr = config["postgres_changes"].to<JsonArray>();
        for (const auto& handler : _postgresHandlers) {
            JsonObject item = pgArr.add<JsonObject>();
            item["event"] = changeEventToString(handler.event);
            item["schema"] = handler.schema;
            item["table"] = handler.table;
            if (handler.filter.length() > 0) {
                item["filter"] = handler.filter;
            }
        }
    }

    if (_manager->accessToken().length() > 0) {
        doc["user_token"] = _manager->accessToken();
    }

    _manager->sendPhxMessage(_topic, "phx_join", doc.as<JsonVariantConst>());
}

void SupabaseRealtimeChannel::leave() {
    if (_manager == nullptr || !_isJoined) return;
    JsonDocument emptyDoc;
    _manager->sendPhxMessage(_topic, "phx_leave", emptyDoc.as<JsonVariantConst>());
    _isJoined = false;
}

void SupabaseRealtimeChannel::sendBroadcast(const String& event, JsonVariantConst payload) {
    if (_manager == nullptr) return;

    JsonDocument doc;
    doc["type"] = "broadcast";
    doc["event"] = event;
    doc["payload"] = payload;

    _manager->sendPhxMessage(_topic, "broadcast", doc.as<JsonVariantConst>());
}

void SupabaseRealtimeChannel::onBroadcast(const String& event, BroadcastCallback callback) {
    _broadcastHandlers.push_back({event, callback});
}

void SupabaseRealtimeChannel::onPostgresChange(
    SupabaseChangeEvent event,
    const String& schema,
    const String& table,
    PostgresChangeCallback callback
) {
    onPostgresChange(event, schema, table, "", callback);
}

void SupabaseRealtimeChannel::onPostgresChange(
    SupabaseChangeEvent event,
    const String& schema,
    const String& table,
    const String& filter,
    PostgresChangeCallback callback
) {
    _postgresHandlers.push_back({event, schema, table, filter, callback});
}

void SupabaseRealtimeChannel::enablePresence(const String& key) {
    _presenceKey = key;
}

void SupabaseRealtimeChannel::track(JsonVariantConst state) {
    if (_manager == nullptr) return;

    JsonDocument doc;
    doc["type"] = "presence";
    doc["event"] = "TRACK";
    doc["payload"] = state;

    _manager->sendPhxMessage(_topic, "presence", doc.as<JsonVariantConst>());
}

void SupabaseRealtimeChannel::onPresenceState(PresenceCallback callback) {
    _presenceStateCallback = callback;
}

void SupabaseRealtimeChannel::onPresenceJoin(PresenceCallback callback) {
    _presenceJoinCallback = callback;
}

void SupabaseRealtimeChannel::onPresenceLeave(PresenceCallback callback) {
    _presenceLeaveCallback = callback;
}

void SupabaseRealtimeChannel::handleIncomingMessage(const String& eventName, const JsonObjectConst& payload) {
    if (eventName == "broadcast") {
        if (payload.containsKey("event") && payload.containsKey("payload")) {
            String evtStr = payload["event"].as<String>();
            JsonObjectConst bPayload = payload["payload"].as<JsonObjectConst>();
            for (const auto& h : _broadcastHandlers) {
                if (h.event == evtStr || h.event == "*") {
                    if (h.callback) h.callback(bPayload);
                }
            }
        }
    } else if (eventName == "postgres_changes") {
        if (payload.containsKey("data") && payload["data"].is<JsonObjectConst>()) {
            JsonObjectConst data = payload["data"].as<JsonObjectConst>();
            SupabasePostgresChange change;
            
            String evtStr = data["type"].as<String>();
            if (evtStr == "INSERT") change.event = SupabaseChangeEvent::Insert;
            else if (evtStr == "UPDATE") change.event = SupabaseChangeEvent::Update;
            else if (evtStr == "DELETE") change.event = SupabaseChangeEvent::Delete;
            else change.event = SupabaseChangeEvent::All;

            change.schema = data["schema"].as<String>();
            change.table = data["table"].as<String>();
            
            if (data.containsKey("record") && !data["record"].isNull()) {
                deserializeJson(change.record, data["record"]);
            }
            if (data.containsKey("old_record") && !data["old_record"].isNull()) {
                deserializeJson(change.oldRecord, data["old_record"]);
            }
            if (data.containsKey("commit_timestamp") && !data["commit_timestamp"].isNull()) {
                change.commitTimestamp = data["commit_timestamp"].as<String>();
            }

            for (const auto& h : _postgresHandlers) {
                if ((h.schema == change.schema || h.schema == "*") &&
                    (h.table == change.table || h.table == "*")) {
                    if (h.event == SupabaseChangeEvent::All || h.event == change.event) {
                        if (h.callback) h.callback(change);
                    }
                }
            }
        }
    } else if (eventName == "presence_state") {
        if (_presenceStateCallback) _presenceStateCallback(payload);
    } else if (eventName == "presence_diff") {
        if (payload.containsKey("joins") && _presenceJoinCallback) {
            _presenceJoinCallback(payload["joins"].as<JsonObjectConst>());
        }
        if (payload.containsKey("leaves") && _presenceLeaveCallback) {
            _presenceLeaveCallback(payload["leaves"].as<JsonObjectConst>());
        }
    }
}


// --- SupabaseRealtime Manager ---

SupabaseRealtime::SupabaseRealtime()
    : _state(RealtimeState::DISCONNECTED),
      _heartbeatIntervalMs(30000),
      _lastHeartbeatMs(0),
      _reconnectMaxDelayMs(30000),
      _currentBackoffMs(1000),
      _lastReconnectAttemptMs(0),
      _messageRef(1) {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    _instance = this;
#endif
}

SupabaseRealtime::~SupabaseRealtime() {
    for (auto& pair : _channels) {
        delete pair.second;
    }
    _channels.clear();
}

void SupabaseRealtime::begin(const String& baseUrl, const String& apiKey) {
    _baseUrl = baseUrl;
    _apiKey = apiKey;
}

void SupabaseRealtime::setAccessToken(const String& token) {
    _accessToken = token;
    if (_state == RealtimeState::CONNECTED || _state == RealtimeState::JOINED) {
        for (auto& pair : _channels) {
            if (pair.second->isJoined()) {
                JsonDocument doc;
                doc["access_token"] = _accessToken;
                sendPhxMessage(pair.second->topic(), "access_token", doc.as<JsonVariantConst>());
            }
        }
    }
}

const String& SupabaseRealtime::accessToken() const {
    return _accessToken;
}

SupabaseRealtimeChannel& SupabaseRealtime::channel(const String& name) {
    if (_channels.find(name) == _channels.end()) {
        _channels[name] = new SupabaseRealtimeChannel(this, name);
    }
    return *(_channels[name]);
}

void SupabaseRealtime::removeChannel(const String& name) {
    auto it = _channels.find(name);
    if (it != _channels.end()) {
        it->second->leave();
        delete it->second;
        _channels.erase(it);
    }
}

void SupabaseRealtime::onStateChange(RealtimeStateCallback callback) {
    _stateCallback = callback;
}

RealtimeState SupabaseRealtime::state() const {
    return _state;
}

void SupabaseRealtime::setHeartbeatInterval(uint32_t seconds) {
    _heartbeatIntervalMs = seconds * 1000;
}

void SupabaseRealtime::setReconnectMaxDelay(uint32_t ms) {
    _reconnectMaxDelayMs = ms;
}

void SupabaseRealtime::updateState(RealtimeState newState) {
    if (_state != newState) {
        _state = newState;
        if (_stateCallback) {
            _stateCallback(_state);
        }
    }
}

String SupabaseRealtime::nextRef() {
    return String(_messageRef++);
}

String SupabaseRealtime::buildWsUrl() const {
    String host = _baseUrl;
    host.replace("https://", "");
    host.replace("http://", "");
    while (host.endsWith("/")) {
        host.remove(host.length() - 1);
    }
    return "/realtime/v1/websocket?apikey=" + _apiKey + "&vsn=1.0.0";
}

void SupabaseRealtime::sendPhxMessage(const String& topic, const String& event, JsonVariantConst payload, const String& ref) {
    String messageRef = ref;
    if (messageRef.length() == 0) {
        messageRef = nextRef();
    }

    JsonDocument doc;
    doc["topic"] = topic;
    doc["event"] = event;
    doc["payload"] = payload;
    doc["ref"] = messageRef;

    if (event == "phx_join") {
        doc["join_ref"] = messageRef;
    }

    String jsonStr;
    serializeJson(doc, jsonStr);

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    if (_state == RealtimeState::CONNECTED || _state == RealtimeState::JOINED) {
        _webSocket.sendTXT(jsonStr);
    }
#endif
}

void SupabaseRealtime::sendHeartbeat() {
    JsonDocument emptyPayload;
    sendPhxMessage("phoenix", "heartbeat", emptyPayload.as<JsonVariantConst>());
    _lastHeartbeatMs = millis();
}

void SupabaseRealtime::processWebSocketMessage(const String& msg) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) return;

    String topic = doc["topic"].as<String>();
    String event = doc["event"].as<String>();
    JsonObjectConst payload = doc["payload"].as<JsonObjectConst>();

    if (event == "phx_reply") {
        String status = payload["status"].as<String>();
        for (auto& pair : _channels) {
            if (pair.second->topic() == topic) {
                if (status == "ok") {
                    pair.second->setJoined(true);
                    updateState(RealtimeState::JOINED);
                }
            }
        }
    } else {
        for (auto& pair : _channels) {
            if (pair.second->topic() == topic) {
                pair.second->handleIncomingMessage(event, payload);
            }
        }
    }
}

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
void SupabaseRealtime::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    if (_instance == nullptr) return;

    switch (type) {
        case WStype_DISCONNECTED:
            _instance->updateState(RealtimeState::DISCONNECTED);
            break;
        case WStype_CONNECTED:
            _instance->updateState(RealtimeState::CONNECTED);
            _instance->_currentBackoffMs = 1000;
            // Auto rejoin channels
            for (auto& pair : _instance->_channels) {
                pair.second->join();
            }
            break;
        case WStype_TEXT:
            if (payload != nullptr && length > 0) {
                String msg = String((char*)payload, length);
                _instance->processWebSocketMessage(msg);
            }
            break;
        default:
            break;
    }
}
#endif

void SupabaseRealtime::loop() {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    uint32_t now = millis();

    if (_state == RealtimeState::DISCONNECTED) {
        if (now - _lastReconnectAttemptMs >= _currentBackoffMs) {
            _lastReconnectAttemptMs = now;
            updateState(RealtimeState::RECONNECTING);
            
            String host = _baseUrl;
            host.replace("https://", "");
            host.replace("http://", "");
            int port = 443;
            String path = buildWsUrl();

            _webSocket.beginSSL(host.c_str(), port, path.c_str(), "", "wss");
            _webSocket.onEvent(webSocketEvent);
            _webSocket.setReconnectInterval(5000);

            _currentBackoffMs = min(_currentBackoffMs * 2, _reconnectMaxDelayMs);
        }
    } else if (_state == RealtimeState::CONNECTED || _state == RealtimeState::JOINED) {
        _webSocket.loop();
        if (now - _lastHeartbeatMs >= _heartbeatIntervalMs) {
            sendHeartbeat();
        }
    }
#endif
}
