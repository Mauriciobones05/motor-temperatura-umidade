#include "hdc1080.h"

static HAL_StatusTypeDef HDC1080_LerRegistrador16(
    HDC1080_t *sensor,
    uint8_t registrador,
    uint16_t *valor)
{
    uint8_t dados[2];

    if (HAL_I2C_Master_Transmit(
            sensor->hi2c,
            HDC1080_ADDR,
            &registrador,
            1,
            100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_I2C_Master_Receive(
            sensor->hi2c,
            HDC1080_ADDR,
            dados,
            2,
            100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    *valor = ((uint16_t)dados[0] << 8) | dados[1];

    return HAL_OK;
}


static HAL_StatusTypeDef HDC1080_EscreverConfig(
    HDC1080_t *sensor,
    uint16_t config)
{
    uint8_t dados[3];

    dados[0] = HDC1080_REG_CONFIG;
    dados[1] = (uint8_t)(config >> 8);
    dados[2] = (uint8_t)(config & 0xFF);

    return HAL_I2C_Master_Transmit(
        sensor->hi2c,
        HDC1080_ADDR,
        dados,
        3,
        100
    );
}


HAL_StatusTypeDef HDC1080_Init(
    HDC1080_t *sensor,
    I2C_HandleTypeDef *hi2c)
{
    sensor->hi2c = hi2c;

    sensor->temperatura = 0.0f;
    sensor->umidade = 0.0f;

    sensor->manufacturer_id = 0;
    sensor->device_id = 0;

    HAL_Delay(20);

    /* Verifica se existe dispositivo no endereço 0x40 */
    if (HAL_I2C_IsDeviceReady(
            sensor->hi2c,
            HDC1080_ADDR,
            3,
            100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Lê Manufacturer ID */
    if (HDC1080_LerRegistrador16(
            sensor,
            HDC1080_REG_MAN_ID,
            &sensor->manufacturer_id) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Lê Device ID */
    if (HDC1080_LerRegistrador16(
            sensor,
            HDC1080_REG_DEV_ID,
            &sensor->device_id) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Confere se realmente é um HDC1080 */
    if (sensor->manufacturer_id != HDC1080_MAN_ID)
    {
        return HAL_ERROR;
    }

    if (sensor->device_id != HDC1080_DEV_ID)
    {
        return HAL_ERROR;
    }

    /*
     * CONFIG = 0x1000
     *
     * MODE = 1
     * mede temperatura + umidade em sequência
     *
     * Temperatura = 14 bits
     * Umidade = 14 bits
     */
    if (HDC1080_EscreverConfig(sensor, 0x1000) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}


HAL_StatusTypeDef HDC1080_LerDados(
    HDC1080_t *sensor)
{
    uint8_t registrador = HDC1080_REG_TEMP;
    uint8_t dados[4];

    uint16_t raw_temp;
    uint16_t raw_hum;

    /*
     * Selecionar 0x00 dispara a medição
     * de temperatura + umidade.
     */
    if (HAL_I2C_Master_Transmit(
            sensor->hi2c,
            HDC1080_ADDR,
            &registrador,
            1,
            100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /*
     * Aguarda as conversões.
     */
    HAL_Delay(15);

    /*
     * Recebe:
     *
     * dados[0] = TEMP MSB
     * dados[1] = TEMP LSB
     * dados[2] = HUM  MSB
     * dados[3] = HUM  LSB
     */
    if (HAL_I2C_Master_Receive(
            sensor->hi2c,
            HDC1080_ADDR,
            dados,
            4,
            100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    raw_temp =
        ((uint16_t)dados[0] << 8) |
        dados[1];

    raw_hum =
        ((uint16_t)dados[2] << 8) |
        dados[3];

    /*
     * Fórmulas do datasheet
     */
    sensor->temperatura =
        ((float)raw_temp / 65536.0f)
        * 165.0f - 40.0f;

    sensor->umidade =
        ((float)raw_hum / 65536.0f)
        * 100.0f;

    return HAL_OK;
}


float HDC1080_GetTemperatura(
    HDC1080_t *sensor)
{
    return sensor->temperatura;
}


float HDC1080_GetUmidade(
    HDC1080_t *sensor)
{
    return sensor->umidade;
}