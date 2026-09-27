#ifndef SUPABASE_DATABASE_H
#define SUPABASE_DATABASE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SupabaseHttp.h"
#include "SupabaseQuery.h"

/**
 * @brief Submodule for Supabase REST Database access (PostgREST).
 */
class SupabaseDatabase {
public:
    SupabaseDatabase();
    explicit SupabaseDatabase(SupabaseHttp* http);

    void setHttp(SupabaseHttp* http);

    SupabaseQuery from(const String& table);
    SupabaseQuery from(const String& schema, const String& table);

    SupabaseQuery rpc(const String& functionName, JsonVariantConst params = JsonVariantConst());

    void setRequireMutationFilter(bool require);

private:
    SupabaseHttp* _http;
    bool _requireMutationFilter;
};

#endif // SUPABASE_DATABASE_H
