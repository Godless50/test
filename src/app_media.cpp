/**
 * @file app_media.cpp
 * @brief Медиаплеер с потоковым приемом и авто-скрытием кнопки закрытия
 */

#include "app_media.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

/* --- Глобальные объекты --- */
static lv_obj_t* media_screen = nullptr;
static lv_obj_t* image_widget = nullptr;
static lv_obj_t* close_btn = nullptr;
static lv_img_dsc_t current_img;

/* Буфер для изображения (JPEG) - используем PSRAM */
#define IMG_BUFFER_SIZE (320 * 240 * 2)  /* Максимальный размер JPEG */
static uint8_t* img_buffer = nullptr;
static size_t img_size = 0;

/* Таймер авто-скрытия */
static lv_timer_t* hide_timer = nullptr;
static bool close_btn_visible = true;

/* Веб-сервер для приема потока */
static AsyncWebServer* server = nullptr;
static AsyncWebSocket* ws = nullptr;

#define MEDIA_PORT 81

/* ============================================================
   Логика авто-скрытия крестика
   ============================================================ */

static void show_close_button(void)
{
    if (!close_btn_visible && close_btn) {
        lv_obj_clear_flag(close_btn, LV_OBJ_FLAG_HIDDEN);
        close_btn_visible = true;
    }
    
    /* Перезапускаем таймер */
    if (hide_timer) {
        lv_timer_reset(hide_timer);
    }
}

static void hide_close_button_cb(lv_timer_t* timer)
{
    if (close_btn_visible && close_btn) {
        lv_obj_add_flag(close_btn, LV_OBJ_FLAG_HIDDEN);
        close_btn_visible = false;
    }
}

static void screen_touch_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
        show_close_button();
    }
}

/* ============================================================
   Обработчики веб-сокета
   ============================================================ */

static void ws_event_cb(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                        AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        
        if (info->opcode == WS_BINARY && info->final) {
            /* Получаем JPEG кадр */
            if (info->len > IMG_BUFFER_SIZE) {
                Serial.printf("[Media] Кадр слишком большой: %d\n", info->len);
                return;
            }
            
            /* Копируем данные в буфер */
            memcpy(img_buffer, data, len);
            img_size = len;
            
            /* Обновляем изображение */
            current_img.data = img_buffer;
            current_img.data_size = img_size;
            current_img.header.w = 320;  /* Предполагаемый размер */
            current_img.header.h = 240;
            current_img.header.always_zero = 0;
            current_img.header.cf = LV_IMG_CF_RAW;
            
            if (image_widget) {
                lv_img_set_src(image_widget, &current_img);
            }
            
            /* Показываем крестик при получении данных */
            show_close_button();
        }
    }
}

/* ============================================================
   Обработчики событий
   ============================================================ */

static void close_event_cb(lv_event_t* e)
{
    media_stop_stream();
    lv_obj_del(media_screen);
    media_screen = nullptr;
    Serial.println("[Media] Закрыто");
}

/* ============================================================
   Публичные функции
   ============================================================ */

void media_app_init(void)
{
    /* Выделяем буфер в PSRAM */
    if (!img_buffer) {
        img_buffer = (uint8_t*)heap_caps_malloc(IMG_BUFFER_SIZE, MALLOC_CAP_SPIRAM);
        if (!img_buffer) {
            Serial.println("[Media] Ошибка выделения памяти!");
            return;
        }
    }
    
    /* Создаем экран */
    media_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(media_screen, lv_color_hex(0x000000), 0);
    
    /* Обработчик касания экрана */
    lv_obj_add_event_cb(media_screen, screen_touch_event_cb, LV_EVENT_ALL, NULL);
    
    /* Виджет изображения */
    image_widget = lv_img_create(media_screen);
    lv_obj_center(image_widget);
    
    /* Заглушка пока нет изображения */
    static lv_color_t placeholder_buf[100 * 100];
    static lv_img_dsc_t placeholder;
    placeholder.header.w = 100;
    placeholder.header.h = 100;
    placeholder.header.always_zero = 0;
    placeholder.header.cf = LV_IMG_CF_TRUE_COLOR;
    placeholder.data_size = sizeof(placeholder_buf);
    placeholder.data = (const uint8_t*)placeholder_buf;
    
    /* Заполняем заглушку серым */
    for (int i = 0; i < 100 * 100; i++) {
        placeholder_buf[i] = lv_color_hex(0x333333);
    }
    
    lv_img_set_src(image_widget, &placeholder);
    
    /* Кнопка закрытия */
    close_btn = lv_btn_create(media_screen);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xE74C3C), 0);
    lv_obj_set_style_opa(close_btn, LV_OPA_90, 0);
    
    lv_obj_t* close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "X");
    lv_obj_center(close_label);
    
    lv_obj_add_event_cb(close_btn, close_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Таймер авто-скрытия (5 секунд) */
    hide_timer = lv_timer_create(hide_close_button_cb, 5000, NULL);
    
    /* Загружаем экран */
    lv_scr_load(media_screen);
    
    /* Стартуем поток */
    media_start_stream();
    
    Serial.println("[Media] Приложение запущено");
}

