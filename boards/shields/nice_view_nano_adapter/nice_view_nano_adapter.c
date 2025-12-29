/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/activity.h>

#define INIT_PRIORITY 60

#define DPP DT_NODELABEL(reg_display_power)
static struct gpio_dt_spec const dpp_spec = GPIO_DT_SPEC_GET(DPP, enable_gpios);

static bool permanently_off = false;

static int switch_on() {
    if (permanently_off) {
        LOG_INF("Not switching on display: permanently off");
        return 0;
    }
    int ret = gpio_pin_set_dt(&dpp_spec, 1);
    if (ret == 0) {
        LOG_INF("Set display power pin");
    }
    else {
        LOG_ERR("Failed to set display power pin");
    }
    return ret;
}

static int switch_off() {
    int ret = gpio_pin_set_dt(&dpp_spec, 0);
    if (ret == 0) {
        LOG_INF("Cleared display power pin");
    }
    else {
        LOG_ERR("Failed to clear display power pin");
    }
    return ret;
}

static int nice_view_nano_adapter_event_listener(const zmk_event_t *eh) {
    const struct zmk_activity_state_changed *eva = as_zmk_activity_state_changed(eh);
    if (eva) {
        switch (eva->state) {
        case ZMK_ACTIVITY_ACTIVE:
            return switch_on();
        case ZMK_ACTIVITY_IDLE:
            return 0;
        case ZMK_ACTIVITY_SLEEP:
            permanently_off = true;
            return switch_off();
        default:
            break;
        }
    }
    return -ENOTSUP;
}

ZMK_LISTENER(nice_view_nano_adapter_listener, nice_view_nano_adapter_event_listener);
ZMK_SUBSCRIPTION(nice_view_nano_adapter_listener, zmk_activity_state_changed);
