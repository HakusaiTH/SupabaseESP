#include <WiFi.h>
#include <SupabaseESP.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_KEY = "sb_publishable_xxxxxxxxx";

SupabaseESP supabase;

void setup() {
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }

    supabase.begin(SUPABASE_URL, SUPABASE_KEY);

    // Optional: enable session persistence in ESP32 Preferences/NVS
    supabase.auth().setStorage(new PreferencesSessionStorage("supabase"));

    auto resp = supabase.auth().signIn("user@example.com", "password123");

    if (resp.ok()) {
        Serial.println("Authentication successful!");
        Serial.print("User ID: ");
        Serial.println(supabase.auth().session().userId);

        // Subsequent database queries will use Bearer <access_token> automatically
        auto dbResp = supabase.from("user_profile").select("*").single().execute();
        Serial.println(dbResp.body());

    } else {
        Serial.printf("Auth Error [%d]: %s\n", resp.statusCode(), resp.errorMessage().c_str());
    }
}

void loop() {
}
