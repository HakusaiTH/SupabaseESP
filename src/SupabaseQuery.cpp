#include "SupabaseQuery.h"

SupabaseQuery::SupabaseQuery(SupabaseHttp* http, const String& table, const String& schema)
    : _http(http),
      _table(table),
      _schema(schema),
      _isRpc(false),
      _method(QueryMethod::GET_SELECT),
      _hasFilter(false),
      _requireMutationFilter(false) {}

SupabaseQuery::SupabaseQuery(SupabaseHttp* http, bool isRpc, const String& rpcFunction)
    : _http(http),
      _table(""),
      _schema("public"),
      _isRpc(isRpc),
      _rpcFunction(rpcFunction),
      _method(QueryMethod::POST_RPC),
      _hasFilter(false),
      _requireMutationFilter(false) {}

SupabaseQuery& SupabaseQuery::schema(const String& schemaName) {
    _schema = schemaName;
    return *this;
}

SupabaseQuery& SupabaseQuery::select(const String& columns) {
    if (_method == QueryMethod::POST_INSERT || _method == QueryMethod::PATCH_UPDATE || _method == QueryMethod::POST_UPSERT) {
        // Representation returned after mutation
        header("Prefer", "return=representation");
    } else if (!_isRpc) {
        _method = QueryMethod::GET_SELECT;
    }

    rawParam("select", columns);
    return *this;
}

SupabaseQuery& SupabaseQuery::insert(JsonVariantConst doc) {
    _method = QueryMethod::POST_INSERT;
    serializeJson(doc, _bodyPayload);
    return *this;
}

SupabaseQuery& SupabaseQuery::update(JsonVariantConst doc) {
    _method = QueryMethod::PATCH_UPDATE;
    serializeJson(doc, _bodyPayload);
    return *this;
}

SupabaseQuery& SupabaseQuery::upsert(JsonVariantConst doc, const String& onConflict) {
    _method = QueryMethod::POST_UPSERT;
    serializeJson(doc, _bodyPayload);
    header("Prefer", "resolution=merge-duplicates");
    if (onConflict.length() > 0) {
        rawParam("on_conflict", onConflict);
    }
    return *this;
}

SupabaseQuery& SupabaseQuery::remove() {
    _method = QueryMethod::DELETE_REMOVE;
    return *this;
}

void SupabaseQuery::addFilter(const String& column, const String& op, const String& value) {
    _hasFilter = true;
    _queryParams.push_back({column, op + "." + value});
}

SupabaseQuery& SupabaseQuery::eq(const String& column, const String& value) {
    addFilter(column, "eq", value);
    return *this;
}

