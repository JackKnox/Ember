#pragma once

#include "ember/core.h"

#include "ember/window/desktop.h"
#include "ember/window/input_codes.h"

/**
 * @brief Opaque input system handling all input devices.
 *
 * Manages keyboard, mouse, and joystick input for a Ember-compliant platform.
 */
typedef struct emwin_input emwin_input;

/**
 * @brief Identifier for active joysticks.
 */
typedef u64 emwin_joystick_id;

/**
 * @brief Static description of a joystick device.
 */
typedef struct emwin_joystick_device {
    /** @brief Number of analog axes. */
    u32 axis_count;

    /** @brief Number of buttons. */
    u32 button_count;

    /** @brief Number of hat switches. */
    u32 hat_count;

    /** @brief Unique identifier for the joystick. */
    emwin_joystick_id id;

    /** @brief Human-readable device name. */
    const char* name;

    /** @brief Platform-specific GUID identifying the device. */
    u8 guid[16];
} emwin_joystick_device;

/**
 * @brief Runtime state of a joystick device.
 *
 * Arrays are owned by the platform layer and remain valid until the next query.
 */
typedef struct emwin_joystick_state {
    /** @brief Axis values in range [-1.0, 1.0]. */
    f32* axis;

    /** @brief Button states (pressed or not). */
    b8* buttons;
    
    /** @brief Hat states. */
    emwin_hat_state* hats;
} emwin_joystick_state;

/**
 * @brief Initializes the input system.
 *
 * @param desktop Pointer to the desktop platform instance.
 * @param out_input Pointer to receive the created input system.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 * 
 * @note Subsystem will be automatically destroyed when dependent
 *       desktop object is destroyed.
 */
em_result emwin_input_init(emwin_desktop* desktop, emwin_input** out_input);

/**
 * @brief Returns whetever input system handle is valid
 * 
 * @param input Pointer to the input system.
 * @return True if valid, false if deinitialized.
 */
b8 emwin_input_valid(emwin_input* input);

/**
 * @brief Checks whether a keyboard key is currently pressed.
 *
 * @param input Pointer to the input system.
 * @param code Key code to query.
 * @return True if the key is pressed, false otherwise.
 */
b8 emwin_input_key_down(emwin_input* input, emwin_key_code code);

/**
 * @brief Checks whether a mouse button is currently pressed.
 *
 * @param input Pointer to the input system.
 * @param code Mouse button code to query.
 * @return True if the button is pressed, false otherwise.
 */
b8 emwin_input_mouse_down(emwin_input* input, emwin_mouse_code code);

/**
 * @brief Gets current mouse position relative to the primary monitor.
 * 
 * @param input Pointer to the input system.
 */
uvec2 emwin_input_mouse_position(emwin_input* input);

/**
 * @brief Retrieves scroll delta for the connected mouse.
 * 
 * @param input Pointer to the input system.
 */
uvec2 emwin_input_mouse_scroll(emwin_input* input);

/**
 * @brief Retrieves a joystick device by ID.
 *
 * @param input Pointer to the input system.
 * @param id Joystick identifier.
 * @return Pointer to the device if connected, or NULL if not found.
 */
const emwin_joystick_device* emplat_input_get_joystick_device(emwin_input* input, emwin_joystick_id id);

/**
 * @brief Retrieves the current state of a joystick.
 *
 * @param input Pointer to the input system.
 * @param id Joystick identifier.
 * @param out_state Pointer to receive the joystick state.
 */
void emplat_input_get_joystick_state(emwin_input* input, u32 id, emwin_joystick_state* out_state);
