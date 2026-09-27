#include <SupabaseESP.h>

SupabaseESP supabase;

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* SUPABASE_URL = "https://YOUR_PROJECT.supabase.co";
const char* SUPABASE_KEY = "YOUR_ANON_KEY";

const char* USER_EMAIL = "user@example.com";
const char* USER_PASSWORD = "password123";

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!supabase.begin(WIFI_SSID, WIFI_PASSWORD, SUPABASE_URL, SUPABASE_KEY)) {
        Serial.print("Supabase init failed: ");
        Serial.println(supabase.lastErrorMessage());
        return;
    }

    Serial.println("--- 1. SIGN IN ---");
    if (supabase.signIn(USER_EMAIL, USER_PASSWORD)) {
        Serial.println("Sign in successful!");
        Serial.print("Access Token: ");
        Serial.println(supabase.auth().session().accessToken);
    } else {
        Serial.print("Sign in failed: ");
        Serial.println(supabase.lastErrorMessage());
    }

    Serial.println("--- 2. SIGN OUT ---");
    if (supabase.signOut()) {
        Serial.println("Sign out successful!");
    } else {
        Serial.print("Sign out failed: ");
        Serial.println(supabase.lastErrorMessage());
    }
}

void loop() {
}
