#include <ember/window/window.h>
#include <ember/gpu/device.h>

#include <ember/gpu/ext/wayland_surface.h>

#include <stdio.h>

#define LOG_OUTPUT(level, message, ...) log_console(level, "Sandbox", message __VA_OPT__(,) __VA_ARGS__)

#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            LOG_OUTPUT(LOG_LEVEL_ERROR, message ": %s",    \
					em_result_string(result, EMBER_BUILD_DEBUG)); \
            goto cleanup;                                 \
        }                                                 \
    }

void* read_file(const char* filepath, u64* out_size) {
    FILE* f = fopen(filepath, "rb");

    fseek(f, 0, SEEK_END);
    u64 size = ftell(f);
    rewind(f);

    void* buffer = malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    if (out_size) *out_size = size;
    return buffer;
}

int main(int argc, char** argv) {
    em_allocator allocator = em_allocator_default();

    emwin_window_config window_config = emwin_window_default();
    window_config.size  = (uvec2) { 640, 640 };
    window_config.title = "Test Window";

    emwin_window window = {};
    CHECK_FUNC(
        emwin_window_open(&window_config, &allocator, NULL, &window),
        "Failed to open window");

    emgpu_extension_desc extensions[] = { emgpu_wayland_surface_extension(emwin_desktop_handle(window.desktop)) };

    emgpu_wayland_surface_api* wsi_extension = NULL; 

    emgpu_device_config device_config = emgpu_device_default();
    device_config.required_modes  = EMBER_DEVICE_MODE_GRAPHICS;
    device_config.optional_modes  = EMBER_DEVICE_MODE_VALIDATION;
    device_config.debug_name      = window_config.title;
    device_config.extensions      = extensions;
    device_config.extension_count = EM_ARRAYSIZE(extensions);
    device_config.out_extensions  = (void**) &wsi_extension;

    emgpu_device device = {};
    CHECK_FUNC(
        emgpu_device_init(&device_config, &allocator, &device),
        "Failed to create rendering device");

	f32 vertices[] = {
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};

	u16 indices[] = { 0, 1, 2, 2, 3, 0 };
    
    // --------------------------------------
    emgpu_device_capabilities capabilities = {};
	CHECK_FUNC(
		device.retrieve_capabilities(&device, &capabilities),
		"Failed to retrieve device capabilities");
	emgpu_device_print_capabilities(&device, &capabilities, LOG_LEVEL_TRACE);
    // --------------------------------------

    // --------------------------------------
    emgpu_wayland_surface_config surface_config = emgpu_surface_default_wayland();
    surface_config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    surface_config.force_format     = FALSE;
    surface_config.display          = emwin_desktop_handle(window.desktop);
    surface_config.surface          = emwin_window_handle(&window);

    emgpu_surface surface = {};
    CHECK_FUNC(
        wsi_extension->create_surface(&device, &allocator, &surface_config, &surface),
        "Failed to create native platform surface");
    
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
		device.create_renderpass(&device, &allocator, &renderpass_config, &mainpass), 
		"Failed to create main renderpass (present)");
    // --------------------------------------

    // --------------------------------------
    emgpu_buffer_config vertex_buffer_config = emgpu_buffer_default();
    vertex_buffer_config.usage = EMBER_BUFFER_USAGE_VERTEX;
    vertex_buffer_config.buffer_size = sizeof(vertices);

    emgpu_buffer vertex_buffer = {};
    CHECK_FUNC(
        device.create_buffer(&device, &allocator, &vertex_buffer_config, &vertex_buffer),
        "Failed to create vertex buffer");
    CHECK_FUNC(
        device.upload_to_buffer(&device, &vertex_buffer, vertices, 0, sizeof(vertices)),
        "Failed to upload data to vertex buffer");

    emgpu_buffer_config index_buffer_config = emgpu_buffer_default();
    index_buffer_config.usage = EMBER_BUFFER_USAGE_INDEX;
    index_buffer_config.buffer_size = sizeof(indices);

    emgpu_buffer index_buffer = {};
    CHECK_FUNC(
        device.create_buffer(&device, &allocator, &index_buffer_config, &index_buffer),
        "Failed to create index buffer");
    CHECK_FUNC(
        device.upload_to_buffer(&device, &index_buffer, indices, 0, sizeof(indices)),
        "Failed to upload data to index buffer");
    // --------------------------------------

    // --------------------------------------
    emgpu_texture_config texture_config = emgpu_texture_default();
    texture_config.image_format   = EMGPU_FORMAT_BGR8_UNORM;
    texture_config.filter_type    = EMBER_FILTER_TYPE_LINEAR;
    texture_config.address_mode   = EMBER_ADDRESS_MODE_CLAMP_TO_BORDER;
    texture_config.usage          = EMBER_TEXTURE_USAGE_SAMPLED | EMBER_TEXTURE_USAGE_STORAGE;
    texture_config.size           = window.size;
    texture_config.max_anisotropy = capabilities.max_anisotropy;

    emgpu_texture image = {};
    CHECK_FUNC(
        device.create_texture(&device, &allocator, &texture_config, &image),
        "Failed to create storage texture");
    // --------------------------------------

    // --------------------------------------
    emgpu_format graphics_attributes[] = { EMGPU_FORMAT_RG32_FLOAT, EMGPU_FORMAT_RGB32_FLOAT, EMGPU_FORMAT_RG32_FLOAT };

    emgpu_descriptor_desc graphics_descriptors[] = {
        {
            .binding = 0,
            .descriptor_type = EMBER_DESCRIPTOR_TYPE_IMAGE_SAMPLER,
            .stage_type = EMBER_SHADER_STAGE_TYPE_FRAGMENT,
        },
    };

    u64 vertex_shader_size = 0;
    void* vertex_shader = read_file("assets/shader_base.vert.spv", &vertex_shader_size);

    u64 fragment_shader_size = 0;
    void* fragment_shader = read_file("assets/shader_base.frag.spv", &fragment_shader_size);

    emgpu_graphics_pipeline_config graphics_pipeline_config = emgpu_pipeline_default_graphics();
    graphics_pipeline_config.topology             = EMBER_PRIMITIVE_TYPE_TRIANGLE_LIST;
    graphics_pipeline_config.attribute_count      = EM_ARRAYSIZE(graphics_attributes);
    graphics_pipeline_config.attributes           = graphics_attributes;
	graphics_pipeline_config.descriptor_count     = EM_ARRAYSIZE(graphics_descriptors);
    graphics_pipeline_config.descriptors          = graphics_descriptors;
    
    graphics_pipeline_config.vertex_shader.data   = vertex_shader;
    graphics_pipeline_config.vertex_shader.size   = vertex_shader_size;
    graphics_pipeline_config.vertex_shader.entry_point = "main";

    graphics_pipeline_config.fragment_shader.data = fragment_shader;
    graphics_pipeline_config.fragment_shader.size = fragment_shader_size;
    graphics_pipeline_config.fragment_shader.entry_point = "main";

    emgpu_pipeline gfx_pipeline = {};
    CHECK_FUNC(
        device.create_graphics_pipeline(&device, &allocator, &graphics_pipeline_config, &mainpass, &gfx_pipeline),
        "Failed to create graphics pipeline");
    // --------------------------------------

	show_memory_stats();

    while (!emwin_window_should_close(&window)) {

		emgpu_frame frame = {};
		if (emgpu_frame_init(&frame, &device.frame_allocator) == EMBER_RESULT_OK) {		
			// -----------------------------------------------------------------------------------------
            emgpu_frame_texture window_tex = emgpu_frame_next_surface_texture(&frame, &surface);

            emgpu_frame_set_renderarea(&frame, (uvec2) { 0, 0 }, window.size);
            emgpu_frame_begin_renderpass(&frame, &mainpass, &window_tex, 1);
            emgpu_frame_bind_pipeline(&frame, &gfx_pipeline, &vertex_buffer, &index_buffer);
            emgpu_frame_draw_indexed(&frame, 6, 1);
            emgpu_frame_end_renderpass(&frame);

            emgpu_frame_flush(&frame);

			CHECK_FUNC(
				device.submit_frame(&device, &frame), 
				"Failed to submit device frame");
		}

		emwin_desktop_pump_messages(window.desktop);
	}

cleanup:
	device.destroy_renderpass(&device, &allocator, &mainpass);
    device.destroy_surface(&device, &allocator, &surface);
    emgpu_device_shutdown(&allocator, &device);
    emwin_window_close(&allocator, &window);

    memory_leaks();
    return 0;
}