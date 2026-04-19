#include <ember/platform/window.h>
#include <ember/platform/system.h>

#include <ember/gpu/device.h>

#define LOG_OUTPUT(level, message, ...) log_output(level, "Sandbox", message __VA_OPT__(,) __VA_ARGS__)

#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            LOG_OUTPUT(LOG_LEVEL_ERROR, message ": %s",    \
					em_result_string(result, EM_ENABLE_VALIDATION)); \
            goto failed_init;                             \
        }                                                 \
    }

int main(int argc, char** argv) {
	emplat_window_config window_config = emplat_window_default();
	window_config.size = (uvec2) { 640, 640 };
	window_config.title = "Test Window";

	emplat_window window = {};
	CHECK_FUNC(
		emplat_window_open(&window_config, &window), 
		"Failed to open window");

	emgpu_device_config device_config = emgpu_device_default();
	device_config.enabled_modes = EMBER_DEVICE_MODE_GRAPHICS | EMBER_DEVICE_MODE_PRESENT;
	device_config.application_name = window_config.title;
	device_config.enable_validation = TRUE;
	
	emgpu_device device = {};
	CHECK_FUNC(
		emgpu_device_init(&device_config, &device), 
		"Failed to create rendering device");

	if (device_config.enable_validation)
		emgpu_device_print_capabilities(&device, LOG_LEVEL_TRACE);

	emgpu_surface_config surface_config = emgpu_surface_default();
	surface_config.window = &window;
	surface_config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
	surface_config.force_format = FALSE;

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

	f64 last_time = emplat_current_time();
	while (!emplat_window_should_close(&window)) {
		f64 now = emplat_current_time();
		f64 delta_time = now - last_time;
		last_time = now;

		emgpu_frame frame = {};
		if (emgpu_frame_init(&frame) == EMBER_RESULT_OK) {		
			// ------------------
			emgpu_frame_texture window_tex = emgpu_frame_next_surface_texture(&frame, &surface);

			emgpu_frame_set_renderarea(&frame, (uvec2) { 0, 0 }, window.size, TRUE);
			emgpu_frame_bind_renderpass(&frame, &mainpass, &window_tex, 1);

			CHECK_FUNC(
				device.submit_frame(&device, &frame), 
				"Failed to submit device frame");
		}

		emplat_window_pump_messages(&window); 
	}

failed_init:
	device.destroy_renderpass(&device, &mainpass);
	device.destroy_surface(&device, &surface);
	emgpu_device_shutdown(&device);
	emplat_window_close(&window);

	memory_leaks();
	return 0;
}