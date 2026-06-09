#include "common.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "control_msg.h"

static void test_serialize_inject_keycode(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        .inject_keycode = {
            .action = AKEY_EVENT_ACTION_UP,
            .keycode = AKEYCODE_ENTER,
            .repeat = 5,
            .metastate = AMETA_SHIFT_ON | AMETA_SHIFT_LEFT_ON,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 14);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        0x01, // AKEY_EVENT_ACTION_UP
        0x00, 0x00, 0x00, 0x42, // AKEYCODE_ENTER
        0x00, 0x00, 0x00, 0X05, // repeat
        0x00, 0x00, 0x00, 0x41, // AMETA_SHIFT_ON | AMETA_SHIFT_LEFT_ON
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_text(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TEXT,
        .inject_text = {
            .text = "hello, world!",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 18);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TEXT,
        0x00, 0x00, 0x00, 0x0d, // text length
        'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', // text
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_text_long(void) {
    struct sc_control_msg msg;
    msg.type = SC_CONTROL_MSG_TYPE_INJECT_TEXT;
    char text[SC_CONTROL_MSG_INJECT_TEXT_MAX_LENGTH + 1];
    memset(text, 'a', SC_CONTROL_MSG_INJECT_TEXT_MAX_LENGTH);
    text[SC_CONTROL_MSG_INJECT_TEXT_MAX_LENGTH] = '\0';
    msg.inject_text.text = text;

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5 + SC_CONTROL_MSG_INJECT_TEXT_MAX_LENGTH);

    uint8_t expected[5 + SC_CONTROL_MSG_INJECT_TEXT_MAX_LENGTH];
    expected[0] = SC_CONTROL_MSG_TYPE_INJECT_TEXT;
    expected[1] = 0x00;
    expected[2] = 0x00;
    expected[3] = 0x01;
    expected[4] = 0x2c; // text length (32 bits)
    memset(&expected[5], 'a', SC_CONTROL_MSG_INJECT_TEXT_MAX_LENGTH);

    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_touch_event(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        .inject_touch_event = {
            .action = AMOTION_EVENT_ACTION_DOWN,
            .pointer_id = UINT64_C(0x1234567887654321),
            .position = {
                .point = {
                    .x = 100,
                    .y = 200,
                },
                .screen_size = {
                    .width = 1080,
                    .height = 1920,
                },
            },
            .pressure = 1.0f,
            .action_button = AMOTION_EVENT_BUTTON_PRIMARY,
            .buttons = AMOTION_EVENT_BUTTON_PRIMARY,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 32);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        0x00, // AKEY_EVENT_ACTION_DOWN
        0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21, // pointer id
        0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xc8, // 100 200
        0x04, 0x38, 0x07, 0x80, // 1080 1920
        0xff, 0xff, // pressure
        0x00, 0x00, 0x00, 0x01, // AMOTION_EVENT_BUTTON_PRIMARY (action button)
        0x00, 0x00, 0x00, 0x01, // AMOTION_EVENT_BUTTON_PRIMARY (buttons)
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_scroll_event(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        .inject_scroll_event = {
            .position = {
                .point = {
                    .x = 260,
                    .y = 1026,
                },
                .screen_size = {
                    .width = 1080,
                    .height = 1920,
                },
            },
            .hscroll = 16,
            .vscroll = -16,
            .buttons = 1,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 21);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x04, 0x02, // 260 1026
        0x04, 0x38, 0x07, 0x80, // 1080 1920
        0x7F, 0xFF, // 16 (float encoded as i16 in the range [-16, 16])
        0x80, 0x00, // -16 (float encoded as i16 in the range [-16, 16])
        0x00, 0x00, 0x00, 0x01, // 1
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_back_or_screen_on(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
        .back_or_screen_on = {
            .action = AKEY_EVENT_ACTION_UP,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
        0x01, // AKEY_EVENT_ACTION_UP
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_expand_notification_panel(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_expand_settings_panel(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_collapse_panels(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_COLLAPSE_PANELS,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_COLLAPSE_PANELS,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_get_clipboard(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        .get_clipboard = {
            .copy_key = SC_COPY_KEY_COPY,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        SC_COPY_KEY_COPY,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_clipboard(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        .set_clipboard = {
            .sequence = UINT64_C(0x0102030405060708),
            .paste = true,
            .text = "hello, world!",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 27);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // sequence
        1, // paste
        0x00, 0x00, 0x00, 0x0d, // text length
        'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', // text
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_clipboard_long(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        .set_clipboard = {
            .sequence = UINT64_C(0x0102030405060708),
            .paste = true,
            .text = NULL,
        },
    };

    char text[SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH + 1];
    memset(text, 'a', SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH);
    text[SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH] = '\0';
    msg.set_clipboard.text = text;

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == SC_CONTROL_MSG_MAX_SIZE);

    uint8_t expected[SC_CONTROL_MSG_MAX_SIZE] = {
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // sequence
        1, // paste
        // text length
        SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH >> 24,
        (SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH >> 16) & 0xff,
        (SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH >> 8) & 0xff,
        SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH & 0xff,
    };
    memset(expected + 14, 'a', SC_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH);

    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_display_power(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER,
        .set_display_power = {
            .on = true,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER,
        0x01, // true
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_rotate_device(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_ROTATE_DEVICE,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_ROTATE_DEVICE,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_create(void) {
    const uint8_t report_desc[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_CREATE,
        .uhid_create = {
            .id = 42,
            .vendor_id = 0x1234,
            .product_id = 0x5678,
            .name = "ABC",
            .report_desc_size = sizeof(report_desc),
            .report_desc = report_desc,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 24);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_CREATE,
        0, 42, // id
        0x12, 0x34, // vendor id
        0x56, 0x78, // product id
        3, // name size
        65, 66, 67, // "ABC"
        0, 11, // report desc size
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_input(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_INPUT,
        .uhid_input = {
            .id = 42,
            .size = 5,
            .data = {1, 2, 3, 4, 5},
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 10);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_INPUT,
        0, 42, // id
        0, 5, // size
        1, 2, 3, 4, 5,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_destroy(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_DESTROY,
        .uhid_destroy = {
            .id = 42,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 3);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_DESTROY,
        0, 42, // id
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_open_hard_keyboard(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_OPEN_HARD_KEYBOARD_SETTINGS,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_OPEN_HARD_KEYBOARD_SETTINGS,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_start_app(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_START_APP,
        .start_app = {
            .name = "firefox",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 9);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_START_APP,
        7, // length
        'f', 'i', 'r', 'e', 'f', 'o', 'x', // app name
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_reset_video(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_RESET_VIDEO,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_RESET_VIDEO,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_camera_set_torch(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_CAMERA_SET_TORCH,
        .camera_set_torch = {
            .on = true,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_CAMERA_SET_TORCH,
        0x01, // true
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_camera_zoom_in(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_CAMERA_ZOOM_IN,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_CAMERA_ZOOM_IN,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_camera_zoom_out(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_CAMERA_ZOOM_OUT,
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 1);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_CAMERA_ZOOM_OUT,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_resize_display(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
        .resize_display = {
            .width = 1920,
            .height = 1080,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
        1920 >> 8, 1920 & 0xff,
        1080 >> 8, 1080 & 0xff,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

// ---- Boundary and edge-case tests ----

static void test_serialize_inject_keycode_all_zeros(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        .inject_keycode = {
            .action = AKEY_EVENT_ACTION_DOWN,
            .keycode = AKEYCODE_UNKNOWN,
            .repeat = 0,
            .metastate = AMETA_NONE,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 14);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        0x00, // AKEY_EVENT_ACTION_DOWN
        0x00, 0x00, 0x00, 0x00, // AKEYCODE_UNKNOWN
        0x00, 0x00, 0x00, 0x00, // repeat
        0x00, 0x00, 0x00, 0x00, // AMETA_NONE
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_keycode_max_values(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        .inject_keycode = {
            .action = AKEY_EVENT_ACTION_MULTIPLE,
            .keycode = (enum android_keycode) 0xFFFFFFFF,
            .repeat = UINT32_MAX,
            .metastate = (enum android_metastate) 0xFFFFFFFF,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 14);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        0x02, // AKEY_EVENT_ACTION_MULTIPLE
        0xFF, 0xFF, 0xFF, 0xFF, // keycode
        0xFF, 0xFF, 0xFF, 0xFF, // repeat
        0xFF, 0xFF, 0xFF, 0xFF, // metastate
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_text_empty(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TEXT,
        .inject_text = {
            .text = "",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TEXT,
        0x00, 0x00, 0x00, 0x00, // text length 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_touch_event_zero_pressure(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        .inject_touch_event = {
            .action = AMOTION_EVENT_ACTION_UP,
            .pointer_id = 0,
            .position = {
                .point = { .x = 0, .y = 0 },
                .screen_size = { .width = 0, .height = 0 },
            },
            .pressure = 0.0f,
            .action_button = 0,
            .buttons = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 32);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        0x01, // AMOTION_EVENT_ACTION_UP
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pointer_id 0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // point (0,0)
        0x00, 0x00, 0x00, 0x00, // screen (0,0)
        0x00, 0x00, // pressure 0.0
        0x00, 0x00, 0x00, 0x00, // action_button
        0x00, 0x00, 0x00, 0x00, // buttons
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_touch_event_half_pressure(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        .inject_touch_event = {
            .action = AMOTION_EVENT_ACTION_MOVE,
            .pointer_id = 1,
            .position = {
                .point = { .x = 100, .y = 200 },
                .screen_size = { .width = 1080, .height = 1920 },
            },
            .pressure = 0.5f,
            .action_button = 0,
            .buttons = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 32);

    // 0.5f * 0x10000 = 0x8000
    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        0x02, // AMOTION_EVENT_ACTION_MOVE
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // pointer_id 1
        0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xc8, // point (100, 200)
        0x04, 0x38, 0x07, 0x80, // screen (1080, 1920)
        0x80, 0x00, // pressure 0.5
        0x00, 0x00, 0x00, 0x00, // action_button
        0x00, 0x00, 0x00, 0x00, // buttons
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_touch_event_mouse_pointer_id(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        .inject_touch_event = {
            .action = AMOTION_EVENT_ACTION_DOWN,
            .pointer_id = SC_POINTER_ID_MOUSE,
            .position = {
                .point = { .x = 500, .y = 500 },
                .screen_size = { .width = 1080, .height = 1920 },
            },
            .pressure = 1.0f,
            .action_button = AMOTION_EVENT_BUTTON_SECONDARY,
            .buttons = AMOTION_EVENT_BUTTON_SECONDARY,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 32);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        0x00, // AMOTION_EVENT_ACTION_DOWN
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // SC_POINTER_ID_MOUSE
        0x00, 0x00, 0x01, 0xF4, 0x00, 0x00, 0x01, 0xF4, // point (500, 500)
        0x04, 0x38, 0x07, 0x80, // screen (1080, 1920)
        0xFF, 0xFF, // pressure 1.0
        0x00, 0x00, 0x00, 0x02, // AMOTION_EVENT_BUTTON_SECONDARY
        0x00, 0x00, 0x00, 0x02, // AMOTION_EVENT_BUTTON_SECONDARY
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_touch_event_virtual_finger(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        .inject_touch_event = {
            .action = AMOTION_EVENT_ACTION_DOWN,
            .pointer_id = SC_POINTER_ID_VIRTUAL_FINGER,
            .position = {
                .point = { .x = 0, .y = 0 },
                .screen_size = { .width = 1920, .height = 1080 },
            },
            .pressure = 1.0f,
            .action_button = 0,
            .buttons = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 32);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        0x00, // AMOTION_EVENT_ACTION_DOWN
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, // SC_POINTER_ID_VIRTUAL_FINGER
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // point (0, 0)
        0x07, 0x80, 0x04, 0x38, // screen (1920, 1080)
        0xFF, 0xFF, // pressure 1.0
        0x00, 0x00, 0x00, 0x00, // action_button
        0x00, 0x00, 0x00, 0x00, // buttons
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_scroll_event_zero(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        .inject_scroll_event = {
            .position = {
                .point = { .x = 0, .y = 0 },
                .screen_size = { .width = 1080, .height = 1920 },
            },
            .hscroll = 0,
            .vscroll = 0,
            .buttons = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 21);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // point (0, 0)
        0x04, 0x38, 0x07, 0x80, // screen (1080, 1920)
        0x00, 0x00, // hscroll 0
        0x00, 0x00, // vscroll 0
        0x00, 0x00, 0x00, 0x00, // buttons
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_scroll_event_fraction(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        .inject_scroll_event = {
            .position = {
                .point = { .x = 100, .y = 200 },
                .screen_size = { .width = 1080, .height = 1920 },
            },
            .hscroll = 8,
            .vscroll = -8,
            .buttons = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 21);

    // hscroll: 8/16=0.5 -> sc_float_to_i16fp(0.5) = 0.5*2^15 = 16384 = 0x4000
    // vscroll: -8/16=-0.5 -> sc_float_to_i16fp(-0.5) = -16384 = 0xC000
    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xc8, // point (100, 200)
        0x04, 0x38, 0x07, 0x80, // screen (1080, 1920)
        0x40, 0x00, // hscroll 8 (normalized 0.5)
        0xC0, 0x00, // vscroll -8 (normalized -0.5)
        0x00, 0x00, 0x00, 0x00, // buttons
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_inject_scroll_event_clamped(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        .inject_scroll_event = {
            .position = {
                .point = { .x = 0, .y = 0 },
                .screen_size = { .width = 1080, .height = 1920 },
            },
            .hscroll = 32,  // > 16, will be clamped to 1.0
            .vscroll = -32, // < -16, will be clamped to -1.0
            .buttons = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 21);

    // hscroll: 32/16=2 -> CLAMP(2,-1,1)=1 -> sc_float_to_i16fp(1) = 0x7FFF
    // vscroll: -32/16=-2 -> CLAMP(-2,-1,1)=-1 -> sc_float_to_i16fp(-1) = 0x8000
    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // point (0, 0)
        0x04, 0x38, 0x07, 0x80, // screen (1080, 1920)
        0x7F, 0xFF, // hscroll clamped to max
        0x80, 0x00, // vscroll clamped to min
        0x00, 0x00, 0x00, 0x00, // buttons
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_back_or_screen_on_down(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
        .back_or_screen_on = {
            .action = AKEY_EVENT_ACTION_DOWN,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
        0x00, // AKEY_EVENT_ACTION_DOWN
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_get_clipboard_none(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        .get_clipboard = {
            .copy_key = SC_COPY_KEY_NONE,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        SC_COPY_KEY_NONE,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_get_clipboard_cut(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        .get_clipboard = {
            .copy_key = SC_COPY_KEY_CUT,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        SC_COPY_KEY_CUT,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_clipboard_empty(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        .set_clipboard = {
            .sequence = 0,
            .paste = false,
            .text = "",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 14);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sequence 0
        0x00, // paste false
        0x00, 0x00, 0x00, 0x00, // text length 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_clipboard_no_paste(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        .set_clipboard = {
            .sequence = UINT64_C(1),
            .paste = false,
            .text = "abc",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 17);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // sequence 1
        0x00, // paste false
        0x00, 0x00, 0x00, 0x03, // text length 3
        'a', 'b', 'c',
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_clipboard_max_sequence(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        .set_clipboard = {
            .sequence = UINT64_MAX,
            .paste = true,
            .text = "x",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 15);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // sequence max
        0x01, // paste true
        0x00, 0x00, 0x00, 0x01, // text length 1
        'x',
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_set_display_power_off(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER,
        .set_display_power = {
            .on = false,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER,
        0x00, // false
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_create_empty_name(void) {
    const uint8_t report_desc[] = {0xAA};
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_CREATE,
        .uhid_create = {
            .id = 1,
            .vendor_id = 0,
            .product_id = 0,
            .name = NULL,
            .report_desc_size = sizeof(report_desc),
            .report_desc = report_desc,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    // 1(type) + 2(id) + 2(vendor) + 2(product) + 1(name_len=0) + 2(rdesc_sz) + 1(rdesc) = 11
    assert(size == 11);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_CREATE,
        0x00, 0x01, // id 1
        0x00, 0x00, // vendor 0
        0x00, 0x00, // product 0
        0x00,       // name length 0
        0x00, 0x01, // report_desc_size 1
        0xAA,       // report_desc
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_create_empty_report_desc(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_CREATE,
        .uhid_create = {
            .id = 1,
            .vendor_id = 0,
            .product_id = 0,
            .name = "X",
            .report_desc_size = 0,
            .report_desc = NULL,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    // 1(type) + 2(id) + 2(vendor) + 2(product) + 1(name_len) + 1(name) + 2(rdesc_sz=0) = 11
    assert(size == 11);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_CREATE,
        0x00, 0x01, // id 1
        0x00, 0x00, // vendor 0
        0x00, 0x00, // product 0
        0x01, 'X',  // name length 1, "X"
        0x00, 0x00, // report_desc_size 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_create_max_id(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_CREATE,
        .uhid_create = {
            .id = UINT16_MAX,
            .vendor_id = UINT16_MAX,
            .product_id = UINT16_MAX,
            .name = "Z",
            .report_desc_size = 0,
            .report_desc = NULL,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 11);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_CREATE,
        0xFF, 0xFF, // id max
        0xFF, 0xFF, // vendor max
        0xFF, 0xFF, // product max
        0x01, 'Z',  // name
        0x00, 0x00, // report_desc_size 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_input_max_size(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_INPUT,
        .uhid_input = {
            .id = 1,
            .size = SC_HID_MAX_SIZE,
            .data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5 + SC_HID_MAX_SIZE);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_INPUT,
        0x00, 0x01, // id 1
        0x00, 0x0F, // size 15
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_uhid_input_zero_size(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_UHID_INPUT,
        .uhid_input = {
            .id = 1,
            .size = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_UHID_INPUT,
        0x00, 0x01, // id 1
        0x00, 0x00, // size 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_start_app_empty(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_START_APP,
        .start_app = {
            .name = "",
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_START_APP,
        0x00, // name length 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_camera_set_torch_off(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_CAMERA_SET_TORCH,
        .camera_set_torch = {
            .on = false,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 2);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_CAMERA_SET_TORCH,
        0x00, // false
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_resize_display_zero(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
        .resize_display = {
            .width = 0,
            .height = 0,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
        0x00, 0x00, // width 0
        0x00, 0x00, // height 0
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_serialize_resize_display_max(void) {
    struct sc_control_msg msg = {
        .type = SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
        .resize_display = {
            .width = UINT16_MAX,
            .height = UINT16_MAX,
        },
    };

    uint8_t buf[SC_CONTROL_MSG_MAX_SIZE];
    size_t size = sc_control_msg_serialize(&msg, buf);
    assert(size == 5);

    const uint8_t expected[] = {
        SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
        0xFF, 0xFF, // width max
        0xFF, 0xFF, // height max
    };
    assert(!memcmp(buf, expected, sizeof(expected)));
}

static void test_control_msg_is_droppable(void) {
    // UHID_CREATE and UHID_DESTROY must NOT be droppable
    struct sc_control_msg msg_uhid_create = {
        .type = SC_CONTROL_MSG_TYPE_UHID_CREATE,
    };
    assert(!sc_control_msg_is_droppable(&msg_uhid_create));

    struct sc_control_msg msg_uhid_destroy = {
        .type = SC_CONTROL_MSG_TYPE_UHID_DESTROY,
    };
    assert(!sc_control_msg_is_droppable(&msg_uhid_destroy));

    // All other types must be droppable
    enum sc_control_msg_type droppable_types[] = {
        SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        SC_CONTROL_MSG_TYPE_INJECT_TEXT,
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        SC_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
        SC_CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
        SC_CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL,
        SC_CONTROL_MSG_TYPE_COLLAPSE_PANELS,
        SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER,
        SC_CONTROL_MSG_TYPE_ROTATE_DEVICE,
        SC_CONTROL_MSG_TYPE_UHID_INPUT,
        SC_CONTROL_MSG_TYPE_OPEN_HARD_KEYBOARD_SETTINGS,
        SC_CONTROL_MSG_TYPE_START_APP,
        SC_CONTROL_MSG_TYPE_RESET_VIDEO,
        SC_CONTROL_MSG_TYPE_CAMERA_SET_TORCH,
        SC_CONTROL_MSG_TYPE_CAMERA_ZOOM_IN,
        SC_CONTROL_MSG_TYPE_CAMERA_ZOOM_OUT,
        SC_CONTROL_MSG_TYPE_RESIZE_DISPLAY,
    };

    for (size_t i = 0; i < ARRAY_LEN(droppable_types); ++i) {
        struct sc_control_msg msg = { .type = droppable_types[i] };
        assert(sc_control_msg_is_droppable(&msg));
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    test_serialize_inject_keycode();
    test_serialize_inject_text();
    test_serialize_inject_text_long();
    test_serialize_inject_touch_event();
    test_serialize_inject_scroll_event();
    test_serialize_back_or_screen_on();
    test_serialize_expand_notification_panel();
    test_serialize_expand_settings_panel();
    test_serialize_collapse_panels();
    test_serialize_get_clipboard();
    test_serialize_set_clipboard();
    test_serialize_set_clipboard_long();
    test_serialize_set_display_power();
    test_serialize_rotate_device();
    test_serialize_uhid_create();
    test_serialize_uhid_input();
    test_serialize_uhid_destroy();
    test_serialize_open_hard_keyboard();
    test_serialize_start_app();
    test_serialize_reset_video();
    test_serialize_camera_set_torch();
    test_serialize_camera_zoom_in();
    test_serialize_camera_zoom_out();
    test_serialize_resize_display();

    // Boundary and edge-case tests
    test_serialize_inject_keycode_all_zeros();
    test_serialize_inject_keycode_max_values();
    test_serialize_inject_text_empty();
    test_serialize_inject_touch_event_zero_pressure();
    test_serialize_inject_touch_event_half_pressure();
    test_serialize_inject_touch_event_mouse_pointer_id();
    test_serialize_inject_touch_event_virtual_finger();
    test_serialize_inject_scroll_event_zero();
    test_serialize_inject_scroll_event_fraction();
    test_serialize_inject_scroll_event_clamped();
    test_serialize_back_or_screen_on_down();
    test_serialize_get_clipboard_none();
    test_serialize_get_clipboard_cut();
    test_serialize_set_clipboard_empty();
    test_serialize_set_clipboard_no_paste();
    test_serialize_set_clipboard_max_sequence();
    test_serialize_set_display_power_off();
    test_serialize_uhid_create_empty_name();
    test_serialize_uhid_create_empty_report_desc();
    test_serialize_uhid_create_max_id();
    test_serialize_uhid_input_max_size();
    test_serialize_uhid_input_zero_size();
    test_serialize_start_app_empty();
    test_serialize_camera_set_torch_off();
    test_serialize_resize_display_zero();
    test_serialize_resize_display_max();
    test_control_msg_is_droppable();
    return 0;
}
