#include "header/LED_Matrix/LED_matrix_app.h"

void LED_matrix_app_init(void)
{
    printf("LED Matrix inicializada!\n");
}
void LED_matrix_app_dot_rgb(uint8_t row, uint8_t col, uint8_t r,uint8_t g,uint8_t b)
{
    printf("LED prendido: fila %d, columna %d, red %d, green %d, blue %d\n", row, col, r, g, b);
}
void LED_matrix_app_dot(uint8_t row, uint8_t col, color_t color)
{
    printf("LED prendido: fila %d, columna %d, red %d, green %d, blue %d\n", row, col, color.r, color.g, color.b);
}
void LED_matrix_app_brightness(int brightness)
{
    printf("Brillo actual: %d\n", brightness);
}