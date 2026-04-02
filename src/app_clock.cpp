/**
 * @file app_clock.cpp
 * @brief Приложение часов с настройкой времени
 */

#include "app_clock.h"
#include "system_time.h"
#include <Arduino.h>

static lv_obj_t* clock_screen = nullptr;
static lv_obj_t* time_label = nullptr;
static lv_obj_t* settings_panel = nullptr;

/* Ролики для настройки */
static lv_obj_t* roller_hour = nullptr;
static lv_obj_t* roller_min = nullptr;
static lv_obj_t* roller_sec = nullptr;
static lv_obj_t* roller_year = nullptr;
static lv_obj_t* roller_month = nullptr;
static lv_obj_t* roller_day = nullptr;

/* Кнопка закрытия */
static lv_obj_t* close_btn = nullptr;

/* ============================================================
   Обработчики событий
   ============================================================ */

static void close_event_cb(lv_event_t* e)
{
    lv_obj_del(clock_screen);
    clock_screen = nullptr;
    Serial.println("[Clock] Закрыто");
}

static void save_time_event_cb(lv_event_t* e)
{
    /* Получаем значения из роллеров */
    uint16_t hour, min, sec, year, month, day;
    
    lv_roller_get_selected(roller_hour, &hour, LV_ROLLER_MODE_NORMAL);
    lv_roller_get_selected(roller_min, &min, LV_ROLLER_MODE_NORMAL);
    lv_roller_get_selected(roller_sec, &sec, LV_ROLLER_MODE_NORMAL);
    lv_roller_get_selected(roller_year, &year, LV_ROLLER_MODE_NORMAL);
    lv_roller_get_selected(roller_month, &month, LV_ROLLER_MODE_NORMAL);
    lv_roller_get_selected(roller_day, &day, LV_ROLLER_MODE_NORMAL);
    
    /* Формируем структуру tm */
    struct tm new_time = {0};
    new_time.tm_hour = hour;
    new_time.tm_min = min;
    new_time.tm_sec = sec;
    new_time.tm_year = year - 1900;  // Год с 1900
    new_time.tm_mon = month;          // Месяц 0-11
    new_time.tm_mday = day + 1;       // День 1-31
    
    time_t t = mktime(&new_time);
    
    /* Устанавливаем время */
    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    
    /* Сохраняем в NVS */
    time_save_to_nvs();
    
    Serial.printf("[Clock] Время установлено: %02d:%02d:%02d %02d.%02d.%04d\n",
                  hour, min, sec, day + 1, month + 1, year);
    
    /* Закрываем настройки */
    if (settings_panel) {
        lv_obj_add_flag(settings_panel, LV_OBJ_FLAG_HIDDEN);
    }
}

