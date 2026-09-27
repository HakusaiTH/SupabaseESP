#pragma once
#ifndef SUPABASE_RESPONSE_H
#define SUPABASE_RESPONSE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SupabaseError.h"

/**
 * @brief Encapsulates HTTP response and error metadata from Supabase requests.
 */
class SupabaseResponse {
public:
    SupabaseResponse();
    SupabaseResponse(int statusCode, const String& body, SupabaseErrorCode libCode = SupabaseErrorCode::NONE, uint32_t elapsedMs = 0);

    bool ok() const;
    int statusCode() const;
    const String& body() const;
    const String& errorMessage() const;
    const String& errorCode() const;
    const String& errorDetails() const;
    const String& errorHint() const;
    SupabaseErrorCode libraryErrorCode() const;
    size_t contentLength() const;
    uint32_t elapsedMs() const;
    int retryAfter() const;

    void setStatusCode(int code);
    void setBody(const String& body);
    void setLibraryErrorCode(SupabaseErrorCode code);
    void setElapsedMs(uint32_t ms);
    void setRetryAfter(int seconds);
    void setErrorMessage(const String& msg);
    void setErrorCode(const String& code);

    void parseErrorJson(const String& jsonStr);

private:
    int _statusCode;
    String _body;
    String _errorMessage;
    String _errorCode;
    String _errorDetails;
    String _errorHint;
    SupabaseErrorCode _libraryErrorCode;
    size_t _contentLength;
    uint32_t _elapsedMs;
    int _retryAfter;
};

#endif // SUPABASE_RESPONSE_H
