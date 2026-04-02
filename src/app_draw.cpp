/**
 * @file app_draw.cpp
 * @brief Приложение рисовалки с палитрой цветов
 */

#include "app_draw.h"
#include <Arduino.h>

static lv_obj_t* draw_screen = nullptr;
static lv_obj_t* canvas = nullptr;
static lv_color_t brush_color = lv_color_hex(0x000000);  /* Черный по умолчанию */
static lv_obj_t* close_btn = nullptr;

/* Буфер для канваса */
#define CANVAS_WIDTH 480
#define CANVAS_HEIGHT 270  /* Оставляем место для палитры */
static lv_color_t canvas_buf[CANVAS_WIDTH * CANVAS_HEIGHT];
static lv_img_dsc_t canvas_img;

/* Палитра цветов */
static const lv_color_t color_palette[] = {
    lv_color_hex(0x000000), lv_color_hex(0xFFFFFF), lv_color_hex(0xFF0000),
    lv_color_hex(0x00FF00), lv_color_hex(0x0000FF), lv_color_hex(0xFFFF00),
    lv_color_hex(0xFF00FF), lv_color_hex(0x00FFFF), lv_color_hex(0xFFA500),
    lv_color_hex(0x800080), lv_color_hex(0x008000), lv_color_hex(0x000080),
    lv_color_hex(0x808080), lv_color_hex(0xC0C0C0), lv_color_hex(0x808000),
    lv_color_hex(0x800000)
};
#define PALETTE_SIZE (sizeof(color_palette) / sizeof(lv_color_t))

/* ============================================================
   Обработчики событий
   ============================================================ */

static void canvas_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
        lv_indev_t* indev = lv_indev_get_act();
        if (!indev) return;
        
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        
        /* Рисуем точку на канвасе */
        lv_canvas_set_px(canvas, point.x, point.y, brush_color);
    }
}

static void color_btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    lv_color_t* color = (lv_color_t*)lv_event_get_user_data(e);
    brush_color = *color;
}

static void clear_canvas_event_cb(lv_event_t* e)
{
    draw_clear_canvas();
}

static void bg_color_event_cb(lv_event_t* e)
{
    /* Смена цвета фона - заполняем канвас текущим цветом кисти */
    lv_color_t temp = brush_color;
    brush_color = lv_color_hex(0xFFFFFF);  /* Временно белый для заливки */
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            lv_canvas_set_px(canvas, x, y, temp);
        }
    }
    brush_color = temp;
}

static void close_event_cb(lv_event_t* e)
{
    lv_obj_del(draw_screen);
    draw_screen = nullptr;
    Serial.println("[Draw] Закрыто");
}

/* ============================================================
   Публичные функции
   ============================================================ */

void draw_set_brush_color(lv_color_t color)
{
    brush_color = color;
}

void draw_clear_canvas(void)
{
    /* Заполняем белым */
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            lv_canvas_set_px(canvas, x, y, lv_color_hex(0xFFFFFF));
        }
    }
}

/* ============================================================
   Инициализация приложения
   ============================================================ */

void draw_app_init(void)
{
    /* Создаем экран */
    draw_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(draw_screen, lv_color_hex(0xE0E0E0), 0);
    
    /* Инициализация канваса */
    canvas_img.header.always_zero = 0;
    canvas_img.header.w = CANVAS_WIDTH;
    canvas_img.header.h = CANVAS_HEIGHT;
    canvas_img.data_size = sizeof(canvas_buf);
    canvas_img.header.cf = LV_IMG_CF_TRUE_COLOR;
    
    canvas = lv_canvas_create(draw_screen);
    lv_canvas_set_buffer(canvas, canvas_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(canvas, 0, 0);
    lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    
    /* Очистка холста (белый фон) */
    draw_clear_canvas();
    
    /* Обработчик рисования */
    lv_obj_add_event_cb(canvas, canvas_event_cb, LV_EVENT_ALL, NULL);
    
    /* Панель инструментов снизу */
    lv_obj_t* toolbar = lv_obj_create(draw_screen);
    lv_obj_set_size(toolbar, 480, 50);
    lv_obj_align(toolbar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(toolbar, lv_color_hex(0x404040), 0);
    lv_obj_set_style_border_width(toolbar, 0, 0);
    lv_obj_set_style_pad_all(toolbar, 5, 0);
    lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toolbar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    /* Кнопки цветов */
    for (int i = 0; i < PALETTE_SIZE; i++) {
        lv_obj_t* color_btn = lv_btn_create(toolbar);
        lv_obj_set_size(color_btn, 28, 28);
        lv_obj_set_style_bg_color(color_btn, color_palette[i], 0);
        lv_obj_set_style_radius(color_btn, 4, 0);
        lv_obj_set_style_border_width(color_btn, 2, 0);
        lv_obj_set_style_border_color(color_btn, lv_color_white(), 0);
        
        lv_obj_add_event_cb(color_btn, color_btn_event_cb, LV_EVENT_CLICKED, (void*)&color_palette[i]);
    }
    
    /* Кнопка очистки */
    lv_obj_t* clear_btn = lv_btn_create(toolbar);
    lv_obj_set_size(clear_btn, 60, 35);
    lv_obj_set_style_bg_color(clear_btn, lv_color_hex(0xE74C3C), 0);
    
    lv_obj_t* clear_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_label, "Стер.");
    lv_obj_center(clear_label);
    
    lv_obj_add_event_cb(clear_btn, clear_canvas_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Кнопка закрытия */
    close_btn = lv_btn_create(draw_screen);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xE74C3C), 0);
    
    lv_obj_t* close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "X");
    lv_obj_center(close_label);
    
    lv_obj_add_event_cb(close_btn, close_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Загружаем экран */
    lv_scr_load(draw_screen);
    
    Serial.println("[Draw] Приложение запущено");
}

void draw_app_deinit(void)
{
    if (draw_screen) {
        lv_obj_del(draw_screen);
        draw_screen = nullptr;
    }
    canvas = nullptr;
}
