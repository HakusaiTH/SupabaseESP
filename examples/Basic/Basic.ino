#include <SupabaseESP.h>

SupabaseESP supabase;

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* SUPABASE_URL = "https://YOUR_PROJECT.supabase.co";
const char* SUPABASE_KEY = "YOUR_ANON_KEY";

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Connecting to WiFi and initializing Supabase...");

    if (supabase.begin(WIFI_SSID, WIFI_PASSWORD, SUPABASE_URL, SUPABASE_KEY)) {
        Serial.println("Successfully connected to WiFi and Supabase initialized!");
    } else {
        Serial.print("Initialization failed: ");
        Serial.println(supabase.lastErrorMessage());
    }
}

void loop() {
    if (supabase.connected()) {
        Serial.println("Supabase connection active");
    } else {
        Serial.println("Supabase disconnected");
    }
    delay(5000);
}
