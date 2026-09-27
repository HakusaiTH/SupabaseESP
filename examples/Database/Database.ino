#include <SupabaseESP.h>

SupabaseESP supabase;

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* SUPABASE_URL = "https://YOUR_PROJECT.supabase.co";
const char* SUPABASE_KEY = "YOUR_ANON_KEY";

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!supabase.begin(WIFI_SSID, WIFI_PASSWORD, SUPABASE_URL, SUPABASE_KEY)) {
        Serial.print("Supabase init failed: ");
        Serial.println(supabase.lastErrorMessage());
        return;
    }

    Serial.println("--- 1. INSERT ---");
    String insertPayload = "{\"name\":\"ESP32-01\",\"status\":\"online\"}";
    if (supabase.insert("devices", insertPayload)) {
        Serial.println("Insert successful!");
    } else {
        Serial.print("Insert failed: ");
        Serial.println(supabase.lastErrorMessage());
    }

    Serial.println("--- 2. SELECT ---");
    String result = supabase.select("devices");
    Serial.print("Select result: ");
    Serial.println(result);

    Serial.println("--- 3. UPDATE ---");
    String updatePayload = "{\"status\":\"active\"}";
    if (supabase.update("devices", updatePayload, "name=eq.ESP32-01")) {
        Serial.println("Update successful!");
    } else {
        Serial.print("Update failed: ");
        Serial.println(supabase.lastErrorMessage());
    }

    Serial.println("--- 4. REMOVE ---");
    if (supabase.remove("devices", "name=eq.ESP32-01")) {
        Serial.println("Remove successful!");
    } else {
        Serial.print("Remove failed: ");
        Serial.println(supabase.lastErrorMessage());
    }
}

void loop() {
}
