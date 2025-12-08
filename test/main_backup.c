//#include <reg51.h>       // old header from SDCC
// #include "STC8Fxx.h"     // Official header from STC-ISP for STC8Fxx / STC8Axx
// #include "STC12C5Axx.h"  // Official header from STC-ISP for STC10Fxx / STC11Fxx / STC12C5Axx / STC12C52xx
// #include "STC12C20xx.h"  // Official header from STC-ISP for STC12C20xx
// #include "STC12C54xx.h"  // Official header from STC-ISP for STC12C54xx
// #include "STC12C56xx.h"  // Official header from STC-ISP for STC12C56xx
// #include "STC15F104E.h"  // Official header from STC-ISP for STC15F204EA / STC15F104E
// #include "STC89xx.h"     // Official header from STC-ISP for STC89xx / STC90xx
#include "STC15Fxx.h"    // Official header from STC-ISP for STC15Wxx / STC15Fxx

#define FOSC 11059200L
#define BAUD 9600

// BTT Relay 1.2 - Klipper PSU Pin Definitions (STC15W201S)
// Pin 1: P3.0 (RXD), Pin 2: P3.1 (TXD), Pin 3: P5.4 (Reset)
// Pin 4: GND, Pin 5: P5.5 (Relay), Pin 6: P3.2 (Analog Input)
// Pin 7: P3.3 (Short Circuit Detection), Pin 8: VCC
#define RELAY P55                      // Relay control output (P5.5 - Pin 5)
#define POWER_IN_PIN P32               // Analog input for relay control (P3.2 - Pin 6)
#define POWER_SHORTCIRCUIT_DET P33     // Short circuit detection (P3.3 - Pin 7)

void UART_Init(void)
{
    SCON = 0x50;
    TMOD = 0x20;
    TH1 = TL1 = 256 - (FOSC / 32 / BAUD);
    TR1 = 1;
    EA = 1;
    ES = 1;
}

void UART_Send(char dat)
{
    SBUF = dat;
    while (!TI);
    TI = 0;
}

void UART_SendStr(char *str)
{
    while (*str) UART_Send(*str++);
}

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 123; j++);
}

void main(void)
{
    UART_Init();
    
    P3M0 = 0x00;
    P3M1 = 0x00;
    P5M0 = 0x20;
    P5M1 = 0x00;
    
    RELAY = 0;
    
    char buf[8];
    int idx = 0;
    unsigned char sc = 0, pwr = 0, lpwr = POWER_IN_PIN;

    while (1)
    {
        // Check for short circuit on P3.3
        if (!POWER_SHORTCIRCUIT_DET && !shortcircuit_detected)
        {
            shortcircuit_detected = 1;
            RELAY = 0;  // Turn off relay immediately
            UART_SendString("ERROR:SHORTCIRCUIT\n");
        }
        else if (POWER_SHORTCIRCUIT_DET && shortcircuit_detected)
        {
            shortcircuit_detected = 0;
            UART_SendString("INFO:SHORTCIRCUIT_CLEARED\n");
        }
        
        // Monitor analog input P3.2 for relay on/off control
        // Monitor analog input P3.2 for relay on/off control
        if (POWER_IN_PIN != last_power_state)
        {
            delay_ms(50); // Debounce
            if (POWER_IN_PIN != last_power_state)
            {
                last_power_state = POWER_IN_PIN;
                power_present = POWER_IN_PIN;
                
                if (power_present && !shortcircuit_detected)
                {
                    RELAY = 1;  // Turn relay ON
                    UART_SendString("RELAY:ON\n");
                }
                else
                {
                    RELAY = 0;  // Turn relay OFF
                    UART_SendString("RELAY:OFF\n");
                }
            }
        }
        
        // Process UART commands
        if (RI)
        {
            buffer[index++] = SBUF;
            RI = 0;
            if (buffer[index-1] == '\n' || index >= 9)
            {
                buffer[index] = '\0'; // Null-terminate
                
                // Remove trailing newline/carriage return
                if (index > 0 && (buffer[index-1] == '\n' || buffer[index-1] == '\r'))
                    buffer[index-1] = '\0';
                if (index > 1 && (buffer[index-2] == '\n' || buffer[index-2] == '\r'))
                    buffer[index-2] = '\0';
                
                // Klipper PSU commands
                if (buffer[0] == 'o' && buffer[1] == 'n' && buffer[2] == '\0')
                {
                    if (!shortcircuit_detected)
                    {
                        RELAY = 1;
                        UART_SendString("ok\n");
                    }
                    else
                    {
                        UART_SendString("error\n");
                    }
                }
                else if (buffer[0] == 'o' && buffer[1] == 'f' && buffer[2] == 'f' && buffer[3] == '\0')
                {
                    RELAY = 0;
                    UART_SendString("ok\n");
                }
                else if (buffer[0] == 's' && buffer[1] == 't' && buffer[2] == 'a' && buffer[3] == 't' && buffer[4] == 'u' && buffer[5] == 's' && buffer[6] == '\0')
                {
                    // Send status report
                    UART_SendString("STATUS:");
                    UART_SendString(RELAY ? "ON" : "OFF");
                    UART_SendString(",POWER:");
                    UART_SendString(power_present ? "OK" : "LOST");
                    UART_SendString(",SHORT:");
                    UART_SendString(shortcircuit_detected ? "YES" : "NO");
                    UART_SendString("\n");
                }
                else if (buffer[0] == 'g' && buffer[1] == 'e' && buffer[2] == 't' && buffer[3] == '\0')
                {
                    // Get relay state (compatible with original)
                    UART_SendString(RELAY ? "ON\n" : "OFF\n");
                }
                else
                {
                    UART_SendString("unknown\n");
                }
                index = 0;
            }
        }
    }
}
