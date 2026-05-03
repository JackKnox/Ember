#include "ember/core.h"
#include "vulkan_pipeline.h"

#include "ember/core/darray.h"

em_result vulkan_pipeline_create_layout(
    emgpu_device* device, 
    emgpu_shader_src* shader_sources, emgpu_shader_stage_type* stage_types, u32 shader_source_count, 
    emgpu_descriptor_desc* descriptors, u32 descriptor_count, 
    VkPipelineShaderStageCreateInfo* out_shader_stages,
    emgpu_pipeline* out_pipeline) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_pipeline* internal_pipeline = (internal_vulkan_pipeline*)out_pipeline->internal_data;

    for (u32 i = 0; i < shader_source_count; ++i) {
        const emgpu_shader_src* src = &shader_sources[i];
        if (src->size == 0) continue;

        VkShaderModuleCreateInfo module_create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        module_create_info.codeSize = src->size;
        module_create_info.pCode = src->data;

        VkPipelineShaderStageCreateInfo* stage_create_info = darray_push_empty(out_shader_stages);
        stage_create_info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info->stage = shader_type_to_vulkan_type(stage_types[i]);
        stage_create_info->pName = src->entry_point;

        CHECK_VKRESULT(
            vkCreateShaderModule(context->logical_device, &module_create_info, context->allocator, &stage_create_info->module),
            "Failed to create Vulkan shader module when creating pipeline");
    }

    if (descriptor_count > 0) {
        // Collect descriptor data into vulkan structs.
		VkDescriptorSetLayoutBinding* descriptor_bindings = darray_reserve(VkDescriptorSetLayoutBinding, descriptor_count, NULL, MEMORY_TAG_TEMP);
		VkDescriptorPoolSize* descriptor_pools = darray_reserve(VkDescriptorPoolSize, descriptor_count, NULL, MEMORY_TAG_TEMP);

        for (u32 i = 0; i < descriptor_count; ++i) {
			VkDescriptorSetLayoutBinding* binding = darray_push_empty(descriptor_bindings);
			binding->binding         = descriptors[i].binding;
			binding->descriptorType  = descriptor_type_to_vulkan_type(descriptors[i].descriptor_type);
			binding->descriptorCount = 1;
			binding->stageFlags      = shader_type_to_vulkan_type(descriptors[i].stage_type);

			VkDescriptorPoolSize* pool_size = darray_push_empty(descriptor_pools);
			pool_size->type            = binding->descriptorType;
			pool_size->descriptorCount = context->frames_in_flight;
		}
        // ------------------------------------------

        // Create descriptor set layout.
		VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = darray_length(descriptor_bindings);
		layoutInfo.pBindings    = descriptor_bindings;
		CHECK_VKRESULT(
            vkCreateDescriptorSetLayout(context->logical_device, &layoutInfo, context->allocator, &internal_pipeline->descriptor),
            "Failed to create Vulkan descriptor set layout when creating pipeline");
        // ------------------------------------------

        // Create descriptor pool.
		VkDescriptorPoolCreateInfo pool_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		pool_info.maxSets       = context->frames_in_flight;
		pool_info.poolSizeCount = darray_length(descriptor_pools);
		pool_info.pPoolSizes    = descriptor_pools;
        CHECK_VKRESULT(
            vkCreateDescriptorPool(context->logical_device, &pool_info, context->allocator, &internal_pipeline->descriptor_pool),
            "Failed to create local Vulkan descriptor pool in pipeline");
        // ------------------------------------------

        // Create descriptor sets per frame in flight.
		internal_pipeline->descriptor_sets = darray_from_data(VkDescriptorSet, context->frames_in_flight, NULL, NULL, MEMORY_TAG_RENDERER);

        VkDescriptorSetLayout* layouts = darray_reserve(VkDescriptorSetLayout, darray_capacity(internal_pipeline->descriptor_sets), NULL, MEMORY_TAG_TEMP);
		for (u32 i = 0; i < context->frames_in_flight; ++i)
			darray_push(layouts, internal_pipeline->descriptor);

        VkDescriptorSetAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		alloc_info.descriptorPool     = internal_pipeline->descriptor_pool;
		alloc_info.descriptorSetCount = darray_length(layouts);
		alloc_info.pSetLayouts        = layouts;
        CHECK_VKRESULT(
            vkAllocateDescriptorSets(context->logical_device, &alloc_info, internal_pipeline->descriptor_sets),
            "Failed to allocate descriptor sets when creating pipeline");
        // ------------------------------------------

        // Destory all temp memory...
		darray_destroy(descriptor_pools);
		darray_destroy(descriptor_bindings);
		darray_destroy(layouts);
    }

    VkPipelineLayoutCreateInfo layout_create_info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    layout_create_info.pSetLayouts = &internal_pipeline->descriptor;
	if (descriptor_count > 0) layout_create_info.setLayoutCount = 1;

    CHECK_VKRESULT(
        vkCreatePipelineLayout(context->logical_device, &layout_create_info, context->allocator, &internal_pipeline->layout),
        "Failed to create Vulkan pipeline layout when creating pipeline");
    return EMBER_RESULT_OK;
}

