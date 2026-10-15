#include <stdio.h>
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "STORAGE_PROJECT";

void app_main(void)
{
    const esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    esp_err_t err = esp_vfs_spiffs_register(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SPIFFS: %s", esp_err_to_name(err));
        return;
    }

    FILE *file = fopen("/spiffs/log.txt", "w");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open log.txt for writing");
        esp_vfs_spiffs_unregister(config.partition_label);
        return;
    }
    if (fprintf(file, "ESP32 SPIFFS storage is working.\n") < 0) {
        ESP_LOGE(TAG, "Failed to write log.txt");
        fclose(file);
        esp_vfs_spiffs_unregister(config.partition_label);
        return;
    }
    if (fclose(file) != 0) {
        ESP_LOGE(TAG, "Failed to close log.txt after writing");
        esp_vfs_spiffs_unregister(config.partition_label);
        return;
    }

    file = fopen("/spiffs/log.txt", "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open log.txt for reading");
        esp_vfs_spiffs_unregister(config.partition_label);
        return;
    }
    char line[96];
    if (fgets(line, sizeof(line), file) == NULL) {
        ESP_LOGE(TAG, "Failed to read log.txt");
        fclose(file);
        esp_vfs_spiffs_unregister(config.partition_label);
        return;
    }
    fclose(file);
    ESP_LOGI(TAG, "Read back from SPIFFS: %s", line);

    ESP_ERROR_CHECK(esp_vfs_spiffs_unregister(config.partition_label));
    ESP_LOGI(TAG, "SPIFFS filesystem unmounted.");
}
