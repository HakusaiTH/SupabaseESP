#include "SupabaseDatabase.h"

SupabaseDatabase::SupabaseDatabase()
    : _http(nullptr), _requireMutationFilter(false) {}

SupabaseDatabase::SupabaseDatabase(SupabaseHttp* http)
    : _http(http), _requireMutationFilter(false) {}

void SupabaseDatabase::setHttp(SupabaseHttp* http) {
    _http = http;
}

SupabaseQuery SupabaseDatabase::from(const String& table) {
    SupabaseQuery q(_http, table, "public");
    q.setRequireMutationFilter(_requireMutationFilter);
    return q;
}

SupabaseQuery SupabaseDatabase::from(const String& schema, const String& table) {
    SupabaseQuery q(_http, table, schema);
    q.setRequireMutationFilter(_requireMutationFilter);
    return q;
}

SupabaseQuery SupabaseDatabase::rpc(const String& functionName, JsonVariantConst params) {
    SupabaseQuery q(_http, true, functionName);
    if (!params.isNull()) {
        q.insert(params);
    }
    return q;
}

void SupabaseDatabase::setRequireMutationFilter(bool require) {
    _requireMutationFilter = require;
}
