#include <ember/platform/window.h>
#include <ember/platform/global.h>

#include <ember/gpu/device.h>

int main(int argc, char** argv) {
	emplat_window_config window_config = emplat_window_default();
	window_config.size = (uvec2) { 640, 640 };
	window_config.title = "Test Window";

	emgpu_device_config device_config = emgpu_device_default();
	device_config.enabled_modes = EMBER_DEVICE_MODE_GRAPHICS | EMBER_DEVICE_MODE_PRESENT;
	device_config.application_name = window_config.title;

	emplat_window window = {};
	if (emplat_window_start(&window_config, &window) != EMBER_RESULT_OK) {
        emc_console_write("Failed to open window\n");
        goto failed_init;
	}
	
	emgpu_device device = {};
	if (emgpu_device_init(&device_config, &device) != EMBER_RESULT_OK) {
		emc_console_write("Failed to create rendering device\n");
		goto failed_init;
	}

	emgpu_surface surface = {};
	if (device.create_surface(&device, &window, &surface) != EMBER_RESULT_OK) {
		emc_console_write("Failed to create window surface\n");
		goto failed_init;
	}

	emgpu_attachment_config attachments[] = {
		{
			.type = EMBER_ATTACHMENT_TYPE_PRESENT,
			.format = surface.pixel_format,
			.load_op = EMBER_LOAD_OP_CLEAR,
			.store_op = EMBER_STORE_OP_STORE,
			.stencil_load_op = EMBER_LOAD_OP_DONT_CARE,
			.stencil_store_op = EMBER_STORE_OP_DONT_CARE,
		}
	};

	emgpu_present_target_config rendertarget_config = emgpu_rendertarget_default_present();
	rendertarget_config.surface = &surface;
	rendertarget_config.attachments = attachments;
	rendertarget_config.attachment_count = EM_ARRAYSIZE(attachments);

	emgpu_rendertarget surface_target = {};
	if (device.create_present_target(&device, &rendertarget_config, &surface_target) != EMBER_RESULT_OK) {
		emc_console_write("Failed to create main rendertarget (present)\n");
		goto failed_init;
	}

	show_memory_stats();

	f64 last_time = emplat_current_time();
	while (!emplat_window_should_close(&window)) {
		f64 now = emplat_current_time();
		f64 delta_time = now - last_time;
		last_time = now;

		emgpu_frame frame = {};
		if (emgpu_frame_init(&frame) == EMBER_RESULT_OK) {		
			// ------------------
			emgpu_frame_bind_rendertarget(&frame, &surface_target);

			if (device.submit_frame(&device, &frame) != EMBER_RESULT_OK) {
				emc_console_write("Failed to end submit device frame\n");
				goto failed_init;
			}
		}

		emplat_window_pump_messages(&window); 
	}

failed_init:
	device.destroy_rendertarget(&device, &surface_target);
	device.destroy_surface(&device, &surface);
	emgpu_device_shutdown(&device);
	emplat_window_close(&window);

	memory_shutdown();
	return 0;
}