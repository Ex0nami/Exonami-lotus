#ifndef BUZZER_SERVICE_H_
#define BUZZER_SERVICE_H_

#include "esp_err.h"

namespace buzzer_service {

/**
 * Konfiguriert den Buzzer-Pin für PWM-Ausgabe (LEDC).
 */
esp_err_t Init();

/**
 * Schaltet das Klickfeedback ein oder aus.
 *
 * Der GameSettings-Wert wird darüber am Klickpunkt selbst
 * durchgesetzt, unabhängig davon, von wo Click() aufgerufen wird.
 */
void SetEnabled(bool enabled);

/**
 * Löst einen kurzen, leisen Klickton aus.
 *
 * Kein Effekt, solange SetEnabled(false) gesetzt ist.
 *
 * Nicht blockierend - der Ton läuft in einer eigenen Task ab.
 * Ein Aufruf während ein Klick noch läuft wird ignoriert, damit
 * sich schnell aufeinanderfolgende Taps nicht zu einem
 * Dauerton aufsummieren.
 */
void Click();

}  // namespace buzzer_service

#endif  // BUZZER_SERVICE_H_
