/**
 * @file lv_conf.h
 * @brief Конфигурация LVGL для ESP32 + MKS TS35
 * 
 * Важные настройки:
 * - Включена поддержка PSRAM для буферов
 * - Разрешение 480x320 (альбомная ориентация)
 * - Оптимизация памяти
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* -- Базовые настройки -- */
#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   1
#define LV_COLOR_SCREEN_TRANSP 0

#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_DPI_DEF 130

/* -- Память -- */
#define LV_MEM_CUSTOM      1
#if LV_MEM_CUSTOM == 0
    #define LV_MEM_SIZE    (48U * 1024U)
#else
    /* Используем внешнюю PSRAM если доступна */
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif

#define LV_MEMCPY_MEMSET_STD 0

/* -- Буферы рендеринга -- */
#define LV_DISP_BUF_MAX_SIZE (480 * 320 / 10) /* 1/10 экрана в PSRAM */
#define LV_DISP_DOUBLE_BUFFER 1

/* -- Логирование -- */
#define LV_USE_LOG      1
#define LV_LOG_LEVEL    LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF   1

/* -- Модули -- */
#define LV_USE_BTN          1
#define LV_USE_LABEL        1
#define LV_USE_IMG          1
#define LV_USE_ARC          1
#define LV_USE_BAR          1
#define LV_USE_KEYBOARD     1
#define LV_USE_CHECKBOX     1
#define LV_USE_DROPDOWN     1
#define LV_USE_ROLLER       1
#define LV_USE_TABLE        1
#define LV_USE_TABVIEW      1
#define LV_USE_TILEVIEW     1
#define LV_USE_WIN          1
#define LV_USE_CANVAS       1
#define LV_USE_LED          1
#define LV_USE_ANIMIMG      1

/* -- Темы -- */
#define LV_USE_THEME_DEFAULT    1
#define LV_THEME_DEFAULT_GROW   1
#define LV_THEME_DEFAULT_DARK   0

/* -- Шрифты -- */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_48 1

/* -- Текстовые поля -- */
#define LV_TXT_ENC           LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS   1
#define LV_TXT_LINE_BREAK_LONG_LEN 12

/* -- Графика -- */
#define LV_USE_SVG          0
#define LV_USE_VECTOR_GRAPHIC 0

/* -- Системные параметры -- */
#define LV_ATTRIBUTE_TICK_INC
#define LV_ATTRIBUTE_TIMER_HANDLER
#define LV_ATTRIBUTE_FLUSH_READY
#define LV_ATTRIBUTE_ALIGN_DMA_STACK

/* -- Инпут устройства -- */
#define LV_INDEV_DEF_READ_PERIOD 30

/* -- Прочее -- */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

#define LV_USE_SNAPSHOT 0
#define LV_USE_FREETYPE 0

/* Кастомные функции инициализации */
#define LV_USER_DATA_FREE_INCLUDE "esp_heap_caps.h"
#define LV_USER_DATA_FREE "heap_caps_free"

#endif /*LV_CONF_H*/
