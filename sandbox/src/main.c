#include <ember/platform/window.h>
#include <ember/platform/global.h>

#include <ember/gpu/device.h>
#include <ember/gpu/commandbuf.h>

int main(int argc, char** argv) {
	emplat_window_config window_config = emplat_window_default();
	window_config.size = (uvec2) { 640, 640 };
	window_config.title = "Test Window";

	emgpu_device_config device_config = emgpu_device_default();
	device_config.application_name = window_config.title;
	device_config.size = window_config.size;

	emplat_window window = {};
	if (!emplat_window_start(&window_config, &window)) {
        emc_console_write("Failed to open window\n");
        goto failed_init;
	}

	emgpu_device device = {};
	if (!emgpu_device_init(&device_config, &window, &device)) {
		emc_console_write("Failed to create rendering device\n");
		goto failed_init;
	}

	device.main_rendertarget.clear_colour = 0x1f1f1fff;

	show_memory_stats();

	emgpu_commandbuf commandbuf = {};

	f64 last_delta_time = 0;

	f64 last_time = emplat_current_time();
	while (!emplat_window_should_close(&window)) {
		f64 now = emplat_current_time();
		f64 delta_time = now - last_time;
		last_time = now;
		
		if (abs(last_delta_time - delta_time) > 4.0f) {
			EM_TRACE("FPS %.2f", 1000.0f / delta_time);
			last_delta_time = delta_time;
		}

		if (device.begin_frame(&device, delta_time)) {
			// ------------------
			emgpu_commandbuf_begin(&commandbuf);
			emgpu_commandbuf_bind_rendertarget(&commandbuf, &device.main_rendertarget);
			emgpu_commandbuf_end(&commandbuf);
			// ------------------

			if (!emgpu_device_submit_commandbuf(&device, &commandbuf)) {
				emc_console_write("Failed to submit command buffer\n");
				goto failed_init;
			}

			if (!device.end_frame(&device)) {
				emc_console_write("Failed to end frame\n");
				goto failed_init;
			}
		}

		emplat_window_pump_messages(&window); 
	}

	emgpu_commandbuf_destroy(&commandbuf);

failed_init:
	emgpu_device_shutdown(&device);
	emplat_window_close(&window);

	memory_shutdown();
	return 0;
}