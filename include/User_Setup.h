/**
 * @file User_Setup.h
 * @brief Конфигурация TFT_eSPI для дисплея MKS TS35-R V2.0
 * 
 * Дисплей: 480x320, ILI9486/ILI9488 контроллер
 * Тачскрин: XPT2046 (резистивный)
 * Калибровка: {216, 3601, 236, 3579, 3}
 */

#define USER_SETUP_ID 255

/* --- Контроллер дисплея --- */
#define ILI9486_DRIVER
// #define ILI9488_DRIVER  // Раскомментировать если используется ILI9488

/* --- Настройки пинов для MKS TS35 --- */
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1  // Нет подключения или объединен с CS

#define TOUCH_CS 33

/* --- Разрешение и ориентация --- */
#define TFT_WIDTH  320
#define TFT_HEIGHT 480
#define TFT_ROTATION ROTATION_1  // Альбомная ориентация

/* --- SPI частоты --- */
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

/* --- Цвета --- */
#define TFT_INVERSION_ON

/* --- Калибровка тачскрина --- */
// Формат: [Xmin, Xmax, Ymin, Ymax, rotation]
// Ваши данные: {216, 3601, 236, 3579, 3}
#define TOUCH_CALIBRATION_X 216, 3601
#define TOUCH_CALIBRATION_Y 236, 3579
#define TOUCH_CALIBRATION_ROTATION 3

/* --- Опции --- */
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

/* --- Поддержка PSRAM --- */
#define USE_SPI_BUFFER
