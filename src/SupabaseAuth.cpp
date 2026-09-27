#include "SupabaseAuth.h"

SupabaseAuth::SupabaseAuth()
    : _http(nullptr),
      _storage(&_defaultMemoryStorage),
      _autoRefresh(true),
      _refreshMarginSeconds(60) {}

SupabaseAuth::SupabaseAuth(SupabaseHttp* http)
    : _http(http),
      _storage(&_defaultMemoryStorage),
      _autoRefresh(true),
      _refreshMarginSeconds(60) {}

SupabaseAuth::~SupabaseAuth() {}

void SupabaseAuth::setHttp(SupabaseHttp* http) {
    _http = http;
    if (_http != nullptr && _session.isValid()) {
        _http->setAccessToken(_session.accessToken);
    }
}

void SupabaseAuth::setStorage(ISupabaseSessionStorage* storage) {
    if (storage != nullptr) {
        _storage = storage;
        loadSession();
    } else {
        _storage = &_defaultMemoryStorage;
    }
}

void SupabaseAuth::setAutoRefresh(bool autoRefresh) {
    _autoRefresh = autoRefresh;
}

bool SupabaseAuth::isAutoRefresh() const {
    return _autoRefresh;
}

void SupabaseAuth::setRefreshMargin(uint32_t seconds) {
    _refreshMarginSeconds = seconds;
}

uint32_t SupabaseAuth::refreshMargin() const {
    return _refreshMarginSeconds;
}

const SupabaseSession& SupabaseAuth::session() const {
    return _session;
}

bool SupabaseAuth::isAuthenticated() const {
    return _session.isValid();
}

void SupabaseAuth::saveSession() {
    if (_storage != nullptr) {
        _storage->save(_session);
    }
}

void SupabaseAuth::loadSession() {
    if (_storage != nullptr) {
        if (_storage->load(_session)) {
            if (_http != nullptr) {
                _http->setAccessToken(_session.accessToken);
            }
        }
    }
}

void SupabaseAuth::clearSession() {
    _session.clear();
    if (_storage != nullptr) {
        _storage->clear();
    }
    if (_http != nullptr) {
        _http->setAccessToken("");
    }
}

void SupabaseAuth::updateSessionFromResponse(const JsonObjectConst& json) {
    if (json.containsKey("access_token") && !json["access_token"].isNull()) {
        _session.accessToken = json["access_token"].as<String>();
    }
    if (json.containsKey("refresh_token") && !json["refresh_token"].isNull()) {
        _session.refreshToken = json["refresh_token"].as<String>();
    }
    if (json.containsKey("token_type") && !json["token_type"].isNull()) {
        _session.tokenType = json["token_type"].as<String>();
    }
    if (json.containsKey("expires_in") && !json["expires_in"].isNull()) {
        _session.expiresIn = json["expires_in"].as<uint32_t>();
        uint64_t nowSec = (uint64_t)time(NULL);
        if (nowSec > 1600000000ULL) {
            _session.expiresAt = nowSec + _session.expiresIn;
        } else {
            _session.expiresAt = 0;
        }
    }
    if (json.containsKey("user") && json["user"].is<JsonObjectConst>()) {
        JsonObjectConst u = json["user"].as<JsonObjectConst>();
        if (u.containsKey("id") && !u["id"].isNull()) {
            _session.userId = u["id"].as<String>();
        }
    }

    if (_http != nullptr) {
        _http->setAccessToken(_session.accessToken);
    }
    saveSession();
}

SupabaseResponse SupabaseAuth::signUp(const String& email, const String& password) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    JsonDocument doc;
    doc["email"] = email;
    doc["password"] = password;

    String bodyStr;
    serializeJson(doc, bodyStr);

    SupabaseResponse resp = _http->execute("POST", "/auth/v1/signup", {}, bodyStr);
    if (resp.ok()) {
        JsonDocument resDoc;
        if (deserializeJson(resDoc, resp.body()) == DeserializationError::Ok) {
            if (resDoc.is<JsonObject>()) {
                updateSessionFromResponse(resDoc.as<JsonObject>());
            }
        }
    }
    return resp;
}

SupabaseResponse SupabaseAuth::signIn(const String& email, const String& password) {
    return signInWithPassword(email, password);
}

SupabaseResponse SupabaseAuth::signInWithPassword(const String& email, const String& password) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    JsonDocument doc;
    doc["email"] = email;
    doc["password"] = password;

    String bodyStr;
    serializeJson(doc, bodyStr);

    SupabaseResponse resp = _http->execute("POST", "/auth/v1/token?grant_type=password", {}, bodyStr);
    if (resp.ok()) {
        JsonDocument resDoc;
        if (deserializeJson(resDoc, resp.body()) == DeserializationError::Ok) {
            if (resDoc.is<JsonObject>()) {
                updateSessionFromResponse(resDoc.as<JsonObject>());
            }
        }
    } else {
        resp.setLibraryErrorCode(SupabaseErrorCode::AUTH_INVALID_CREDENTIALS);
    }
    return resp;
}

SupabaseResponse SupabaseAuth::refreshSession() {
    return refreshSession(_session.refreshToken);
}

SupabaseResponse SupabaseAuth::refreshSession(const String& refreshToken) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }
    if (refreshToken.length() == 0) {
        return SupabaseResponse(0, "", SupabaseErrorCode::AUTH_SESSION_EXPIRED);
    }

    JsonDocument doc;
    doc["refresh_token"] = refreshToken;

    String bodyStr;
    serializeJson(doc, bodyStr);

    SupabaseResponse resp = _http->execute("POST", "/auth/v1/token?grant_type=refresh_token", {}, bodyStr);
    if (resp.ok()) {
        JsonDocument resDoc;
        if (deserializeJson(resDoc, resp.body()) == DeserializationError::Ok) {
            if (resDoc.is<JsonObject>()) {
                updateSessionFromResponse(resDoc.as<JsonObject>());
            }
        }
    } else {
        clearSession();
        resp.setLibraryErrorCode(SupabaseErrorCode::AUTH_REFRESH_FAILED);
    }
    return resp;
}

SupabaseResponse SupabaseAuth::getUser() {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }
    if (!_session.isValid()) {
        return SupabaseResponse(0, "", SupabaseErrorCode::AUTH_SESSION_EXPIRED);
    }

    checkAutoRefresh();

    return _http->execute("GET", "/auth/v1/user");
}

SupabaseResponse SupabaseAuth::signOut() {
    SupabaseResponse resp;
    if (_http != nullptr && _session.isValid()) {
        resp = _http->execute("POST", "/auth/v1/logout");
    }
    clearSession();
    return resp;
}

bool SupabaseAuth::checkAutoRefresh() {
    if (!_autoRefresh || !_session.isValid()) return false;
    if (_session.isExpired(_refreshMarginSeconds)) {
        SupabaseResponse resp = refreshSession();
        return resp.ok();
    }
    return true;
}
