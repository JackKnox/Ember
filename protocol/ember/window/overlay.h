#pragma once 

#include "ember/core.h"

#include "ember/window/desktop.h"
#include "ember/window/shm.h"

/*
 * @brief Configuration used when creating an overlay.
 *
 * This structure define all parameters required to initialise a platform overlay,
 * including size, anchor, layering, exclusive zones and behavioral flags.
 */
typedef enum emwin_overlay_config {
    /* @brief Debug name used for the overlay. */
    const char* debug_name;

    /* Overlay creation flags. */
    emwin_overlay_flags flags;

    /* @brief Side of revelant monitor to anchor overlay too. */
    emwin_overlay_anchor anchor;

    /* @brief Layer of WM to render overlay on. */
    emwin_overlay_layer layer;

    /**
     * @brief Exclusive zone in surface-local coordinates.
     *
     * Positive values reserve space; 0 allows the surface to be moved around
     * exclusive zones; -1 prevents the surface from being moved to accommodate them.
     */
    u32 exclusive_zone;

    /** 
     * @brief Size of overlay.
     *
     * 0 allows the overlay to extend to the end of the anchored edge.
     */
    uvec2 size;
} emwin_overlay_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default overlay configuration.
 *
 * @return A default-initialized emwin_overlay_config.
 */
emwin_overlay_config emwin_overlay_default();

#endif

typedef struct emwin_overlay {
    /** @brief Current client area size in pixels. Updated on resize events. */
    uvec2 size;

    /** @brief Assigned ID of the overlay. */
    emwin_window_id id;

    /** @brief Owner desktop object, represents a connection to the WM. */
    emwin_desktop* desktop;

    /** @brief Platform-specific overlay state. */
    void* internal_context;
} emwin_overlay;

/*
 * @brief Creates and opens a overlay.
 *
 * Initialises a platform overlay achored to a monitors edge and writes
 * the resulting state to @p out_window.
 *
 * @param allocator Allocator used for unternal allocations.
 * @param config Overlay configuration.
 * @param monitor Monitor to assign overlay to.
 * @param out_overlay Output overlay pointer.
 * @param out_desktop Pointer to desktop, lazyily initialises.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeds.
 * 
 * @note Passing a valid @p out_desktop is strongly recommended to avoid
 *       reinitialising shared global platform state.
 */
em_result emwin_overlay_open(em_allocator* allocator, const emwin_overlay_config* config, emwin_monitor_id monitor, emwin_overlay* out_overlay, emwin_desktop* out_desktop);

/*
 * @brief Forces closing a overley and destroys all OS resources.
 *
 * Releases all platform and renderer resource associated with the window and immedialtly closes.
 */
void emwin_overlay_close(em_allocator* allocator, emwin_overlay* overlay);

/**
 * @brief Attaches a shared-memory buffer to a overlay.
 *
 * The buffer is used as the overlay's backing pixel storage. The specified
 * offset determines the position within the window at which the buffer is
 * attached.
 *
 * @param overlay Overlay to attach the buffer to.
 * @param buffer Shared-memory buffer to attach.
 * @param offset Offset within the window at which to attach the buffer.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emwin_overlay_attach(emwin_overlay* overlay, emwin_shm_buffer* buffer, uvec2 offset);

/**
 * @brief Marks a region of a overlay as damaged.
 *
 * The damaged region indicates an area whose contents have changed and
 * should be presented to the desktop.
 *
 * @param overlay Overlay whose contents were modified.
 * @param offset Offset of the damaged region within the window.
 * @param size Size of the damaged region.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emwin_overlay_damage(emwin_overlay* overlay, uvec2 offset, uvec2 size);
