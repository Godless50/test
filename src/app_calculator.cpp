/**
 * @file app_calculator.cpp
 * @brief Полноэкранный калькулятор с базовыми операциями
 */

#include "app_calculator.h"
#include <Arduino.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static lv_obj_t* calc_screen = nullptr;
static lv_obj_t* display_label = nullptr;
static lv_obj_t* close_btn = nullptr;

/* Буфер для текущего числа */
static char current_num[32] = "0";
static double prev_value = 0;
static char operation = 0;
static bool new_number = true;

/* ============================================================
   Логика калькулятора
   ============================================================ */

static void append_digit(int digit)
{
    if (new_number) {
        snprintf(current_num, sizeof(current_num), "%d", digit);
        new_number = false;
    } else {
        if (strlen(current_num) < 15) {
            strcat(current_num, itoa(digit, current_num + strlen(current_num), 10));
        }
    }
    lv_label_set_text(display_label, current_num);
}

static void append_dot(void)
{
    if (new_number) {
        strcpy(current_num, "0.");
        new_number = false;
    } else if (strchr(current_num, '.') == NULL) {
        strcat(current_num, ".");
    }
    lv_label_set_text(display_label, current_num);
}

static void clear_all(void)
{
    strcpy(current_num, "0");
    prev_value = 0;
    operation = 0;
    new_number = true;
    lv_label_set_text(display_label, current_num);
}

static void delete_last(void)
{
    int len = strlen(current_num);
    if (len > 1) {
        current_num[len - 1] = '\0';
    } else {
        strcpy(current_num, "0");
        new_number = true;
    }
    lv_label_set_text(display_label, current_num);
}

static void set_operation(char op)
{
    prev_value = atof(current_num);
    operation = op;
    new_number = true;
}

static void calculate_result(void)
{
    if (operation == 0) return;
    
    double curr = atof(current_num);
    double result = 0;
    
    switch (operation) {
        case '+': result = prev_value + curr; break;
        case '-': result = prev_value - curr; break;
        case '*': result = prev_value * curr; break;
        case '/': 
            if (curr != 0) {
                result = prev_value / curr;
            } else {
                lv_label_set_text(display_label, "Ошибка");
                clear_all();
                return;
            }
            break;
    }
    
    snprintf(current_num, sizeof(current_num), "%.8g", result);
    lv_label_set_text(display_label, current_num);
    
    operation = 0;
    prev_value = 0;
    new_number = true;
}

/* ============================================================
   Обработчики кнопок
   ============================================================ */

static void btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    lv_obj_t* btn = lv_event_get_target(e);
    const char* text = lv_label_get_text(lv_obj_get_child(btn, 0));
    
    if (strcmp(text, "C") == 0) {
        clear_all();
    } else if (strcmp(text, "⌫") == 0) {
        delete_last();
    } else if (strcmp(text, "+") == 0 || strcmp(text, "-") == 0 ||
               strcmp(text, "×") == 0 || strcmp(text, "÷") == 0) {
        char op = text[0];
        if (op == '×') op = '*';
        if (op == '÷') op = '/';
        set_operation(op);
    } else if (strcmp(text, "=") == 0) {
        calculate_result();
    } else if (strcmp(text, ".") == 0) {
        append_dot();
    } else {
        append_digit(atoi(text));
    }
}

static void close_event_cb(lv_event_t* e)
{
    clear_all();
    lv_obj_del(calc_screen);
    calc_screen = nullptr;
    Serial.println("[Calculator] Закрыто");
}

/* ============================================================
   Инициализация приложения
   ============================================================ */

void calculator_app_init(void)
{
    /* Создаем экран */
    calc_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(calc_screen, lv_color_hex(0x2C2C2C), 0);
    
    /* Дисплей */
    lv_obj_t* display_panel = lv_obj_create(calc_screen);
    lv_obj_set_size(display_panel, 460, 80);
    lv_obj_align(display_panel, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(display_panel, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_radius(display_panel, 8, 0);
    lv_obj_set_style_border_width(display_panel, 0, 0);
    
    display_label = lv_label_create(display_panel);
    lv_label_set_text(display_label, "0");
    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(display_label, lv_color_white(), 0);
    lv_obj_align(display_label, LV_ALIGN_RIGHT_MID, -10, 0);
    
    /* Кнопка закрытия */
    close_btn = lv_btn_create(calc_screen);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xE74C3C), 0);
    
    lv_obj_t* close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "X");
    lv_obj_center(close_label);
    
    lv_obj_add_event_cb(close_btn, close_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Сетка кнопок */
    static const char* btn_texts[] = {
        "C", "⌫", "/", "*",
        "7", "8", "9", "-",
        "4", "5", "6", "+",
        "1", "2", "3", "=",
        "0", ".", "", ""
    };
    
    /* Цвета кнопок */
    static lv_color_t btn_colors[] = {
        lv_color_hex(0xFF9500), lv_color_hex(0xA5A5A5), lv_color_hex(0xFF9500), lv_color_hex(0xFF9500),
        lv_color_hex(0x505050), lv_color_hex(0x505050), lv_color_hex(0x505050), lv_color_hex(0xFF9500),
        lv_color_hex(0x505050), lv_color_hex(0x505050), lv_color_hex(0x505050), lv_color_hex(0xFF9500),
        lv_color_hex(0x505050), lv_color_hex(0x505050), lv_color_hex(0x505050), lv_color_hex(0xFF9500),
        lv_color_hex(0x505050), lv_color_hex(0x505050), 0, 0
    };
    
    int btn_idx = 0;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            if (btn_texts[btn_idx][0] == '\0') {
                btn_idx++;
                continue;
            }
            
            lv_obj_t* btn = lv_btn_create(calc_screen);
            lv_obj_set_size(btn, 100, 70);
            lv_obj_set_pos(btn, 10 + col * 115, 110 + row * 80);
            
            if (btn_colors[btn_idx].full != 0) {
                lv_obj_set_style_bg_color(btn, btn_colors[btn_idx], 0);
            } else {
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x505050), 0);
            }
            
            lv_obj_set_style_radius(btn, 8, 0);
            lv_obj_set_style_shadow_width(btn, 2, 0);
            
            lv_obj_t* label = lv_label_create(btn);
            lv_label_set_text(label, btn_texts[btn_idx]);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
            lv_obj_center(label);
            
            lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
            
            btn_idx++;
        }
    }
    
    /* Загружаем экран */
    lv_scr_load(calc_screen);
    
    Serial.println("[Calculator] Приложение запущено");
}

void calculator_app_deinit(void)
{
    if (calc_screen) {
        lv_obj_del(calc_screen);
        calc_screen = nullptr;
    }
    clear_all();
}
