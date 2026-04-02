/**
 * @file system_time.cpp
 * @brief Реализация модуля системного времени с NTP и NVS
 */

#include "system_time.h"
#include <WiFi.h>
#include <lwip/apps/sntp.h>
#include <Preferences.h>
#include <Arduino.h>

#define NVS_NAMESPACE "time_store"
#define NVS_KEY_TIMESTAMP "last_time"

static Preferences prefs;

void time_init(void)
{
    // Настройка часового пояса
    setenv("TZ", "MSK-3", 1);
    tzset();
    
    prefs.begin(NVS_NAMESPACE, false);
}

bool time_sync_ntp(const char* server)
{
    if (!WiFi.isConnected()) {
        Serial.println("[Time] Wi-Fi не подключен, пропуск NTP");
        return false;
    }
    
    Serial.printf("[Time] Синхронизация с %s...\n", server);
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, server);
    sntp_init();
    
    // Ждем синхронизации до 10 секунд
    int attempts = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
        Serial.println("[Time] Синхронизация успешна!");
        time_save_to_nvs();
        return true;
    } else {
        Serial.println("[Time] Синхронизация не удалась, загружаем из NVS");
        time_t loaded_time;
        if (time_load_from_nvs(&loaded_time)) {
            struct timeval tv;
            tv.tv_sec = loaded_time;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
            Serial.println("[Time] Время восстановлено из NVS");
            return true;
        }
        return false;
    }
}

void time_save_to_nvs(void)
{
    time_t now = time(NULL);
    prefs.putLong(NVS_KEY_TIMESTAMP, now);
    Serial.printf("[Time] Сохранено время: %ld\n", now);
}

bool time_load_from_nvs(time_t* out_time)
{
    if (!prefs.isKey(NVS_KEY_TIMESTAMP)) {
        Serial.println("[Time] Нет сохраненного времени в NVS");
        return false;
    }
    
    *out_time = prefs.getLong(NVS_KEY_TIMESTAMP, 0);
    if (*out_time > 1600000000) {  // Проверка на адекватность (после 2020 года)
        return true;
    }
    
    return false;
}

time_t time_get_current(void)
{
    return time(NULL);
}

void time_format_string(char* buf, size_t len, time_t t)
{
    struct tm* tm_info = localtime(&t);
    strftime(buf, len, "%H:%M:%S\n%d.%m.%Y", tm_info);
}

void time_handle_bt_data(const uint8_t* data, size_t len)
{
    // Простой протокол: первые 4 байта - timestamp (little endian)
    // Формат команды: [0xAA, 0xBB, timestamp[4], ...]
    if (len >= 6 && data[0] == 0xAA && data[1] == 0xBB) {
        time_t bt_time = 0;
        memcpy(&bt_time, &data[2], sizeof(time_t));
        
        if (bt_time > 1600000000) {
            struct timeval tv;
            tv.tv_sec = bt_time;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
            time_save_to_nvs();
            Serial.println("[Time] Время обновлено через Bluetooth");
        }
    }
}
