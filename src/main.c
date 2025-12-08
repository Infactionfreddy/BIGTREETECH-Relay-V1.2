/*
 * BTT Relay 1.2 Firmware - Klipper Power PSU Kompatibel
 * 
 * Hardware: BIGTREETECH Relay V1.2
 * MCU: STC15W201S @ 11.0592 MHz (1KB Flash, 256B RAM)
 * 
 * Pin-Belegung:
 * - P3.0 (Pin 1): UART RXD - Empfang von Klipper-Befehlen
 * - P3.1 (Pin 2): UART TXD - Senden von Status-Meldungen
 * - P3.2 (Pin 3): Analog Input (PWR) - Spannungserkennung für automatische Steuerung
 * - P3.3 (Pin 4): Short Circuit Detection (SC) - Kurzschlusserkennung (Low = Kurzschluss)
 * - P5.4 (Pin 5): Reset
 * - P5.5 (Pin 6): Relay Output - Steuert das Relais (High = Ein)
 * 
 * UART-Protokoll (9600 Baud, 8N1):
 * Befehle (von Klipper):
 *   "on\n"   - Schaltet Relais ein (wenn kein Kurzschluss)
 *   "off\n"  - Schaltet Relais aus
 *   "g\n"    - Gibt aktuellen Status zurück
 * 
 * Antworten (an Klipper):
 *   "ok\n"   - Befehl erfolgreich ausgeführt
 *   "ON\n"   - Relais ist eingeschaltet
 *   "OFF\n"  - Relais ist ausgeschaltet
 *   "ERR\n"  - Kurzschluss erkannt, Relais abgeschaltet
 *   "OK\n"   - Kurzschluss behoben
 * 
 * Features:
 * - Klipper Power PSU Plugin Kompatibilität
 * - Automatische Kurzschlusserkennung mit Sicherheitsabschaltung
 * - Spannungsüberwachung für automatische Relais-Steuerung
 * - UART-Kommunikation für externe Steuerung
 */

#include "STC15Fxx.h"

// Hardware-Konfiguration
#define FOSC 11059200L      // Oszillatorfrequenz: 11.0592 MHz (Standard für UART)
#define BAUD 9600           // UART Baudrate: 9600 Baud (Klipper Standard)

// Pin-Definitionen
#define RELAY P55           // P5.5 (Pin 6): Relais-Ausgang
#define PWR   P32           // P3.2 (Pin 3): Spannungseingang (Analog Input)
#define SC    P33           // P3.3 (Pin 4): Kurzschlusserkennung (Low-Aktiv)

/*
 * UART Initialisierung
 * Konfiguriert UART im Modus 1 (8-Bit variable Baudrate)
 * Timer 1 im Auto-Reload Modus für Baudrate-Generierung
 */
void UART_Init(void) {
    SCON = 0x50;            // Modus 1: 8-Bit UART, REN=1 (Empfang aktiviert)
    TMOD = 0x20;            // Timer 1 Modus 2: 8-Bit Auto-Reload
    TH1 = TL1 = (unsigned char)(256 - (FOSC / 32 / BAUD)); // Baudrate berechnen
    TR1 = 1;                // Timer 1 starten
    EA = 1;                 // Globale Interrupts aktivieren
    ES = 1;                 // UART Interrupt aktivieren
}

/*
 * Sendet ein einzelnes Zeichen über UART
 * Wartet auf Sende-Abschluss (TI Flag)
 */
void send(char c) {
    SBUF = c;               // Zeichen in Sendepuffer schreiben
    while (!TI);            // Warten bis Übertragung abgeschlossen
    TI = 0;                 // Transmit Interrupt Flag zurücksetzen
}

/*
 * Sendet einen Null-terminierten String über UART
 * Iteriert durch String bis '\0' erreicht wird
 */
void sendStr(char *s) {
    while (*s) send(*s++);  // Jedes Zeichen einzeln senden
}

/*
 * Software-Delay Funktion
 * Erzeugt Verzögerung für Entprellung der Eingänge
 * Bei 11.0592 MHz: ~1ms pro Aufruf mit ms=1
 */
void delay(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 123; j++);  // Kalibriert für ~1ms bei 11MHz
}


/*
 * Hauptprogramm
 * Initialisiert Hardware und führt Hauptschleife aus
 */
