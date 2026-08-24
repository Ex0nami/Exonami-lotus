#include "buzzer_service.h"

#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_log.h"
#include "followup_task_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sticky_board_config.h"

namespace buzzer_service {
namespace {

constexpr const char* kTag = "BuzzerService";

constexpr ledc_mode_t kSpeedMode = LEDC_LOW_SPEED_MODE;
constexpr ledc_timer_t kTimer = LEDC_TIMER_0;
constexpr ledc_channel_t kChannel = LEDC_CHANNEL_0;
constexpr ledc_timer_bit_t kDutyResolution = LEDC_TIMER_10_BIT;

/*
 * Kurzer, hochfrequenter Burst statt eines längeren Tons -
 * das klingt eher nach einem Klacken als nach einem Piepsen.
 * kClickFrequencyHz — höher klingt spitzer/heller, niedriger dumpfer.
 * kClickDurationMs — kürzer wirkt knackiger, länger eher wie ein kurzer Piepton.
 */
constexpr uint32_t kClickFrequencyHz = 200;
constexpr uint32_t kClickDurationMs = 13;

/*
 * 50% Duty für eine einfache Rechteckwelle.
 * (2^10) / 2 = 512.
 */
constexpr uint32_t kClickDuty = 512;

bool s_initialized = false;
bool s_enabled = true;
TaskHandle_t s_buzzerTask = nullptr;

void StopTone()
{
    ledc_set_duty(kSpeedMode, kChannel, 0);
    ledc_update_duty(kSpeedMode, kChannel);
}

void BuzzerTask(void* durationMsArg)
{
    const uint32_t durationMs =
        static_cast<uint32_t>(reinterpret_cast<intptr_t>(durationMsArg));

    ledc_set_duty(kSpeedMode, kChannel, kClickDuty);
    ledc_update_duty(kSpeedMode, kChannel);

    vTaskDelay(pdMS_TO_TICKS(durationMs));

    StopTone();

    s_buzzerTask = nullptr;
    vTaskDelete(nullptr);
}

}  // namespace

esp_err_t Init()
{
    if (s_initialized) {
        return ESP_OK;
    }

    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = kSpeedMode;
    timerConfig.duty_resolution = kDutyResolution;
    timerConfig.timer_num = kTimer;
    timerConfig.freq_hz = kClickFrequencyHz;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;

    ESP_RETURN_ON_ERROR(ledc_timer_config(&timerConfig), kTag,
                        "LEDC timer config failed");

    ledc_channel_config_t channelConfig = {};
    channelConfig.gpio_num = STICKY_BUZZER_PIN;
    channelConfig.speed_mode = kSpeedMode;
    channelConfig.channel = kChannel;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = kTimer;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;

    ESP_RETURN_ON_ERROR(ledc_channel_config(&channelConfig), kTag,
                        "LEDC channel config failed");

    s_initialized = true;

    ESP_LOGI(kTag, "Buzzer initialized on GPIO%d", STICKY_BUZZER_PIN);

    return ESP_OK;
}

void SetEnabled(const bool enabled)
{
    s_enabled = enabled;
}

void Click()
{
    if (!s_initialized || !s_enabled) {
        return;
    }

    /*
     * Läuft noch ein Klick, neuen Aufruf verwerfen statt
     * überlappende Töne zu erzeugen.
     */
    if (s_buzzerTask != nullptr) {
        return;
    }

    const BaseType_t created = xTaskCreatePinnedToCore(
        BuzzerTask,
        "buzzer",
        2048,
        reinterpret_cast<void*>(static_cast<intptr_t>(kClickDurationMs)),
        followup_task_config::kPriorityBuzzer,
        &s_buzzerTask,
        followup_task_config::kAppCore);

    if (created != pdPASS) {
        s_buzzerTask = nullptr;
        ESP_LOGW(kTag, "Could not start buzzer task");
    }
}

}  // namespace buzzer_service