void media_start_stream(void)
{
    if (server) {
        Serial.println("[Media] Сервер уже запущен");
        return;
    }
    
    /* Создаем веб-сервер */
    server = new AsyncWebServer(MEDIA_PORT);
    ws = new AsyncWebSocket("/stream");
    
    ws->onEvent(ws_event_cb);
    server->addHandler(ws);
    
    /* Страница инструкции для смартфона */
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                      "<title>ESP32 Media Stream</title>"
                      "<style>body{font-family:Arial;padding:20px;}"
                      ".btn{background:#4CAF50;color:white;padding:15px 30px;"
                      "text-decoration:none;border-radius:5px;display:inline-block;margin:10px;}</style>"
                      "</head><body><h1>Медиапоток ESP32</h1>"
                      "<p>Подключитесь к этому серверу со смартфона:</p>"
                      "<p><strong>ws://" + WiFi.localIP().toString() + ":" + String(MEDIA_PORT) + "/stream</strong></p>"
                      "<p>Отправляйте JPEG кадры через WebSocket (binary).</p>"
                      "<h3>Пример для Android (Kotlin):</h3>"
                      "<pre>val ws = WebSocket(uri(\"ws://IP:81/stream\"))\n"
                      "ws.send(Bitmap.compress(Bitmap.CompressFormat.JPEG, 80))</pre>"
                      "</body></html>";
        request->send(200, "text/html", html);
    });
    
    server->begin();
    Serial.printf("[Media] Сервер запущен на порту %d\n", MEDIA_PORT);
    Serial.printf("[Media] Откройте http://%s:%d/ для инструкций\n", 
                  WiFi.localIP().toString().c_str(), MEDIA_PORT);
}

void media_stop_stream(void)
{
    if (ws) {
        ws->closeAll();
        delete ws;
        ws = nullptr;
    }
    
    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }
    
    Serial.println("[Media] Сервер остановлен");
}

void media_on_frame_received(uint8_t* data, size_t len)
{
    /* Эта функция может вызываться из других источников (например, Bluetooth) */
    if (len > IMG_BUFFER_SIZE) return;
    
    memcpy(img_buffer, data, len);
    img_size = len;
    
    current_img.data = img_buffer;
    current_img.data_size = img_size;
    current_img.header.w = 320;
    current_img.header.h = 240;
    current_img.header.always_zero = 0;
    current_img.header.cf = LV_IMG_CF_RAW;
    
    if (image_widget) {
        lv_img_set_src(image_widget, &current_img);
    }
    
    show_close_button();
}

void media_app_deinit(void)
{
    media_stop_stream();
    
    if (hide_timer) {
        lv_timer_del(hide_timer);
        hide_timer = nullptr;
    }
    
    if (img_buffer) {
        heap_caps_free(img_buffer);
        img_buffer = nullptr;
    }
    
    if (media_screen) {
        lv_obj_del(media_screen);
        media_screen = nullptr;
    }
    
    image_widget = nullptr;
    close_btn = nullptr;
    close_btn_visible = true;
}
