#define DT_DRV_COMPAT zmk_input_processor_movement_sustain

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <drivers/input_processor.h>

/*
 * Passes X/Y movement events only once the movement has been going on for at
 * least `duration-ms`. A keystroke shock produces a single short burst of
 * reports and is filtered out, while deliberate pointer movement lasts long
 * enough to get through.
 *
 * A burst is considered finished — and the timer restarted — when no qualifying
 * event arrives for `gap-ms`.
 *
 * Because sub-duration events are stopped (which also stops the HID update),
 * this processor belongs in an AML-only listener, never in the listener that
 * drives the pointer.
 */

struct movement_sustain_data {
    /* uptime (ms) at which the current burst started */
    int64_t burst_start;
    /* uptime (ms) of the most recent movement event seen */
    int64_t last_event;
    /* the current burst has already reached duration-ms */
    bool passing;
};

static int movement_sustain_handle_event(const struct device *dev,
                                          struct input_event *event,
                                          uint32_t param1, uint32_t param2,
                                          struct zmk_input_processor_state *state) {
    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }
    if (event->code != INPUT_REL_X && event->code != INPUT_REL_Y) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    struct movement_sustain_data *data = (struct movement_sustain_data *)dev->data;
    const int64_t now = k_uptime_get();
    const int64_t duration_ms = (int64_t)param1;
    const int64_t gap_ms = (int64_t)param2;

    /* Idle for longer than gap-ms (which includes the very first event after
     * boot, since last_event starts at 0): this is a new burst. */
    if (now - data->last_event > gap_ms) {
        data->burst_start = now;
        data->passing = false;
    }
    data->last_event = now;

    if (!data->passing && now - data->burst_start >= duration_ms) {
        data->passing = true;
    }

    return data->passing ? ZMK_INPUT_PROC_CONTINUE : ZMK_INPUT_PROC_STOP;
}

static const struct zmk_input_processor_driver_api movement_sustain_driver_api = {
    .handle_event = movement_sustain_handle_event,
};

#define MOVEMENT_SUSTAIN_INST(n) \
    static struct movement_sustain_data movement_sustain_data_##n = {}; \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &movement_sustain_data_##n, NULL, \
        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
        &movement_sustain_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MOVEMENT_SUSTAIN_INST)
