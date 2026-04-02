/**
 * @file app_desktop.cpp
 * @brief Реализация рабочего стола с иконками приложений
 */

#include "app_desktop.h"
#include "app_clock.h"
#include "app_calculator.h"
#include "app_draw.h"
#include "app_media.h"
#include "system_time.h"
#include <Arduino.h>

/* --- Глобальные объекты LVGL --- */
static lv_obj_t* desktop_screen = nullptr;
static lv_obj_t* clock_label = nullptr;
static lv_timer_t* clock_timer = nullptr;

/* --- Позиции иконок --- */
#define ICON_SIZE 80
#define ICON_SPACING 20
#define DESKTOP_PADDING 20

typedef struct {
    const char* name;
    const char* emoji;
    lv_color_t color;
    void (*on_click)(void);
} DesktopIcon;

/* Прототипы обработчиков */
static void icon_clock_click(void);
static void icon_calc_click(void);
static void icon_draw_click(void);
static void icon_media_click(void);

static DesktopIcon icons[] = {
    {"Часы", "\u23F0", lv_color_hex(0x4A90E2), icon_clock_click},
    {"Калькулятор", "\uD83E\uDDEE", lv_color_hex(0xE24A4A), icon_calc_click},
    {"Рисовалка", "\uD83C\uDFA8", lv_color_hex(0x50C878), icon_draw_click},
    {"Медиаплеер", "\uD83D\uDCFA", lv_color_hex(0xF5A623), icon_media_click},
    {nullptr, nullptr, 0, nullptr}
};

/* ============================================================
   Обработчики нажатий
   ============================================================ */

static void icon_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DesktopIcon* icon = (DesktopIcon*)lv_event_get_user_data(e);
        if (icon && icon->on_click) {
            icon->on_click();
        }
    }
}

static void icon_clock_click(void)
{
    Serial.println("[Desktop] Открытие часов");
    clock_app_init();
}

static void icon_calc_click(void)
{
    Serial.println("[Desktop] Открытие калькулятора");
    calculator_app_init();
}

static void icon_draw_click(void)
{
    Serial.println("[Desktop] Открытие рисовалки");
    draw_app_init();
}

static void icon_media_click(void)
{
    Serial.println("[Desktop] Открытие медиаплеера");
    media_app_init();
}

/* ============================================================
   Обновление часов
   ============================================================ */

static void clock_timer_cb(lv_timer_t* timer)
{
    time_t now = time_get_current();
    char buf[32];
    time_format_string(buf, sizeof(buf), now);
    
    if (clock_label) {
        lv_label_set_text(clock_label, buf);
    }
}

/* ============================================================
   Инициализация рабочего стола
   ============================================================ */

void desktop_init(void)
{
    /* Создаем основной экран */
    desktop_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(desktop_screen);
    lv_obj_set_style_bg_color(desktop_screen, lv_color_hex(0x008080), 0);  // Цвет в стиле Windows XP
    
    /* Фон - градиент */
    static lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(0x3B82C9);  // Голубой верх
    grad.stops[0].frac = 0;
    grad.stops[1].color = lv_color_hex(0x7CB342);  // Зеленый низ
    grad.stops[1].frac = 255;
    
    lv_obj_set_style_local_bg_grad(desktop_screen, LV_PART_MAIN, LV_STYLE_BG_GRAD, &grad);
    
    /* Создаем иконки */
    int icon_count = 0;
    for (int i = 0; icons[i].name != nullptr; i++) {
        DesktopIcon* icon = &icons[i];
        
        /* Контейнер иконки */
        lv_obj_t* icon_cont = lv_obj_create(desktop_screen);
        lv_obj_set_size(icon_cont, ICON_SIZE, ICON_SIZE + 20);
        lv_obj_set_pos(icon_cont, 
                       DESKTOP_PADDING + (ICON_SIZE + ICON_SPACING) * (i % 4),
                       DESKTOP_PADDING + (ICON_SIZE + ICON_SPACING + 20) * (i / 4));
        lv_obj_remove_style(icon_cont, NULL, LV_PART_SCROLLBAR);
        lv_obj_set_style_bg_opa(icon_cont, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(icon_cont, 0, 0);
        lv_obj_set_style_bg_opa(icon_cont, LV_OPA_20, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(icon_cont, lv_color_white(), LV_STATE_PRESSED);
        
        /* Эмодзи/символ иконки */
        lv_obj_t* icon_symbol = lv_label_create(icon_cont);
        lv_label_set_text(icon_symbol, icon->emoji);
        lv_obj_set_style_text_font(icon_symbol, &lv_font_montserrat_32, 0);
        lv_obj_align(icon_symbol, LV_ALIGN_TOP_MID, 0, 5);
        
        /* Название иконки */
        lv_obj_t* icon_label = lv_label_create(icon_cont);
        lv_label_set_text(icon_label, icon->name);
        lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
        lv_obj_align(icon_label, LV_ALIGN_BOTTOM_MID, 0, -5);
        
        /* Привязка данных и обработчика */
        lv_obj_add_event_cb(icon_cont, icon_event_cb, LV_EVENT_ALL, icon);
        
        icon_count++;
    }
    
    /* Системные часы в правом нижнем углу */
    lv_obj_t* clock_panel = lv_obj_create(desktop_screen);
    lv_obj_set_size(clock_panel, 120, 50);
    lv_obj_align(clock_panel, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(clock_panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(clock_panel, LV_OPA_50, 0);
    lv_obj_set_style_radius(clock_panel, 8, 0);
    lv_obj_set_style_border_width(clock_panel, 0, 0);
    
    clock_label = lv_label_create(clock_panel);
    lv_label_set_text(clock_label, "--:--:--");
    lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(clock_label, lv_color_white(), 0);
    lv_obj_center(clock_label);
    
    /* Обработчик нажатия на панель часов */
    lv_obj_add_event_cb(clock_panel, [](lv_event_t* e) {
        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_CLICKED) {
            clock_app_show_settings();
        }
    }, LV_EVENT_CLICKED, NULL);
    
    /* Таймер обновления часов */
    clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
    
    /* Загружаем экран */
    lv_scr_load(desktop_screen);
    
    Serial.println("[Desktop] Инициализирован");
}

void desktop_update_clock(void)
{
    if (clock_timer) {
        clock_timer_cb(clock_timer);
    }
}

void desktop_deinit(void)
{
    if (clock_timer) {
        lv_timer_del(clock_timer);
        clock_timer = nullptr;
    }
    
    if (desktop_screen) {
        lv_obj_del(desktop_screen);
        desktop_screen = nullptr;
    }
}
