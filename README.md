# Monitor de Temperatura e Umidade

Firmware embarcado para monitoramento de **temperatura e umidade** utilizando o sensor **HDC1080** conectado via I²C a uma **STM32 NUCLEO-F334R8**.

O projeto foi desenvolvido com **STM32CubeMX + STM32 HAL**, utilizando **CMake** e **GCC ARM Embedded** para compilação.

## Visão geral

O firmware executa um ciclo periódico de aquisição e controle:

1. Inicializa os periféricos configurados no STM32CubeMX.
2. Verifica a presença do dispositivo no barramento I²C.
3. Inicializa o HDC1080 e valida seus identificadores.
4. Realiza a leitura de temperatura e umidade.
5. Lê o ADC para definir o limite de temperatura.
6. Compara a temperatura medida com o limite configurado.
7. Aciona ou desliga o LED da placa e um LED externo.
8. Em caso de falha de comunicação ou inicialização do sensor, entra em estado de erro e sinaliza a condição pelo LED.

## Hardware

### Microcontrolador

- **MCU:** STM32F334R8T6
- **Placa:** NUCLEO-F334R8
- **Package:** LQFP64
- **Clock do sistema:** 64 MHz

### Sensor

- **Sensor:** Texas Instruments HDC1080
- **Interface:** I²C
- **Endereço I²C:** `0x40`
- **Medições:** temperatura e umidade relativa
- O firmware valida o **Manufacturer ID `0x5449`** e o **Device ID `0x1050`** antes de aceitar o sensor.

## Periféricos utilizados

| Periférico | Configuração | Função |
|---|---|---|
| ADC1 | 12 bits, canal ADC1_IN1 | Define o limite de temperatura |
| I²C1 | PB8/PB9 | Comunicação com HDC1080 |
| TIM6 | Prescaler 63999 / Period 999 | Temporização periódica da aplicação |
| USART2 | 38400 baud, 8N1 | Comunicação serial disponível |
| GPIO PA5 | LED da NUCLEO | Indicação de temperatura/erro |
| GPIO PB5 | LED_EXT | Saída para LED externo |
| GPIO PC13 | B1 | Botão da NUCLEO / EXTI |

## Pinagem principal

| Pino | Função |
|---|---|
| PA0 | ADC1_IN1 |
| PA2 | USART2_TX |
| PA3 | USART2_RX |
| PA5 | LD2 – LED verde da NUCLEO |
| PB5 | LED externo |
| PB8 | I²C1_SCL |
| PB9 | I²C1_SDA |
| PC13 | Botão B1 |
| PA13 | SWDIO |
| PA14 | SWCLK |

## Lógica de controle

A aplicação utiliza uma máquina de estados:

`ESTADO_INICIALIZAR → ESTADO_ESPERAR → ESTADO_LER_SENSOR → ESTADO_LER_ADC → ESTADO_VERIFICAR → ESTADO_ATUALIZAR_LED`

Em caso de falha:

`ESTADO_ERRO → ESTADO_INICIALIZAR`

### Limite de temperatura

O valor do limite é obtido pelo ADC e convertido linearmente entre **20 °C e 40 °C**:

- ADC = 0 → 20 °C
- ADC = 4095 → 40 °C

Quando:

`temperatura >= limite_temperatura`

o firmware liga:

- LED verde da NUCLEO;
- LED externo conectado ao PB5.

Caso contrário, ambos permanecem desligados.

> **Observação:** o código atual não utiliza a variável `umidade` para tomada de decisão; ela é medida e armazenada para uso posterior.

## Driver HDC1080

O driver está separado em:

- `Core/Inc/hdc1080.h`
- `Core/Src/hdc1080.c`

Principais funções:

- `HDC1080_Init()` — inicializa e valida o sensor.
- `HDC1080_LerDados()` — realiza a aquisição de temperatura e umidade.
- `HDC1080_GetTemperatura()` — retorna a temperatura medida.
- `HDC1080_GetUmidade()` — retorna a umidade medida.

A conversão utiliza as equações especificadas para o HDC1080:

