#include "SupabaseStorage.h"

SupabaseStorageBucket::SupabaseStorageBucket(SupabaseHttp* http, const String& bucketName)
    : _http(http), _bucket(bucketName) {}

SupabaseResponse SupabaseStorageBucket::upload(
    const String& path,
    const uint8_t* data,
    size_t length,
    const String& contentType,
    bool upsert
) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);

    std::vector<SupabaseHeader> headers;
    headers.push_back({"Content-Type", contentType});
    headers.push_back({"x-upsert", upsert ? "true" : "false"});

    String payload = "";
    if (data != nullptr && length > 0) {
        payload = String((const char*)data, length);
    }

    return _http->execute("POST", endpoint, headers, payload);
}

SupabaseResponse SupabaseStorageBucket::upload(
    const String& path,
    Stream& inputStream,
    size_t inputStreamLength,
    const String& contentType,
    bool upsert
) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);

    std::vector<SupabaseHeader> headers;
    headers.push_back({"Content-Type", contentType});
    headers.push_back({"x-upsert", upsert ? "true" : "false"});

    return _http->executeStream("POST", endpoint, headers, &inputStream, inputStreamLength, nullptr);
}

SupabaseResponse SupabaseStorageBucket::upload(
    const String& path,
    const String& textContent,
    const String& contentType,
    bool upsert
) {
    return upload(path, (const uint8_t*)textContent.c_str(), textContent.length(), contentType, upsert);
}

SupabaseResponse SupabaseStorageBucket::download(const String& path, Stream& outputStream) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);

    return _http->executeStream("GET", endpoint, {}, nullptr, 0, &outputStream);
}

String SupabaseStorageBucket::downloadText(const String& path) {
    if (_http == nullptr) return "";

    String endpoint = "/storage/v1/object/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);

    SupabaseResponse resp = _http->execute("GET", endpoint);
    if (resp.ok()) {
        return resp.body();
    }
    return "";
}

SupabaseResponse SupabaseStorageBucket::list(
    const String& path,
    size_t limit,
    size_t offset,
    const String& search,
    const String& sortBy,
    const String& sortOrder
) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/list/" + _bucket;

    JsonDocument doc;
    doc["prefix"] = path;
    doc["limit"] = limit;
    doc["offset"] = offset;
    if (search.length() > 0) {
        doc["search"] = search;
    }
    JsonObject sortObj = doc["sortBy"].to<JsonObject>();
    sortObj["column"] = sortBy;
    sortObj["order"] = sortOrder;

    String bodyStr;
    serializeJson(doc, bodyStr);

    return _http->execute("POST", endpoint, {}, bodyStr);
}

SupabaseResponse SupabaseStorageBucket::remove(const String& path) {
    std::vector<String> paths = {path};
    return remove(paths);
}

SupabaseResponse SupabaseStorageBucket::remove(const std::vector<String>& paths) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/" + _bucket;

    JsonDocument doc;
    JsonArray prefixes = doc["prefixes"].to<JsonArray>();
    for (const auto& p : paths) {
        prefixes.add(p);
    }

    String bodyStr;
    serializeJson(doc, bodyStr);

    return _http->execute("DELETE", endpoint, {}, bodyStr);
}

String SupabaseStorageBucket::publicUrl(const String& path) {
    if (_http == nullptr) return "";

    String url = _http->baseUrl();
    url += "/storage/v1/object/public/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);
    return url;
}

String SupabaseStorageBucket::createSignedUrl(const String& path, uint32_t expiresInSeconds) {
    if (_http == nullptr) return "";

    String endpoint = "/storage/v1/object/sign/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);

    JsonDocument doc;
    doc["expiresIn"] = expiresInSeconds;

    String bodyStr;
    serializeJson(doc, bodyStr);

    SupabaseResponse resp = _http->execute("POST", endpoint, {}, bodyStr);
    if (resp.ok()) {
        JsonDocument resDoc;
        if (deserializeJson(resDoc, resp.body()) == DeserializationError::Ok) {
            if (resDoc.containsKey("signedURL")) {
                String fullPath = resDoc["signedURL"].as<String>();
                if (fullPath.startsWith("http://") || fullPath.startsWith("https://")) {
                    return fullPath;
                }
                return _http->baseUrl() + fullPath;
            }
        }
    }
    return "";
}

String SupabaseStorageBucket::createSignedUploadUrl(const String& path) {
    if (_http == nullptr) return "";

    String endpoint = "/storage/v1/object/upload/sign/" + _bucket + "/" + SupabaseUrlEncoder::encodePath(path);

    SupabaseResponse resp = _http->execute("POST", endpoint);
    if (resp.ok()) {
        JsonDocument resDoc;
        if (deserializeJson(resDoc, resp.body()) == DeserializationError::Ok) {
            if (resDoc.containsKey("url")) {
                String fullPath = resDoc["url"].as<String>();
                if (fullPath.startsWith("http://") || fullPath.startsWith("https://")) {
                    return fullPath;
                }
                return _http->baseUrl() + fullPath;
            }
        }
    }
    return "";
}

SupabaseResponse SupabaseStorageBucket::move(const String& fromPath, const String& toPath) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/move";

    JsonDocument doc;
    doc["bucketId"] = _bucket;
    doc["sourceKey"] = fromPath;
    doc["destinationKey"] = toPath;

    String bodyStr;
    serializeJson(doc, bodyStr);

    return _http->execute("POST", endpoint, {}, bodyStr);
}

SupabaseResponse SupabaseStorageBucket::copy(const String& fromPath, const String& toPath) {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    String endpoint = "/storage/v1/object/copy";

    JsonDocument doc;
    doc["bucketId"] = _bucket;
    doc["sourceKey"] = fromPath;
    doc["destinationKey"] = toPath;

    String bodyStr;
    serializeJson(doc, bodyStr);

    return _http->execute("POST", endpoint, {}, bodyStr);
}

// Submodule SupabaseStorage
SupabaseStorage::SupabaseStorage()
    : _http(nullptr) {}

SupabaseStorage::SupabaseStorage(SupabaseHttp* http)
    : _http(http) {}

void SupabaseStorage::setHttp(SupabaseHttp* http) {
    _http = http;
}

SupabaseStorageBucket SupabaseStorage::from(const String& bucketName) {
    return SupabaseStorageBucket(_http, bucketName);
}
