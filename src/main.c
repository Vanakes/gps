#include "main.h"
#include "milis.h"
#include <stdbool.h>
#include <stdio.h>
#include <stm8s.h>
#include "uart1.h"
#include <string.h>
#include "LCD_I2C.h"
#include "delay.h"

#define BUFFER_SIZE 128

void init(void) {
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
#if defined(BTN_PORT) || defined(BTN_PIN)
    GPIO_Init(BTN_PORT, BTN_PIN, GPIO_MODE_IN_FL_NO_IT);
#endif

    init_milis();
    init_uart1();

    UART3_DeInit();
    UART3_Init(9600, UART3_WORDLENGTH_8D, UART3_STOPBITS_1, UART3_PARITY_NO,
               UART3_MODE_TXRX_ENABLE);
}

char latitude[16] = {0};
char longitude[16] = {0};
char date[16] = {0};

void processNMEA(char *buffer) {
    if (strstr(buffer, "$GPGGA") != NULL) {
        char *token = strtok(buffer, ",");
        int fieldIndex = 0;

        while (token != NULL) {
            fieldIndex++;
            if (fieldIndex == 2) {
            } else if (fieldIndex == 3) {
                strncpy(latitude, token, sizeof(latitude) - 1);
            } else if (fieldIndex == 4) {  
            } else if (fieldIndex == 5) {
                strncpy(longitude, token, sizeof(longitude) - 1);
            }
            token = strtok(NULL, ",");
        }
    } else if (strstr(buffer, "$GPRMC") != NULL) {
        char *token = strtok(buffer, ",");
        int fieldIndex = 0;

        while (token != NULL) {
            fieldIndex++;
            if (fieldIndex == 9) { 
                strncpy(date, token, sizeof(date) - 1);
            }
            token = strtok(NULL, ",");
        }
    }
}

void displayLatitude() {
    LCD_I2C_SetCursor(0, 0);
    LCD_I2C_Print("Dnes je:");
    LCD_I2C_Print(date); 
    LCD_I2C_SetCursor(0, 1);
    LCD_I2C_Print("Sirka:");
    LCD_I2C_Print(latitude); 
}

void displayLongitude() {
    LCD_I2C_SetCursor(0, 0);
    LCD_I2C_Print("Dnes je:");
    LCD_I2C_Print(date); 
    LCD_I2C_SetCursor(0, 1);
    LCD_I2C_Print("Delka:");
    LCD_I2C_Print(longitude); 
}

int main(void) {
    uint32_t time = 0;
    char buffer[BUFFER_SIZE] = {0};
    uint8_t bufferIndex = 0;
    bool displayLat = true;

    LCD_I2C_Init(0x27, 16, 2);
    init();

    displayLatitude();

    while (1) {
        if (UART3_GetFlagStatus(UART3_FLAG_RXNE) != RESET) {
            char receivedChar = UART3_ReceiveData8();
            if (receivedChar == '\n' || receivedChar == '\r') {
                buffer[bufferIndex] = '\0';
                processNMEA(buffer);
                bufferIndex = 0;
                memset(buffer, 0, sizeof(buffer));
            } else {
                if (bufferIndex < BUFFER_SIZE - 1) {
                    buffer[bufferIndex++] = receivedChar;
                }
            }
        }

        if (milis() - time > 10000) { 
            time = milis();
            displayLat = !displayLat;

            if (displayLat) {
                displayLatitude();
            } else {
                displayLongitude();
            }


            GPIO_WriteReverse(LED_PORT, LED_PIN);
        }
    }
}
