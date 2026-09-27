<p align="center">
  <img src="https://i.ibb.co/0jX77HjM/Chat-GPT-Image-28-2569-00-15-48.png" alt="SupabaseESP Logo" width="350"/>
</p>

# SupabaseESP

> A production-oriented Supabase client library for ESP32 / ESP32-S3 / ESP32-C3 microcontrollers using the Arduino framework.

---

## ⚡ Overview

**SupabaseESP** brings the power of Supabase Database REST, Auth, Storage, RPC, and Realtime WebSockets to embedded C++ on ESP32 microcontrollers. Designed specifically for memory-constrained hardware, `SupabaseESP` uses dynamic stream allocations, non-blocking real-time heartbeats, and predictable error handling.

---

## 💻 Supported Boards & Core
- ESP32 DevKit v1 / ESP32-WROOM-32
- ESP32-S3
- ESP32-C3 / ESP32-C6
- ESP32 Arduino Core 2.x & 3.x

---

## 📦 Dependencies
- **ArduinoJson** (>= 6.x / 7.x)
- **WebSockets** (by Markus Sattler - for Realtime WebSocket support)

---

## 🚀 Quick Start

```cpp
#include <WiFi.h>
#include <SupabaseESP.h>

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASS"

#define SUPABASE_URL "https://your-project.supabase.co"
#define SUPABASE_KEY "sb_publishable_xxxxxxxxx"

SupabaseESP supabase;

void setup() {
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }

    supabase.begin(SUPABASE_URL, SUPABASE_KEY);

    // Insert telemetry data into 'telemetry' table
    JsonDocument row;
    row["device_id"] = "ESP32-001";
    row["temperature"] = 28.5;
    row["humidity"] = 61.2;

    auto result = supabase.from("telemetry").insert(row).execute();

    if (result.ok()) {
        Serial.println("Telemetry uploaded successfully!");
    } else {
        Serial.printf("Error %d: %s\n", result.statusCode(), result.errorMessage().c_str());
    }
}

void loop() {
}
```

---

## 🗄️ Database REST Examples

### Select with Filters & Ordering
```cpp
auto result = supabase
    .from("sensor_data")
    .select("id,temperature,created_at")
    .gte("temperature", 25.0)
    .order("created_at", false) // descending
    .limit(10)
    .execute();

if (result.ok()) {
    Serial.println(result.body());
}
```

### Update Row
```cpp
JsonDocument updateDoc;
updateDoc["status"] = "maintenance";

auto result = supabase
    .from("devices")
    .update(updateDoc)
    .eq("device_id", "ESP32-001")
    .execute();
```

### Upsert Row
```cpp
JsonDocument doc;
doc["device_id"] = "ESP32-001";
doc["status"] = "online";

auto result = supabase
    .from("devices")
    .upsert(doc, "device_id")
    .execute();
```

### Remote Procedure Call (RPC)
```cpp
JsonDocument args;
args["device_id"] = "ESP32-001";

auto result = supabase.rpc("get_device_status", args).execute();
```

---

## 🔐 Auth Examples

```cpp
// Sign in user with email & password
auto resp = supabase.auth().signIn("user@example.com", "secretpassword");

if (resp.ok()) {
    Serial.println("User authenticated!");
    Serial.print("User ID: ");
    Serial.println(supabase.auth().session().userId);
}
```

Persist sessions across reboot using ESP32 NVS (Preferences):
```cpp
supabase.auth().setStorage(new PreferencesSessionStorage("supabase"));
```

---

## 📁 Storage Examples

### Upload File
```cpp
String payload = "{\"log\": \"system initialized\"}";
auto resp = supabase.storage().from("logs").upload("esp32/boot.json", payload, "application/json", true);
```

### Signed URL
```cpp
String signedUrl = supabase.storage().from("private_bucket").createSignedUrl("firmware/v1.0.bin", 3600);
Serial.println("Download link valid for 1 hour: " + signedUrl);
```

---

## 📡 Realtime Examples (Broadcast & Postgres Changes)

```cpp
auto channel = supabase.realtime().channel("room-1");

// Listen for broadcast messages
channel.onBroadcast("robot_command", [](const JsonObjectConst& payload) {
    Serial.print("Command received: ");
    Serial.println(payload["command"].as<String>());
});

// Subscribe to database INSERTs
channel.onPostgresChange(SupabaseChangeEvent::Insert, "public", "telemetry", [](const SupabasePostgresChange& change) {
    Serial.print("New telemetry row inserted: ");
    Serial.println(change.record["temperature"].as<float>());
});

channel.join();

void loop() {
    supabase.realtime().loop(); // MUST be called in loop()
}
```

---

## 🛡️ Row Level Security (RLS) & Security Guidelines

> **IMPORTANT:** ESP32 firmware can be extracted from physical flash memory.

- **Never** embed a `service_role` or `sb_secret_*` key in your firmware.
- Use only `sb_publishable_...` or legacy `anon` public API keys.
- Enforce access controls using PostgreSQL **Row Level Security (RLS)** in Supabase Console.

Example RLS SQL:
```sql
alter table telemetry enable row level security;

create policy "Authenticated users insert telemetry"
on telemetry for insert to authenticated
with check ((select auth.uid()) = user_id);
```

---

## ⚠️ Memory Considerations & TLS

- **Default Max Response Size:** 16 KB (configurable via `supabase.setMaxResponseSize(bytes)`).
- **HTTPS/TLS:** HTTPS and WSS connections are enforced. Development mode `supabase.setInsecureTLS(true)` bypasses certificate verification for test environments.

---

## 📄 License
Released under the [MIT License](LICENSE).