em_result vulkan_pipeline_create_graphics(
    emgpu_device* device, 
    const emgpu_graphics_pipeline_config* config, 
    emgpu_renderpass* bound_renderpass, 
    emgpu_pipeline* out_graphics_pipeline) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    out_graphics_pipeline->internal_data = (internal_vulkan_pipeline*)mem_allocate(NULL, sizeof(internal_vulkan_pipeline), MEMORY_TAG_RENDERER);
    internal_vulkan_pipeline* internal_pipeline = (internal_vulkan_pipeline*)out_graphics_pipeline->internal_data;

    out_graphics_pipeline->type = EMBER_OPER_TYPE_GRAPHICS;
    internal_pipeline->graphics.vertex_buffer = config->vertex_buffer;
    internal_pipeline->graphics.index_buffer = config->index_buffer;

    VkPipelineShaderStageCreateInfo* shader_stages = darray_create(VkPipelineShaderStageCreateInfo, NULL, MEMORY_TAG_TEMP);
    emgpu_shader_src shaders_srcs[] = { config->vertex_shader, config->fragment_shader };
    emgpu_shader_stage_type stage_types[] = { EMBER_SHADER_STAGE_TYPE_VERTEX, EMBER_SHADER_STAGE_TYPE_FRAGMENT };

    em_result result = vulkan_pipeline_create_layout(
        device, shaders_srcs, stage_types, EM_ARRAYSIZE(shaders_srcs), config->descriptors, config->descriptor_count, shader_stages, out_graphics_pipeline);
    if (result != EMBER_RESULT_OK) return result;
    
    // Dynamic states.
    // Viewports and scissor is needed to handle resizing.
    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT ,
        VK_DYNAMIC_STATE_SCISSOR ,
        //VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    // Vertex input configuration
    // Calculate total vertex stride and fill attribute descriptions.   
    VkVertexInputBindingDescription binding_desc = {};
    VkVertexInputAttributeDescription* attributes = darray_reserve(VkVertexInputAttributeDescription, config->vertex_attribute_count, NULL, MEMORY_TAG_TEMP);

    u64 attribute_stride = 0;
    for (u32 i = 0; i < config->vertex_attribute_count; ++i) {
        emgpu_format attribute = config->vertex_attributes[i];

        VkVertexInputAttributeDescription* descriptor = darray_push_empty(attributes);
        descriptor->location = i;
        descriptor->binding  = 0;
        descriptor->format   = format_to_vulkan_type(attribute);
        descriptor->offset   = attribute_stride;
        attribute_stride += EMBER_FORMAT_SIZE(attribute);
    }

    binding_desc.binding = 0;
    binding_desc.stride  = attribute_stride;
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Colour attachments 
    VkPipelineColorBlendAttachmentState colour_blending = {};
    colour_blending.blendEnable = FALSE; // TODO: Enable / disable blending.
    colour_blending.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colour_blending.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colour_blending.colorBlendOp        = VK_BLEND_OP_ADD;
    colour_blending.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colour_blending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colour_blending.alphaBlendOp        = VK_BLEND_OP_ADD;

    colour_blending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.depthClampEnable        = FALSE;
    rasterizer.rasterizerDiscardEnable = FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode                = VK_CULL_MODE_NONE;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth               = 1.0f;

    // Multisampling state
    VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // No MSAA
    multisampling.sampleShadingEnable  = FALSE;

    // Depth / stencil state
    VkPipelineDepthStencilStateCreateInfo depth_stencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depth_stencil.depthTestEnable       = FALSE; // TODO: Make configurable
    depth_stencil.depthWriteEnable      = FALSE;
    depth_stencil.depthCompareOp        = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = FALSE;
    depth_stencil.stencilTestEnable     = FALSE;

    // Color blending state
    VkPipelineColorBlendStateCreateInfo color_blending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    color_blending.logicOpEnable   = FALSE;
    color_blending.logicOp         = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments    = &colour_blending;

    // Dynamic states
    VkPipelineDynamicStateCreateInfo dynamic_state = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic_state.dynamicStateCount = EM_ARRAYSIZE(dynamic_states);
    dynamic_state.pDynamicStates    = dynamic_states;

    // Vertex input state
    VkPipelineVertexInputStateCreateInfo vertex_input_state = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    // Only configure if vertex attributes exist
    if (config->vertex_attribute_count > 0) {
        vertex_input_state.vertexAttributeDescriptionCount = darray_length(attributes);
        vertex_input_state.pVertexAttributeDescriptions    = attributes;
        vertex_input_state.vertexBindingDescriptionCount   = 1;
        vertex_input_state.pVertexBindingDescriptions      = &binding_desc;
    }

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo input_assembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    input_assembly.topology = primitive_to_vulkan_type(config->vertex_topology);

    internal_vulkan_renderpass* attachted_renderpass = (internal_vulkan_renderpass*)bound_renderpass->internal_data;

    // Attach all configured states to the pipeline create info
    VkGraphicsPipelineCreateInfo pipeline_create_info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipeline_create_info.stageCount = darray_length(shader_stages);
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &input_assembly;
    pipeline_create_info.pRasterizationState = &rasterizer;
    pipeline_create_info.pMultisampleState = &multisampling;
    pipeline_create_info.pDepthStencilState = &depth_stencil;
    pipeline_create_info.pColorBlendState = &color_blending;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.layout = internal_pipeline->layout;
    pipeline_create_info.renderPass = attachted_renderpass->handle;

    darray_destroy(shader_stages);
    return EMBER_RESULT_OK;
}

