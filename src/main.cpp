#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Pin del LED integrado en ESP32-S3-DevKitM-1
#define LED_PIN 48

// Handle del semáforo binario
SemaphoreHandle_t xSemaphore = NULL;

// Tarea 1: Enciende el LED y libera el semáforo
void taskEncender(void *pvParameters) {
    while (true) {
        // Espera a tomar el semáforo (bloqueante)
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED ENCENDIDO - Task 1");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo
            // Libera el semáforo para que la otra tarea pueda ejecutarse
            xSemaphoreGive(xSemaphore);
            vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño delay para evitar starvation
        }
    }
}

// Tarea 2: Apaga el LED y libera el semáforo
void taskApagar(void *pvParameters) {
    while (true) {
        // Espera a tomar el semáforo (bloqueante)
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED APAGADO  - Task 2");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo
            // Libera el semáforo para que la otra tarea pueda ejecutarse
            xSemaphoreGive(xSemaphore);
            vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño delay para evitar starvation
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // Crear el semáforo binario
    xSemaphore = xSemaphoreCreateBinary();

    if (xSemaphore == NULL) {
        Serial.println("Error: No se pudo crear el semáforo");
        while (true); // Detener ejecución si falla
    }

    // Dar el semáforo inicialmente para que taskEncender arranque primero
    xSemaphoreGive(xSemaphore);

    // Crear tarea 1: Encender LED (Core 0, prioridad 1)
    xTaskCreatePinnedToCore(
        taskEncender,    // Función de la tarea
        "TaskEncender",  // Nombre
        2048,            // Stack size (bytes)
        NULL,            // Parámetros
        1,               // Prioridad
        NULL,            // Handle (no necesario)
        0                // Core 0
    );

    // Crear tarea 2: Apagar LED (Core 1, prioridad 1)
    xTaskCreatePinnedToCore(
        taskApagar,      // Función de la tarea
        "TaskApagar",    // Nombre
        2048,            // Stack size (bytes)
        NULL,            // Parámetros
        1,               // Prioridad
        NULL,            // Handle (no necesario)
        1                // Core 1
    );

    Serial.println("Sistema iniciado - FreeRTOS Semáforo Binario");
}

void loop() {
    // Con FreeRTOS no se usa loop(), las tareas gestionan todo
    vTaskDelay(pdMS_TO_TICKS(1000));
}