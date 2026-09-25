#ifndef HDC1080_H
#define HDC1080_H

#include "stm32f3xx_hal.h"

/* Endereço I2C */
#define HDC1080_ADDR           (0x40 << 1)

/* Registradores */
#define HDC1080_REG_TEMP       0x00
#define HDC1080_REG_HUM        0x01
#define HDC1080_REG_CONFIG     0x02
#define HDC1080_REG_MAN_ID     0xFE
#define HDC1080_REG_DEV_ID     0xFF

/* IDs esperados */
#define HDC1080_MAN_ID         0x5449
#define HDC1080_DEV_ID         0x1050

typedef struct
{
    I2C_HandleTypeDef *hi2c;

    float temperatura;
    float umidade;

    uint16_t manufacturer_id;
    uint16_t device_id;

} HDC1080_t;


/* Inicializa e verifica o sensor */
HAL_StatusTypeDef HDC1080_Init(
    HDC1080_t *sensor,
    I2C_HandleTypeDef *hi2c
);

/* Lê temperatura + umidade */
HAL_StatusTypeDef HDC1080_LerDados(
    HDC1080_t *sensor
);

/* Retorna os valores já medidos */
float HDC1080_GetTemperatura(HDC1080_t *sensor);

float HDC1080_GetUmidade(HDC1080_t *sensor);

#endif