em_result vulkan_pipeline_create_compute(
    emgpu_device* device, 
    const emgpu_compute_pipeline_config* config, 
    emgpu_pipeline* out_compute_pipeline) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    out_compute_pipeline->internal_data = (internal_vulkan_pipeline*)mem_allocate(NULL, sizeof(internal_vulkan_pipeline), MEMORY_TAG_RENDERER);
    internal_vulkan_pipeline* internal_pipeline = (internal_vulkan_pipeline*)out_compute_pipeline->internal_data;

    out_compute_pipeline->type = EMBER_OPER_TYPE_COMPUTE;

    VkPipelineShaderStageCreateInfo* shader_stages = darray_create(VkPipelineShaderStageCreateInfo, NULL, MEMORY_TAG_TEMP);
    emgpu_shader_src shaders_srcs[] = { config->compute_shader };
    emgpu_shader_stage_type stage_types[] = { EMBER_SHADER_STAGE_TYPE_COMPUTE };

    em_result result = vulkan_pipeline_create_layout(
        device, shaders_srcs, stage_types, EM_ARRAYSIZE(shaders_srcs), config->descriptors, config->descriptor_count, shader_stages, out_compute_pipeline);
    if (result != EMBER_RESULT_OK) return result;

    VkComputePipelineCreateInfo pipeline_create_info = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    pipeline_create_info.stage = shader_stages[0];
    pipeline_create_info.layout = internal_pipeline->layout;

    CHECK_VKRESULT(
        vkCreateComputePipelines(context->logical_device, 0, 1, &pipeline_create_info, context->allocator, &internal_pipeline->handle),
        "Failed to create internal Vulkan pipeline");
    
    for (u32 i = 0; i < darray_length(shader_stages); ++i)
        vkDestroyShaderModule(context->logical_device, shader_stages[i].module, context->allocator);
    darray_destroy(shader_stages);
    return EMBER_RESULT_OK;
}

em_result vulkan_pipeline_update_descriptors(
    emgpu_device* device, 
    emgpu_pipeline* pipeline, 
    emgpu_update_descriptors* descriptors, 
    u32 descriptor_count) {

}

void vulkan_pipeline_bind(
    emgpu_device* device, 
    VkCommandBuffer command_buffer,
    emgpu_pipeline* pipeline) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_pipeline* internal_pipeline = (internal_vulkan_pipeline*)pipeline->internal_data;
    
    VkPipelineBindPoint bind_point = ops_type_to_bind_point(pipeline->type);

    vkCmdBindPipeline(command_buffer, bind_point, internal_pipeline->handle);

    if (internal_pipeline->descriptor_sets)
        vkCmdBindDescriptorSets(command_buffer, bind_point, internal_pipeline->layout, 0, 1, &internal_pipeline->descriptor_sets[device->current_frame], 0, 0);
    
    // Bind vertex and index buffers.
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS && internal_pipeline->graphics.vertex_buffer) {
        VkDeviceSize offset = 0;

        internal_vulkan_buffer* vertex_buffer = (internal_vulkan_buffer*)internal_pipeline->graphics.vertex_buffer->internal_data;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer->handle, &offset);

        if (internal_pipeline->graphics.index_buffer) {
            internal_vulkan_buffer* index_buffer = (internal_vulkan_buffer*)internal_pipeline->graphics.index_buffer->internal_data;
            vkCmdBindIndexBuffer(command_buffer, index_buffer->handle, offset, VK_INDEX_TYPE_UINT16); // TODO: Customize index type?
        }
    }
}

void vulkan_pipeline_destroy(
    emgpu_device* device, 
    emgpu_pipeline* pipeline) {

}