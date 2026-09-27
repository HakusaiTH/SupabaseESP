#ifndef SUPABASE_STORAGE_H
#define SUPABASE_STORAGE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "SupabaseHttp.h"
#include "SupabaseResponse.h"
#include "SupabaseUrlEncoder.h"

/**
 * @brief Storage Bucket interface for uploading, downloading, listing, and managing files.
 */
class SupabaseStorageBucket {
public:
    SupabaseStorageBucket(SupabaseHttp* http, const String& bucketName);

    // Uploads
    SupabaseResponse upload(
        const String& path,
        const uint8_t* data,
        size_t length,
        const String& contentType = "application/octet-stream",
        bool upsert = false
    );

    SupabaseResponse upload(
        const String& path,
        Stream& inputStream,
        size_t inputStreamLength,
        const String& contentType = "application/octet-stream",
        bool upsert = false
    );

    SupabaseResponse upload(
        const String& path,
        const String& textContent,
        const String& contentType = "text/plain",
        bool upsert = false
    );

    // Downloads
    SupabaseResponse download(const String& path, Stream& outputStream);
    String downloadText(const String& path);

    // File Management
    SupabaseResponse list(
        const String& path = "",
        size_t limit = 100,
        size_t offset = 0,
        const String& search = "",
        const String& sortBy = "name",
        const String& sortOrder = "asc"
    );

    SupabaseResponse remove(const String& path);
    SupabaseResponse remove(const std::vector<String>& paths);

    // URLs
    String publicUrl(const String& path);
    String createSignedUrl(const String& path, uint32_t expiresInSeconds);
    String createSignedUploadUrl(const String& path);

    // Move & Copy
    SupabaseResponse move(const String& fromPath, const String& toPath);
    SupabaseResponse copy(const String& fromPath, const String& toPath);

private:
    SupabaseHttp* _http;
    String _bucket;
};

/**
 * @brief Submodule for Supabase Storage.
 */
class SupabaseStorage {
public:
    SupabaseStorage();
    explicit SupabaseStorage(SupabaseHttp* http);

    void setHttp(SupabaseHttp* http);

    SupabaseStorageBucket from(const String& bucketName);

private:
    SupabaseHttp* _http;
};

#endif // SUPABASE_STORAGE_H