static void show_settings_event_cb(lv_event_t* e)
{
    if (settings_panel) {
        lv_obj_clear_flag(settings_panel, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ============================================================
   Инициализация приложения
   ============================================================ */

void clock_app_init(void)
{
    /* Создаем экран */
    clock_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(clock_screen, lv_color_hex(0x2C3E50), 0);
    
    /* Большая метка времени по центру */
    time_label = lv_label_create(clock_screen);
    lv_label_set_text(time_label, "00:00:00");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -50);
    
    /* Таймер обновления */
    static lv_timer_t* update_timer;
    update_timer = lv_timer_create([](lv_timer_t* t) {
        time_t now = time_get_current();
        char buf[32];
        time_format_string(buf, sizeof(buf), now);
        if (time_label) {
            lv_label_set_text(time_label, buf);
        }
    }, 1000, NULL);
    
    /* Привязываем таймер к экрану для очистки */
    lv_obj_set_user_data(clock_screen, update_timer);
    
    /* Кнопка настройки */
    lv_obj_t* settings_btn = lv_btn_create(clock_screen);
    lv_obj_align(settings_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_size(settings_btn, 200, 50);
    
    lv_obj_t* settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, "Настроить время");
    lv_obj_center(settings_label);
    
    lv_obj_add_event_cb(settings_btn, show_settings_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Кнопка закрытия (красный крестик) */
    close_btn = lv_btn_create(clock_screen);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xE74C3C), 0);
    
    lv_obj_t* close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "X");
    lv_obj_center(close_label);
    
    lv_obj_add_event_cb(close_btn, close_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Панель настроек (скрыта по умолчанию) */
    settings_panel = lv_obj_create(clock_screen);
    lv_obj_set_size(settings_panel, 400, 250);
    lv_obj_align(settings_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(settings_panel, lv_color_hex(0x34495E), 0);
    lv_obj_set_style_radius(settings_panel, 10, 0);
    lv_obj_add_flag(settings_panel, LV_OBJ_FLAG_HIDDEN);
    
    /* Ролики для времени */
    const char* hour_opts = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
    const char* min_sec_opts = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59";
    
    roller_hour = lv_roller_create(settings_panel);
    lv_roller_set_options(roller_hour, hour_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(roller_hour, 20, 20);
    lv_roller_set_visible_row_count(roller_hour, 3);
    
    roller_min = lv_roller_create(settings_panel);
    lv_roller_set_options(roller_min, min_sec_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(roller_min, 100, 20);
    lv_roller_set_visible_row_count(roller_min, 3);
    
    roller_sec = lv_roller_create(settings_panel);
    lv_roller_set_options(roller_sec, min_sec_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(roller_sec, 180, 20);
    lv_roller_set_visible_row_count(roller_sec, 3);
    
    /* Ролики для даты */
    const char* day_opts = "01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31";
    const char* month_opts = "Янв\nФев\nМар\nАпр\nМай\nИюн\nИюл\nАвг\nСен\nОкт\nНоя\nДек";
    const char* year_opts = "2020\n2021\n2022\n2023\n2024\n2025\n2026\n2027\n2028\n2029\n2030";
    
    roller_day = lv_roller_create(settings_panel);
    lv_roller_set_options(roller_day, day_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(roller_day, 20, 120);
    lv_roller_set_visible_row_count(roller_day, 3);
    
    roller_month = lv_roller_create(settings_panel);
    lv_roller_set_options(roller_month, month_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(roller_month, 100, 120);
    lv_roller_set_visible_row_count(roller_month, 3);
    
    roller_year = lv_roller_create(settings_panel);
    lv_roller_set_options(roller_year, year_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(roller_year, 180, 120);
    lv_roller_set_visible_row_count(roller_year, 3);
    
    /* Кнопка сохранения */
    lv_obj_t* save_btn = lv_btn_create(settings_panel);
    lv_obj_set_size(save_btn, 150, 40);
    lv_obj_align(save_btn, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x27AE60), 0);
    
    lv_obj_t* save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, "Сохранить");
    lv_obj_center(save_label);
    
    lv_obj_add_event_cb(save_btn, save_time_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Загружаем экран */
    lv_scr_load(clock_screen);
    
    Serial.println("[Clock] Приложение запущено");
}

void clock_app_show_settings(void)
{
    if (settings_panel) {
        lv_obj_clear_flag(settings_panel, LV_OBJ_FLAG_HIDDEN);
    } else {
        /* Если приложение не запущено, запускаем */
        clock_app_init();
    }
}

void clock_save_time(time_t t)
{
    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    time_save_to_nvs();
}

time_t clock_load_time(void)
{
    time_t t;
    if (time_load_from_nvs(&t)) {
        return t;
    }
    return 0;
}

bool clock_sync_ntp(const char* ntp_server)
{
    return time_sync_ntp(ntp_server);
}

void clock_app_deinit(void)
{
    if (clock_screen) {
        lv_obj_del(clock_screen);
        clock_screen = nullptr;
    }
    settings_panel = nullptr;
    time_label = nullptr;
    close_btn = nullptr;
}
