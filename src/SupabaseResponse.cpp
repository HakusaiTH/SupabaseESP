#include "SupabaseResponse.h"

SupabaseResponse::SupabaseResponse()
    : _statusCode(0),
      _libraryErrorCode(SupabaseErrorCode::NONE),
      _contentLength(0),
      _elapsedMs(0),
      _retryAfter(0) {}

SupabaseResponse::SupabaseResponse(int statusCode, const String& body, SupabaseErrorCode libCode, uint32_t elapsedMs)
    : _statusCode(statusCode),
      _body(body),
      _libraryErrorCode(libCode),
      _contentLength(body.length()),
      _elapsedMs(elapsedMs),
      _retryAfter(0) {
    if (_statusCode >= 200 && _statusCode < 300 && _libraryErrorCode == SupabaseErrorCode::NONE) {
        // Success
    } else {
        if (_libraryErrorCode == SupabaseErrorCode::NONE) {
            if (_statusCode == 429) {
                _libraryErrorCode = SupabaseErrorCode::RATE_LIMITED;
            } else if (_statusCode == 401 || _statusCode == 403) {
                _libraryErrorCode = SupabaseErrorCode::AUTH_ERROR;
            } else {
                _libraryErrorCode = SupabaseErrorCode::HTTP_ERROR;
            }
        }
        parseErrorJson(_body);
    }
}

bool SupabaseResponse::ok() const {
    return (_statusCode >= 200 && _statusCode < 300) && (_libraryErrorCode == SupabaseErrorCode::NONE);
}

int SupabaseResponse::statusCode() const {
    return _statusCode;
}

const String& SupabaseResponse::body() const {
    return _body;
}

const String& SupabaseResponse::errorMessage() const {
    return _errorMessage;
}

const String& SupabaseResponse::errorCode() const {
    return _errorCode;
}

const String& SupabaseResponse::errorDetails() const {
    return _errorDetails;
}

const String& SupabaseResponse::errorHint() const {
    return _errorHint;
}

SupabaseErrorCode SupabaseResponse::libraryErrorCode() const {
    return _libraryErrorCode;
}

size_t SupabaseResponse::contentLength() const {
    return _contentLength;
}

uint32_t SupabaseResponse::elapsedMs() const {
    return _elapsedMs;
}

int SupabaseResponse::retryAfter() const {
    return _retryAfter;
}

void SupabaseResponse::setStatusCode(int code) {
    _statusCode = code;
}

void SupabaseResponse::setBody(const String& body) {
    _body = body;
    _contentLength = body.length();
}

void SupabaseResponse::setLibraryErrorCode(SupabaseErrorCode code) {
    _libraryErrorCode = code;
}

void SupabaseResponse::setElapsedMs(uint32_t ms) {
    _elapsedMs = ms;
}

void SupabaseResponse::setRetryAfter(int seconds) {
    _retryAfter = seconds;
}

void SupabaseResponse::setErrorMessage(const String& msg) {
    _errorMessage = msg;
}

void SupabaseResponse::setErrorCode(const String& code) {
    _errorCode = code;
}

void SupabaseResponse::parseErrorJson(const String& jsonStr) {
    if (jsonStr.length() == 0) return;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonStr);
    if (err) {
        if (_errorMessage.length() == 0) {
            _errorMessage = jsonStr;
        }
        return;
    }

    if (doc.is<JsonObject>()) {
        JsonObject obj = doc.as<JsonObject>();
        
        // PostgREST / Supabase REST standard error fields
        if (obj.containsKey("code") && !obj["code"].isNull()) {
            _errorCode = obj["code"].as<String>();
        } else if (obj.containsKey("error") && !obj["error"].isNull()) {
            _errorCode = obj["error"].as<String>();
        }

        if (obj.containsKey("message") && !obj["message"].isNull()) {
            _errorMessage = obj["message"].as<String>();
        } else if (obj.containsKey("error_description") && !obj["error_description"].isNull()) {
            _errorMessage = obj["error_description"].as<String>();
        } else if (obj.containsKey("msg") && !obj["msg"].isNull()) {
            _errorMessage = obj["msg"].as<String>();
        }

        if (obj.containsKey("details") && !obj["details"].isNull()) {
            _errorDetails = obj["details"].as<String>();
        }
        if (obj.containsKey("hint") && !obj["hint"].isNull()) {
            _errorHint = obj["hint"].as<String>();
        }
    }
}
