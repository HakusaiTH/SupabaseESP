#ifndef SUPABASE_URL_ENCODER_H
#define SUPABASE_URL_ENCODER_H

#include <Arduino.h>

/**
 * @brief Helper utility for percent-encoding URLs and query parameters (RFC 3986).
 */
class SupabaseUrlEncoder {
public:
    static String encodeComponent(const String& input) {
        String encoded = "";
        encoded.reserve(input.length() * 3 / 2);
        
        const char hexChars[] = "0123456789ABCDEF";

        for (size_t i = 0; i < input.length(); i++) {
            char c = input.charAt(i);

            // RFC 3986 unreserved characters
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.' || c == '~') {
                encoded += c;
            } else {
                encoded += '%';
                encoded += hexChars[(c >> 4) & 0x0F];
                encoded += hexChars[c & 0x0F];
            }
        }
        return encoded;
    }

    static String encodePath(const String& path) {
        String encoded = "";
        encoded.reserve(path.length() * 3 / 2);
        
        const char hexChars[] = "0123456789ABCDEF";

        for (size_t i = 0; i < path.length(); i++) {
            char c = path.charAt(i);

            // Keep '/' unencoded for URL paths
            if (c == '/' ||
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.' || c == '~') {
                encoded += c;
            } else {
                encoded += '%';
                encoded += hexChars[(c >> 4) & 0x0F];
                encoded += hexChars[c & 0x0F];
            }
        }
        return encoded;
    }
};

#endif // SUPABASE_URL_ENCODER_H
