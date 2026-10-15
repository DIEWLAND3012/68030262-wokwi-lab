#include <inttypes.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define BUTTON_PIN GPIO_NUM_4

static const char *TAG = "RTOS_ISR_PROJECT";
static QueueHandle_t gpio_evt_queue;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    const uint32_t gpio_num = (uint32_t)(uintptr_t)arg;
    BaseType_t higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, &higher_priority_task_woken);
    if (higher_priority_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void button_worker_task(void *arg)
{
    uint32_t io_num;
    uint32_t press_count = 0;
    while (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY) == pdTRUE) {
        ++press_count;
        ESP_LOGI(TAG, "Button on GPIO %" PRIu32 " pressed %" PRIu32 " times.",
                 io_num, press_count);
    }
}

void app_main(void)
{
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    if (gpio_evt_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create GPIO event queue");
        return;
    }
    if (xTaskCreate(button_worker_task, "button_worker", 2048, NULL,
                    10, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button worker task");
        return;
    }

    const gpio_config_t io_config = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_config));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(
        BUTTON_PIN, gpio_isr_handler, (void *)(uintptr_t)BUTTON_PIN));

    ESP_LOGI(TAG, "Press the button to send GPIO events to the worker task.");
}
