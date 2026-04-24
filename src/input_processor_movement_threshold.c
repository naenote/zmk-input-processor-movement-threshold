#define DT_DRV_COMPAT zmk_input_processor_movement_threshold

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/util.h>
#include <zmk/input_processor.h>

static int movement_threshold_handle_event(const struct device *dev,
                                            struct input_event *event,
                                            uint32_t param1, uint32_t param2,
                                            struct zmk_input_processor_state *state) {
    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }
    if (event->code != INPUT_REL_X && event->code != INPUT_REL_Y) {
        return ZMK_INPUT_PROC_CONTINUE;
    }
    int32_t val = event->value;
    if (val < 0) {
        val = -val;
    }
    if (val <= (int32_t)param1) {
        event->value = 0;
    }
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api movement_threshold_driver_api = {
    .handle_event = movement_threshold_handle_event,
};

#define MOVEMENT_THRESHOLD_INST(n) \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, \
        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
        &movement_threshold_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MOVEMENT_THRESHOLD_INST)
