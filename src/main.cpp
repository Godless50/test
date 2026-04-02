/**
 * @file main.cpp
 * @brief Точка входа приложения ESP32 Desktop
 * 
 * Инициализирует:
 * - Дисплей и тачскрин
 * - LVGL
 * - Wi-Fi и NTP
 * - Bluetooth
 * - Рабочий стол и приложения
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <BluetoothSerial.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <time.h>

#include "system_time.h"
#include "app_desktop.h"
#include "app_clock.h"
#include "app_calculator.h"
#include "app_draw.h"
#include "app_media.h"

/* --- Конфигурация Wi-Fi --- */
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

/* --- NTP сервер --- */
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 10800  // UTC+3 (Москва)
#define DAY_LIGHT_OFFSET_SEC 0

/* --- Буферы LVGL --- */
#define LVGL_BUF_SIZE (480 * 320 / 10)  // 1/10 экрана

/* --- Глобальные объекты --- */
static TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* buf1 = nullptr;
static lv_color_t* buf2 = nullptr;

static bool wifi_connected = false;

#if defined(CONFIG_BT_ENABLED)
BluetoothSerial SerialBT;
#endif

/* --- Прототипы --- */
void setup_wifi();
void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map);
void lvgl_touch_cb(lv_indev_drv_t* drv, lv_indev_data_t* data);
void lvgl_tick_task(void* arg);

/* ============================================================
   LVGL Callbacks
   ============================================================ */

/**
 * @brief Callback отрисовки LVGL на дисплей
 */
void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    // TFT_eSPI требует координаты в портретной ориентации,
    // но у нас уже настроена ROTATION_1, поэтому передаем напрямую
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    
    // Цвета в LVGL уже в формате RGB565 с swap из-за LV_COLOR_16_SWAP
    tft.pushColors((uint16_t*)&color_map->full, w * h, false);
    tft.endWrite();
    
    lv_disp_flush_ready(drv);
}

/**
 * @brief Callback чтения тачскрина
 */
static bool touch_pressed = false;
static int16_t touch_x = 0, touch_y = 0;

void lvgl_touch_cb(lv_indev_drv_t* drv, lv_indev_data_t* data)
{
    uint16_t x, y;
    
    if (tft.getTouch(&x, &y)) {
        touch_pressed = true;
        touch_x = x;
        touch_y = y;
    } else {
        touch_pressed = false;
    }
    
    data->state = touch_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    data->point.x = touch_x;
    data->point.y = touch_y;
}

/**
 * @brief Тикер LVGL (вызывается каждую 5 мс)
 */
void lvgl_tick_task(void* arg)
{
    lv_tick_inc(5);
}

/* ============================================================
   Wi-Fi и сеть
   ============================================================ */

void setup_wifi()
{
    Serial.println("[WiFi] Подключение...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        Serial.println("\n[WiFi] Подключено!");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
        
        // Синхронизация времени через NTP
        time_sync_ntp(NTP_SERVER);
    } else {
        Serial.println("\n[WiFi] Не удалось подключиться");
    }
}

/* ============================================================
   Setup & Loop
   ============================================================ */

void setup()
{
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Desktop ===");
    
    /* Инициализация дисплея */
    tft.init();
    tft.setRotation(1);  // Альбомная ориентация
    tft.fillScreen(TFT_BLACK);
    Serial.println("[Display] Инициализирован");
    
    /* Инициализация LVGL */
    lv_init();
    
    // Выделение буферов в PSRAM
    buf1 = (lv_color_t*)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t*)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    
    if (!buf1 || !buf2) {
        Serial.println("[LVGL] Ошибка выделения памяти для буферов!");
        while (1) delay(1000);
    }
    
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_BUF_SIZE);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);
    
    Serial.println("[LVGL] Инициализирован");
    
    /* Тикер LVGL */
    esp_timer_handle_t tick_timer;
    esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_task,
        .name = "lvgl_tick"
    };
    esp_timer_create(&timer_args, &tick_timer);
    esp_timer_start_periodic(tick_timer, 5000);  // 5 мс
    
    /* Инициализация времени */
    time_init();
    
    /* Wi-Fi */
    setup_wifi();
    
    /* Bluetooth */
#if defined(CONFIG_BT_ENABLED)
    SerialBT.begin("ESP32-Desktop");
    Serial.println("[Bluetooth] Запущен");
#endif
    
    /* Инициализация рабочего стола */
    desktop_init();
    
    Serial.println("=== Готово ===");
}

void loop()
{
    /* Обработка LVGL */
    lv_timer_handler();
    
    /* Обработка Bluetooth данных */
#if defined(CONFIG_BT_ENABLED)
    if (SerialBT.available()) {
        uint8_t data[64];
        size_t len = SerialBT.readBytes(data, sizeof(data));
        // Здесь можно парсить команды от смартфона, включая время
        time_handle_bt_data(data, len);
    }
#endif
    
    delay(5);  // Небольшая задержка для стабильности
}