SupabaseQuery& SupabaseQuery::eq(const String& column, int value) {
    addFilter(column, "eq", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::eq(const String& column, double value) {
    addFilter(column, "eq", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::neq(const String& column, const String& value) {
    addFilter(column, "neq", value);
    return *this;
}

SupabaseQuery& SupabaseQuery::neq(const String& column, int value) {
    addFilter(column, "neq", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::gt(const String& column, const String& value) {
    addFilter(column, "gt", value);
    return *this;
}

SupabaseQuery& SupabaseQuery::gt(const String& column, double value) {
    addFilter(column, "gt", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::gte(const String& column, const String& value) {
    addFilter(column, "gte", value);
    return *this;
}

SupabaseQuery& SupabaseQuery::gte(const String& column, double value) {
    addFilter(column, "gte", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::lt(const String& column, const String& value) {
    addFilter(column, "lt", value);
    return *this;
}

SupabaseQuery& SupabaseQuery::lt(const String& column, double value) {
    addFilter(column, "lt", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::lte(const String& column, const String& value) {
    addFilter(column, "lte", value);
    return *this;
}

SupabaseQuery& SupabaseQuery::lte(const String& column, double value) {
    addFilter(column, "lte", String(value));
    return *this;
}

SupabaseQuery& SupabaseQuery::like(const String& column, const String& pattern) {
    addFilter(column, "like", pattern);
    return *this;
}

SupabaseQuery& SupabaseQuery::ilike(const String& column, const String& pattern) {
    addFilter(column, "ilike", pattern);
    return *this;
}

SupabaseQuery& SupabaseQuery::is(const String& column, const char* val) {
    if (val == nullptr) {
        addFilter(column, "is", "null");
    } else {
        addFilter(column, "is", String(val));
    }
    return *this;
}

SupabaseQuery& SupabaseQuery::in(const String& column, const String& commaSeparatedValues) {
    addFilter(column, "in", "(" + commaSeparatedValues + ")");
    return *this;
}

SupabaseQuery& SupabaseQuery::rawParam(const String& key, const String& value) {
    _queryParams.push_back({key, value});
    return *this;
}

SupabaseQuery& SupabaseQuery::order(const String& column, bool ascending) {
    String orderVal = column + (ascending ? ".asc" : ".desc");
    rawParam("order", orderVal);
    return *this;
}

SupabaseQuery& SupabaseQuery::limit(size_t count) {
    rawParam("limit", String(count));
    return *this;
}

SupabaseQuery& SupabaseQuery::range(size_t from, size_t to) {
    header("Range", String(from) + "-" + String(to));
    return *this;
}

SupabaseQuery& SupabaseQuery::single() {
    header("Accept", "application/vnd.pgrst.object+json");
    return *this;
}

SupabaseQuery& SupabaseQuery::maybeSingle() {
    header("Accept", "application/vnd.pgrst.object+json");
    return *this;
}

SupabaseQuery& SupabaseQuery::header(const String& name, const String& value) {
    for (auto& h : _requestHeaders) {
        if (h.name.equalsIgnoreCase(name)) {
            h.value = value;
            return *this;
        }
    }
    _requestHeaders.push_back({name, value});
    return *this;
}

SupabaseQuery& SupabaseQuery::setRequireMutationFilter(bool require) {
    _requireMutationFilter = require;
    return *this;
}

SupabaseResponse SupabaseQuery::execute() {
    if (_http == nullptr) {
        return SupabaseResponse(0, "", SupabaseErrorCode::INVALID_CONFIG);
    }

    if (_requireMutationFilter && !_hasFilter &&
        (_method == QueryMethod::PATCH_UPDATE || _method == QueryMethod::DELETE_REMOVE)) {
        return SupabaseResponse(0, "", SupabaseErrorCode::MUTATION_FILTER_REQUIRED);
    }

    String path = "/rest/v1/";
    if (_isRpc) {
        path += "rpc/" + _rpcFunction;
    } else {
        path += _table;
    }

    // Build query string
    if (!_queryParams.empty()) {
        path += "?";
        for (size_t i = 0; i < _queryParams.size(); i++) {
            if (i > 0) path += "&";
            path += SupabaseUrlEncoder::encodeComponent(_queryParams[i].key);
            path += "=";
            path += SupabaseUrlEncoder::encodeComponent(_queryParams[i].value);
        }
    }

    // Schema headers
    if (_schema.length() > 0 && _schema != "public") {
        header("Accept-Profile", _schema);
        header("Content-Profile", _schema);
    }

    String httpMethod = "GET";
    switch (_method) {
        case QueryMethod::GET_SELECT:   httpMethod = "GET"; break;
        case QueryMethod::POST_INSERT:  httpMethod = "POST"; break;
        case QueryMethod::PATCH_UPDATE: httpMethod = "PATCH"; break;
        case QueryMethod::POST_UPSERT:  httpMethod = "POST"; break;
        case QueryMethod::DELETE_REMOVE: httpMethod = "DELETE"; break;
        case QueryMethod::POST_RPC:     httpMethod = "POST"; break;
    }

    return _http->execute(httpMethod, path, _requestHeaders, _bodyPayload);
}
