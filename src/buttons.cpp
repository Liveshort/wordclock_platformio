#include <Arduino.h>

#include "buttons.h"
#include "main.h"

#define DEBOUNCE_DELAY 300  // milliseconds

byte BUTTON_PINS[BUTTON_COUNT] = {18, 19, 21, 22, 23};

TaskHandle_t button_task_handle;

void IRAM_ATTR any_button_isr() {
    if (millis() - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(button_task_handle, &xHigherPriorityTaskWoken);
        TIMERS[INTERRUPT_DEBOUNCE] = millis();
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// This task handles the button presses. It is notified by the button ISRs and then checks which button was pressed and
// sets the corresponding flag. Testing showed that the power line is noisy, which can result in multiple buttons firing
// when only one is pressed, or a button firing if some other device is connected to the same power line. To mitigate
// this, we digitalread all buttons 50 times after an interrupt has fired for any of the buttons and choose a winner
// according to which button was read the most. A minimum amount of 10 reads is required to register a button press,
// which should filter out most noise while still being responsive. The task is run on core 0, so it does not interfere
// with the main loop and animations running on core 1.
void button_task(void* parameters) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        unsigned long start_time = millis();

        int counts[BUTTON_COUNT] = {0};
        for (int i = 0; i < 50; ++i) {
            for (int j = 0; j < BUTTON_COUNT; ++j) {
                if (digitalRead(BUTTON_PINS[j]) == LOW) {
                    counts[j]++;
                }
            }
        }

        int max_count = 0;
        int max_index = -1;
        for (int j = 0; j < BUTTON_COUNT; ++j) {
            if (counts[j] > max_count) {
                max_count = counts[j];
                max_index = j;
            }
        }

        if (max_count > 5) {
            xQueueSend(BUTTON_QUEUE, &max_index, 0);
        }

        LOGGER.println("Button counts: " + String(counts[0]) + ", " + String(counts[1]) + ", " + String(counts[2]) +
                       ", " + String(counts[3]) + ", " + String(counts[4]) +
                       "\n    Time taken: " + String(millis() - start_time) + "ms");
    }
}

void initialize_buttons() {
    xTaskCreatePinnedToCore(button_task, "Button Task", 2048, NULL, 1, &button_task_handle, 0);

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[i]), any_button_isr, FALLING);
    }
}
