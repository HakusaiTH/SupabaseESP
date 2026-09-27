# SupabaseESP

A lightweight Supabase client library for ESP32 and Arduino.

---

## ⚡ Overview

**SupabaseESP** provides a simple, clean, and beginner-friendly C++ API for connecting ESP32 microcontrollers to Supabase services including Database REST API, Authentication, Storage, RPC, and Realtime WebSockets.

---

## ✨ Features

- **WiFi Auto-Connect & Initialization:** Single `begin(...)` call handles WiFi connection and Supabase setup.
- **Simple Database REST Operations:** Easy `select()`, `insert()`, `update()`, and `remove()` methods.
- **Fluent Query Builder:** Chain filters (`eq`, `gte`, `order`, `limit`, etc.) for advanced PostgREST queries.
- **Authentication:** Built-in support for `signUp()`, `signIn()`, and `signOut()`.
- **Error Handling:** Explicit error codes, HTTP status tracking, and debugging messages (`lastErrorMessage()`).
- **Connection Diagnostics:** `connected()` and `wifiConnected()` helpers.
- **Memory Efficient:** Native `WiFiClientSecure` HTTPS stream handling designed for ESP32 RAM bounds.

---

## 💻 Supported ESP32 Boards

- **ESP32** (DevKit v1, WROOM, WROVER)
- **ESP32-S2**
- **ESP32-S3**
- **ESP32-C3**
- **ESP32-C6**

---

## 📋 Requirements

- **Arduino Core for ESP32** (v2.0.0 or higher)
- **ArduinoJson** (v6.x or v7.x)
- **WebSockets** (by Markus Sattler - required for Realtime features)

---

## 📦 Installation

### Arduino Library Manager Installation

1. Open Arduino IDE.
2. Navigate to **Tools** → **Manage Libraries...** (or press `Ctrl+Shift+I` / `Cmd+Shift+I`).
3. Search for **SupabaseESP**.
4. Click **Install**.

### Manual GitHub Installation

1. Download the repository as a `.zip` file from [GitHub](https://github.com/HakusaiTH/SupabaseESP).
2. In Arduino IDE, go to **Sketch** → **Include Library** → **Add .ZIP Library...**
3. Select the downloaded `.zip` file.

---

## 🚀 Quick Start

```cpp
#include <SupabaseESP.h>

SupabaseESP supabase;

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

const char* SUPABASE_URL = "https://YOUR_PROJECT.supabase.co";
const char* SUPABASE_KEY = "YOUR_ANON_KEY";

void setup() {
    Serial.begin(115200);

    // Connect to WiFi & initialize Supabase
    if (supabase.begin(WIFI_SSID, WIFI_PASSWORD, SUPABASE_URL, SUPABASE_KEY)) {
        Serial.println("Supabase ready!");
    } else {
        Serial.print("Initialization failed: ");
        Serial.println(supabase.lastErrorMessage());
    }

    // Query 'devices' table
    String result = supabase.select("devices");
    Serial.println(result);
}

void loop() {
}
```

---

## 🗄️ Database Examples

### Insert

```cpp
String json = "{\"name\":\"ESP32-01\",\"status\":\"online\"}";

if (supabase.insert("devices", json)) {
    Serial.println("Data inserted successfully");
} else {
    Serial.print("Insert failed: ");
    Serial.println(supabase.lastErrorMessage());
}
```

### Select

```cpp
// Select all columns
String allDevices = supabase.select("devices");

// Select specific columns
String namesOnly = supabase.select("devices", "name,status");
```

### Update

```cpp
String updateJson = "{\"status\":\"active\"}";

if (supabase.update("devices", updateJson, "name=eq.ESP32-01")) {
    Serial.println("Data updated successfully");
} else {
    Serial.print("Update failed: ");
    Serial.println(supabase.lastErrorMessage());
}
```

### Remove / Delete

```cpp
if (supabase.remove("devices", "name=eq.ESP32-01")) {
    Serial.println("Row deleted successfully");
} else {
    Serial.print("Delete failed: ");
    Serial.println(supabase.lastErrorMessage());
}
```

### Fluent Query Builder (Advanced)

```cpp
auto result = supabase
    .from("devices")
    .select("id,name,status")
    .eq("status", "online")
    .order("created_at", false)
    .limit(5)
    .execute();

if (result.ok()) {
    Serial.println(result.body());
}
```

---

## 🔐 Auth Examples

```cpp
// Sign In
if (supabase.signIn("user@example.com", "userpassword")) {
    Serial.println("User authenticated");
} else {
    Serial.print("Sign in failed: ");
    Serial.println(supabase.lastErrorMessage());
}

// Sign Out
supabase.signOut();
```

---

## 📖 API Reference

### Initialization & Status

- `bool begin(ssid, password, supabaseUrl, apiKey, wifiTimeoutMs = 15000)`: Connects WiFi and initializes Supabase client.
- `bool begin(supabaseUrl, apiKey)`: Initializes Supabase client when WiFi is already connected.
- `bool connected()`: Returns `true` if WiFi is connected and credentials are configured.
- `bool wifiConnected()`: Returns `true` if ESP32 has an active WiFi connection.

### REST Operations

- `String select(table, columns = "*")`: Fetches rows from the given table.
- `bool insert(table, json)`: Inserts a JSON row into table.
- `bool update(table, json, filter)`: Updates rows matching filter (e.g. `"id=eq.1"`).
- `bool remove(table, filter)`: Deletes rows matching filter.

### Authentication

- `bool signUp(email, password)`: Registers a new user with email and password.
- `bool signIn(email, password)`: Authenticates user with email and password.
- `bool signOut()`: Signs out the current user session.

### Error Handling

- `int lastError()`: Returns numeric error code.
- `String lastErrorMessage()`: Returns descriptive error string (e.g., `"WiFi connection timeout"`, `"HTTP 401 Unauthorized"`, `"HTTP 404 Not Found"`).
- `int lastStatusCode()`: Returns HTTP status code (e.g., `200`, `401`, `404`, `500`).

---

## 🛡️ Security & Best Practices

- **Never commit credentials to GitHub:** Never push WiFi passwords, Supabase URLs, or keys to public repositories.
- **Use Anonymous Key (`anon`) only:** The Supabase anonymous key is intended for client devices when Row Level Security (RLS) is configured correctly.
- **NEVER embed Service-Role Keys:** `service_role` keys bypass RLS and grant full administrative access. Storing them on firmware is a high security risk.
- **Configure Row Level Security (RLS):** Always configure RLS policies in your Supabase project console for all tables accessed by ESP32 devices.

---

## ❓ Troubleshooting

| Issue | Cause & Solution |
| :--- | :--- |
| `"WiFi connection timeout"` | Check SSID and WiFi password. Ensure 2.4 GHz WiFi network is used (ESP32 does not support 5 GHz). |
| `"HTTP 401 Unauthorized"` | Invalid API key or missing JWT token. Verify `SUPABASE_KEY`. |
| `"HTTP 404 Not Found"` | Table name or endpoint URL is incorrect. |
| `"HTTP 500 Server Error"` | Database constraint violation or invalid payload structure. |
| Build Errors | Ensure `ArduinoJson` library is installed via Arduino Library Manager. |

---

## 📄 License

This library is licensed under the [MIT License](LICENSE).