void main(void) {
    // UART für Klipper-Kommunikation initialisieren
    UART_Init();
    
    // Port-Konfiguration
    P3M0 = P3M1 = 0;        // Port 3: Standard Bidirektional (P3.2, P3.3 als Eingang)
    P5M0 = 0x20;            // Port 5: P5.5 als Push-Pull Ausgang (Relais)
    P5M1 = 0;               // P5M0=1, P5M1=0 für P5.5 = Push-Pull Output
    RELAY = 0;              // Relais initial ausgeschaltet (Sicherheit)
    
    // Variablen
    char b[8];              // UART Empfangspuffer (max 7 Zeichen + '\0')
    char i = 0;             // Index für Empfangspuffer
    char sc_det = 0;        // Kurzschluss-Status: 0=OK, 1=Kurzschluss erkannt
    char pwr_st = PWR;      // Aktueller Spannungs-Status (Power State)
    char lpwr = PWR;        // Letzter Spannungs-Status (für Änderungserkennung)

    // Hauptschleife - läuft endlos
    while (1) {
        
        /*
         * Kurzschlusserkennung
         * P3.3 (SC) ist Low-Aktiv: Low = Kurzschluss, High = OK
         * Bei Kurzschluss: Relais sofort abschalten und "ERR" senden
         * Bei Behebung: "OK" senden
         */
        if (!SC && !sc_det) {           // Kurzschluss neu erkannt
            sc_det = 1;                 // Kurzschluss-Flag setzen
            RELAY = 0;                  // Relais sofort ausschalten (Sicherheit!)
            sendStr("ERROR: SHORT CIRCUIT DETECTED - RELAY OFF\n"); // Detaillierte Fehlermeldung
        }
        else if (SC && sc_det) {        // Kurzschluss behoben
            sc_det = 0;                 // Kurzschluss-Flag zurücksetzen
            sendStr("INFO: SHORT CIRCUIT CLEARED\n"); // Bestätigung mit Details
        }
        
        /*
         * Spannungsüberwachung (Analog Input auf P3.2)
         * Überwacht PWR-Pin auf Zustandsänderung
         * Mit Entprellung (50ms Delay) gegen Störungen
         * Relais wird automatisch gesteuert (wenn kein Kurzschluss)
         */
        if (PWR != lpwr) {              // Spannungsänderung erkannt
            delay(50);                  // 50ms warten zur Entprellung
            if (PWR != lpwr) {          // Änderung bestätigt (kein Glitch)
                lpwr = pwr_st = PWR;    // Neuen Zustand übernehmen
                RELAY = (pwr_st && !sc_det); // Relais ein, wenn PWR=High UND kein Kurzschluss
                sendStr(RELAY ? "ON\n" : "OFF\n"); // Status an Klipper melden
            }
        }
        
        /*
         * UART Befehlsverarbeitung
         * Empfängt Befehle von Klipper über UART
         * Befehle müssen mit '\n' enden oder 7 Zeichen erreichen
         * Verarbeitet: "on", "off", "g" (get status)
         */
        if (RI) {                       // UART Empfangs-Interrupt Flag gesetzt
            b[i++] = SBUF;              // Empfangenes Byte in Puffer speichern
            RI = 0;                     // Receive Interrupt Flag zurücksetzen
            
            // Befehl komplett? (Newline oder Puffer voll)
            if (b[i-1] == '\n' || i >= 7) {
                b[i] = 0;               // Null-Terminierung für String
                
                // Befehl: "on\n" - Relais einschalten
                if (b[0] == 'o' && b[1] == 'n' && !b[2]) {
                    RELAY = !sc_det;    // Nur einschalten wenn kein Kurzschluss
                    sendStr("ok\n");    // Bestätigung an Klipper
                }
                // Befehl: "off\n" - Relais ausschalten
                else if (b[0] == 'o' && b[1] == 'f' && b[2] == 'f' && !b[3]) {
                    RELAY = 0;          // Relais ausschalten
                    sendStr("ok\n");    // Bestätigung an Klipper
                }
                // Befehl: "g\n" - Get Status (aktuellen Zustand abfragen)
                else if (b[0] == 'g' && !b[1]) {
                    sendStr(RELAY ? "ON\n" : "OFF\n"); // Aktuellen Status senden
                }
                // Unbekannter Befehl wird ignoriert
                
                i = 0;                  // Puffer-Index zurücksetzen für nächsten Befehl
            }
        }
    }
}