`Temperatura = (raw / 65536) × 165 − 40`

`Umidade = (raw / 65536) × 100`

## Estrutura do projeto

```text
motor-temperatura-umidade/
├── Core/
│   ├── Inc/
│   │   ├── hdc1080.h
│   │   ├── main.h
│   │   ├── stm32f3xx_hal_conf.h
│   │   └── stm32f3xx_it.h
│   └── Src/
│       ├── hdc1080.c
│       ├── main.c
│       ├── stm32f3xx_hal_msp.c
│       ├── stm32f3xx_it.c
│       ├── syscalls.c
│       ├── sysmem.c
│       └── system_stm32f3xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F3xx_HAL_Driver/
├── cmake/
│   ├── gcc-arm-none-eabi.cmake
│   └── stm32cubemx/
├── CMakeLists.txt
├── CMakePresets.json
├── Monitor_Temperatura_Umidade.ioc
├── STM32F334xx_FLASH.ld
└── startup_stm32f334x8.s
```

## Requisitos

Para abrir, configurar ou compilar o projeto, recomenda-se:

- STM32CubeMX **6.18.1** ou compatível
- STM32Cube FW_F3 **V1.11.6**
- GCC ARM Embedded (`arm-none-eabi-gcc`)
- CMake **3.22** ou superior
- Ninja
- Placa STM32 NUCLEO-F334R8
- Sensor HDC1080

## Compilação com CMake

O projeto possui presets para **Debug** e **Release**.

### Debug

```bash
cmake --preset Debug
cmake --build --preset Debug
```

### Release

```bash
cmake --preset Release
cmake --build --preset Release
```

Os arquivos de build são gerados dentro de:

```text
build/Debug/
build/Release/
```

## Configuração pelo STM32CubeMX

O arquivo de configuração principal é:

```text
Monitor_Temperatura_Umidade.ioc
```

A configuração atual utiliza:

- MCU STM32F334R8Tx
- NUCLEO-F334R8
- ADC1
- I²C1
- TIM6
- USART2
- GPIO/EXTI
- SWD para debug

Ao modificar o arquivo `.ioc`, o código gerado pelo STM32CubeMX pode sobrescrever trechos fora das áreas `USER CODE`. Alterações específicas da aplicação devem permanecer dentro das áreas destinadas ao código do usuário ou nos módulos próprios, como o driver `hdc1080.c`.

## Tratamento de erros

Se o HDC1080 não responder, apresentar identificadores diferentes dos esperados ou ocorrer falha durante a leitura I²C, a aplicação entra em `ESTADO_ERRO`.

Nesse estado, o LED da NUCLEO é alternado e, após um pequeno atraso, o firmware tenta novamente inicializar o sensor.

## Estado atual

O repositório contém uma implementação funcional de base para:

- aquisição de temperatura;
- aquisição de umidade;
- comunicação I²C com HDC1080;
- leitura analógica para definição do limite;
- controle de LEDs;
- temporização por TIM6;
- máquina de estados;
- projeto configurado pelo STM32CubeMX;
- build via CMake.

A USART2 está configurada e disponível, porém a lógica atual não possui uma camada de telemetria serial implementada no fluxo principal.

## Melhorias futuras

Algumas extensões naturais para o projeto são:

- exibição das medições em LCD;
- registro de temperatura e umidade;
- comunicação serial para diagnóstico/telemetria;
- alarmes independentes de temperatura e umidade;
- configuração do limite sem recompilar o firmware;
- inclusão de RTC para data/hora das medições;
- filtro ou média das leituras;
- watchdog para aumentar a robustez da aplicação;
- tratamento mais detalhado de falhas do barramento I²C.

## Licença

Nenhuma licença específica foi identificada na estrutura atual do repositório. Caso o projeto seja distribuído publicamente, recomenda-se adicionar um arquivo `LICENSE` com os termos desejados.

---

**Projeto:** Monitor de Temperatura e Umidade  
**MCU:** STM32F334R8T6  
**Sensor:** HDC1080  
**Placa:** NUCLEO-F334R8  
**Interface do sensor:** I²C
