#include "header/Accelerometer/TEST-Acelerometro.h"
#include <stdint.h>


static accel_t accelerations = {1, -1, 3};

void Accelerometro_Init(void)
{
    printf("Acelerometro iniciado!\n");
}

void Acelerometro_enable(void)
{
    printf("Acelerometro activado!\n");
}

void Acelerometro_disable(void)
{
    printf("Acelerometro desactivado!\n");
}


float Acelerometro_getXint(void)
{
    return accelerations.accel_x;
}
float Acelerometro_getYint(void)
{
    return accelerations.accel_y;
}
float Acelerometro_getZint(void)
{
    return accelerations.accel_z;
}


accel_t get_accelerations(void)
{
    return accelerations;
}