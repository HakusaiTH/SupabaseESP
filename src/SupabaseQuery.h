#pragma once
#ifndef SUPABASE_QUERY_H
#define SUPABASE_QUERY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "SupabaseHttp.h"
#include "SupabaseResponse.h"
#include "SupabaseUrlEncoder.h"

enum class QueryMethod {
    GET_SELECT,
    POST_INSERT,
    PATCH_UPDATE,
    POST_UPSERT,
    DELETE_REMOVE,
    POST_RPC
};

struct QueryParam {
    String key;
    String value;
};

/**
 * @brief Fluent PostgREST query builder for database operations.
 */
class SupabaseQuery {
public:
    SupabaseQuery(SupabaseHttp* http, const String& table, const String& schema = "public");
    SupabaseQuery(SupabaseHttp* http, bool isRpc, const String& rpcFunction);

    // Schema selection
    SupabaseQuery& schema(const String& schemaName);

    // Core operations
    SupabaseQuery& select(const String& columns = "*");
    SupabaseQuery& insert(JsonVariantConst doc);
    SupabaseQuery& update(JsonVariantConst doc);
    SupabaseQuery& upsert(JsonVariantConst doc, const String& onConflict = "");
    SupabaseQuery& remove();

    // Filters
    SupabaseQuery& eq(const String& column, const String& value);
    SupabaseQuery& eq(const String& column, int value);
    SupabaseQuery& eq(const String& column, double value);

    SupabaseQuery& neq(const String& column, const String& value);
    SupabaseQuery& neq(const String& column, int value);

    SupabaseQuery& gt(const String& column, const String& value);
    SupabaseQuery& gt(const String& column, double value);

    SupabaseQuery& gte(const String& column, const String& value);
    SupabaseQuery& gte(const String& column, double value);

    SupabaseQuery& lt(const String& column, const String& value);
    SupabaseQuery& lt(const String& column, double value);

    SupabaseQuery& lte(const String& column, const String& value);
    SupabaseQuery& lte(const String& column, double value);

    SupabaseQuery& like(const String& column, const String& pattern);
    SupabaseQuery& ilike(const String& column, const String& pattern);
    SupabaseQuery& is(const String& column, const char* val);

    SupabaseQuery& in(const String& column, const String& commaSeparatedValues);

    SupabaseQuery& rawParam(const String& key, const String& value);

    // Ordering and Pagination
    SupabaseQuery& order(const String& column, bool ascending = true);
    SupabaseQuery& limit(size_t count);
    SupabaseQuery& range(size_t from, size_t to);

    // Modifiers
    SupabaseQuery& single();
    SupabaseQuery& maybeSingle();
    SupabaseQuery& header(const String& name, const String& value);
    SupabaseQuery& setRequireMutationFilter(bool require);

    // Execution
    SupabaseResponse execute();

private:
    SupabaseHttp* _http;
    String _table;
    String _schema;
    bool _isRpc;
    String _rpcFunction;
    
    QueryMethod _method;
    std::vector<QueryParam> _queryParams;
    std::vector<SupabaseHeader> _requestHeaders;
    
    String _bodyPayload;
    bool _hasFilter;
    bool _requireMutationFilter;

    void addFilter(const String& column, const String& op, const String& value);
};

#endif // SUPABASE_QUERY_H
