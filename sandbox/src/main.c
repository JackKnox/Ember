#include <ember/window/window.h>
#include <ember/gpu/device.h>

#define LOG_OUTPUT(level, message, ...) log_console(level, "Sandbox", message __VA_OPT__(,) __VA_ARGS__)

#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            LOG_OUTPUT(LOG_LEVEL_ERROR, message ": %s",    \
					em_result_string(result, EMBER_BUILD_DEBUG)); \
            goto failed_init;                             \
        }                                                 \
    }

int main(int argc, char** argv) {
	em_allocator system_alloc = em_allocator_default(); // malloc / free.

	emwin_window_config window_config = emwin_window_default();
	window_config.size = (uvec2) { 640, 640 };
	window_config.title = "Test Window";

	emwin_window window = {};
	CHECK_FUNC(
		emwin_window_open(&window_config, &system_alloc, NULL, &window), 
		"Failed to open window");
	
	emgpu_device_config device_config = emgpu_device_default();
	device_config.required_modes = EMBER_DEVICE_MODE_GRAPHICS | EMBER_DEVICE_MODE_PRESENT;
	device_config.optional_modes = EMBER_DEVICE_MODE_VALIDATION;
	device_config.application_name = window_config.title;
	
	emgpu_device device = {};
	CHECK_FUNC(
		emgpu_device_init(&device_config, &system_alloc, &device), 
		"Failed to create rendering device");
	
	emgpu_device_capabilities capabilities = {};
	CHECK_FUNC(
		device.retreive_capabilities(&device, &capabilities),
		"Failed to retrieve device capabilities");
	emgpu_device_print_capabilities(&device, &capabilities, LOG_LEVEL_TRACE);

	emgpu_surface_config surface_config = emgpu_surface_default();
	surface_config.size = window.size;
	surface_config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
	surface_config.force_format = FALSE;
	
	surface_config.window.display    = emwin_desktop_handle(window.global_desktop);
	surface_config.window.handle     = emwin_window_handle(&window);
	surface_config.window.debug_name = window.title;

	emgpu_surface surface = {};
	CHECK_FUNC(
		device.create_surface(&device, &surface_config, &surface),
		"Failed to create window surface");

	emgpu_attachment_config attachments[] = {
		{
			.type = EMBER_ATTACHMENT_TYPE_COLOUR,
			.format = surface.pixel_format,
			.load_op = EMBER_LOAD_OP_CLEAR,
			.store_op = EMBER_STORE_OP_STORE,
			.stencil_load_op = EMBER_LOAD_OP_DONT_CARE,
			.stencil_store_op = EMBER_STORE_OP_DONT_CARE,
			.presentable = TRUE,
		}
	};

	emgpu_renderpass_config renderpass_config = emgpu_renderpass_default();
	renderpass_config.attachments = attachments;
	renderpass_config.attachment_count = EM_ARRAYSIZE(attachments);

	emgpu_renderpass mainpass = {};
	CHECK_FUNC(
		device.create_renderpass(&device, &renderpass_config, &mainpass), 
		"Failed to create main renderpass (present)");

	show_memory_stats();

	while (!emwin_window_should_close(&window)) {

		emgpu_frame frame = {};
		if (emgpu_frame_init(&frame, &device.frame_allocator) == EMBER_RESULT_OK) {		
			// ------------------
			emgpu_frame_texture window_tex = emgpu_frame_next_surface_texture(&frame, &surface);

			emgpu_frame_set_renderarea(&frame, (uvec2) { 0, 0 }, window.size);
			emgpu_frame_begin_renderpass(&frame, &mainpass, &window_tex, 1);
			emgpu_frame_end_renderpass(&frame);

			emgpu_frame_flush(&frame);

			CHECK_FUNC(
				device.submit_frame(&device, &frame), 
				"Failed to submit device frame");
		}

		emwin_desktop_pump_messages(window.global_desktop);
	}

failed_init:
	device.destroy_renderpass(&device, &mainpass);
	device.destroy_surface(&device, &surface);
	emgpu_device_shutdown(&device);
	emwin_window_close(&window);

	memory_leaks();
	return 0;
}