// Example when we open a window then clear a colour to it.
//
#include <ember/window/window.h>
#include <ember/gpu/ext/emwin_surface.h>

#include <ember/gpu/raster.h>
#include <ember/gpu/frame.h>

#include <stddef.h>
#include <stdbool.h>

// A small helper macro to cut down on the repetitive error-check boilerplate.
// Calls a function, checks the result, logs a message and jumps to cleanup if it failed.
// em_result_string() converts the result code into something human readable.
#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            emlog_console(LOG_LEVEL_ERROR, "Examples", message ": %s", \
				em_result_string(result, true));           \
            goto cleanup;                                  \
        }                                                  \
    }

int main(int argc, char** argv) {
	// The Allocator API. A simple structure wrapping malloc / free / realloc and
	// some metadata. Every Ember subsystem takes one of these so you can plug in
	// a custom allocator (arena, pool, tracking, etc.) wherever you need to.
	// em_allocator_default() just hands back the OS allocator — malloc and free.
	em_allocator system_alloc = em_allocator_default();

	// Before we can open a window we need to fill out a config.
	// emwin_window_default() pre-fills sensible values so we only need to touch the fields we actually care about.
	emwin_window_config window_config = emwin_window_default();
	window_config.size  = (uvec2) { 640, 640 };             // Initial dimensions of the window in pixels.
	window_config.title = "Example - Clear Colour";         // Shows up in the title bar; uses UTF-8 strings.
	//window_config.desktop = NULL;

	// emwin_desktop represents our connection to the system's window manager.
	// emwin_window_open will create one for us and write the pointer here.
    emwin_desktop* desktop = NULL;

	// Open the window. The desktop pointer is also populated here as a side effect —
	// we need it for the GPU surface extension below.
	emwin_window window = {};
	CHECK_FUNC(
		emwin_window_open(&window_config, &system_alloc, &window, &desktop), 
		"Failed to open window");

	// The GPU device supports extensions for platform-specific surface creation.
	// Using ember_window we use the emwin_gpu_surface extension and pass it our desktop handle
	// so the backend knows which desktop to talk to. This interally wraps to actual platform APIs.
	emgpu_extension_desc extensions[] = { emgpu_emwin_surface_extension(desktop) };

	// This is an 'out extension' — after device init, Ember writes the actual
	// surface creation function pointers into this struct so we can call them.
    emgpu_emwin_surface_ext wsi_extension = {};

	// Now configure the GPU device. Like the window, there's a _default() helper
	// that pre-fills everything sensible so we only need to set what matters.
	emgpu_device_config device_config = emgpu_device_default();
	device_config.debug_name       = window_config.title;    // Shows up in GPU debug tooling.
	device_config.frame_allocator  = em_allocator_default(); // Allocator used to allocate frame-local resources.
	device_config.app_version      = EMBER_VERSION;
	device_config.required_modes   = EMBER_DEVICE_MODE_RASTER | EMBER_DEVICE_MODE_PRESENT; // We need both — no point continuing without them.
	device_config.optional_modes   = EMBER_DEVICE_MODE_VALIDATION; // Nice to have for debugging but we won't bail if it's unavailable.
	device_config.frames_in_flight = 3;                            // Triple buffering.
    device_config.extension_count  = EM_ARRAYSIZE(extensions);
    device_config.extensions       = extensions;    // Pass in the extensions we want...
    device_config.out_extensions   = &wsi_extension; // ...and where to write the function pointers back to.
	
	emgpu_device device = {};
	CHECK_FUNC(
		emgpu_device_init(&device_config, &system_alloc, &device), 
		"Failed to create rendering device");

	// Let's see what the chosen device actually supports and print it to the trace logger — handy during development.
	emgpu_device_capabilities capabilities = {};
	CHECK_FUNC(
		emgpu_device_get_capabilities(&device, &capabilities),
		"Failed to retrieve device capabilities");
	emgpu_device_print_capabilities(&device, &capabilities, LOG_LEVEL_TRACE);

	// Now we can create a ember_window-backed surface using the function pointer the device
	// extension filled in for us. This connects the Vulkan swapchain to our window.
	emgpu_emwin_surface_config surface_config = emgpu_emwin_surface_default();
	surface_config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM; // Common format; force_format = FALSE means we fall back gracefully if unavailable.
	surface_config.force_format     = false;                    // force_format = TRUE means it will only accept the exact format. Still preserves colour / depth / stencil type.
    surface_config.window           = &window;

	// Call the function from the 'out extension' which creates the window.
	emgpu_surface surface = {};
	CHECK_FUNC(
		wsi_extension.create_surface(&device, &system_alloc, &surface_config, &surface),
		"Failed to create window surface");

	// A renderpass describes what attachments we're going to draw into and how.
	// Here we just have a single colour attachment that will be presented to the screen.
	// load_op CLEAR means 'wipe it at the start of the pass'.
	// store_op STORE means 'keep the result so we can present it'.
	emgpu_attachment_config attachments[] = {
		emgpu_attachment_from_surface(&surface) // Match whatever format the surface negotiated.

		// Equivalent to this...
		//{
		//	.type = EMBER_ATTACHMENT_TYPE_COLOUR,
		//	.format = surface.pixel_format,  
		//	.load_op = EMBER_LOAD_OP_CLEAR,
		//	.store_op = EMBER_STORE_OP_STORE,
		//	.stencil_load_op = EMBER_LOAD_OP_DONT_CARE,   // No stencil buffer — we don't care.
		//	.stencil_store_op = EMBER_STORE_OP_DONT_CARE,
		//	.presentable = TRUE, // Tells the backend this attachment will be shown on screen.
		//}
	};

	emgpu_renderpass_config renderpass_config = emgpu_renderpass_default();
	renderpass_config.attachments      = attachments;
	renderpass_config.attachment_count = EM_ARRAYSIZE(attachments);

	emgpu_renderpass mainpass = {};
	CHECK_FUNC(
		emgpu_device_create_renderpass(&device, &system_alloc, &renderpass_config, &mainpass), 
		"Failed to create main renderpass (present)");
	
	// Main loop. emwin_window_should_close becomes true when the user hits the X
	// button or we signal the window to close ourselves.
	while (!emwin_window_should_close(&window)) {

		// Each frame is driven by emgpu_frame. It manages per-frame allocations
		// using the device's frame_allocator — everything allocated here is
		// automatically freed when the frame is submitted.
		emgpu_frame frame = {};
		if (emgpu_frame_init(&frame, &device.frame_allocator) == EMBER_RESULT_OK) {
			// Grab the next texture from the swapchain to render into.
			emgpu_frame_texture window_tex = emgpu_frame_next_surface_texture(&frame, &surface);

			// Set the render area to cover the full window, begin our single pass,
			// then immediately end it. No actual draw calls yet — this just clears the surface.
			emgpu_frame_set_renderarea(&frame, (uvec2) { 0, 0 }, window.size);
			emgpu_frame_begin_renderpass(&frame, &mainpass, 0xAAFFFFFF, &window_tex, 1);
			emgpu_frame_end_renderpass(&frame);

			// Flush finalises the command buffer and prepares it for submission.
			emgpu_frame_flush(&frame);

			// Submit the frame to the GPU. This also handles presentation — CHECK_FUNC
			// will jump to cleanup if something goes wrong here.
			CHECK_FUNC(
				emgpu_device_submit_frame(&device, &frame), 
				"Failed to submit device frame");
		}

		// Process all pending window events — input, resize, close requests, etc.
		// This single call covers every window managed by this desktop connection.
		emwin_desktop_update(desktop);
	}

	// Cleanup. Order matters here — destroy GPU resources before shutting down
	// the device, and shut down the device before closing the window.
	// The allocator passed to each destroy call MUST match the one used to create it
	// or you'll corrupt the allocator's internal state.
cleanup:
	emgpu_device_destroy_renderpass(&device, &system_alloc, &mainpass);
	emgpu_device_destroy_surface(&device, &system_alloc, &surface);
	emgpu_device_shutdown(&system_alloc, &device);
	emwin_window_close(&system_alloc, &window); // Also tears down the desktop connection.
	return 0;
}