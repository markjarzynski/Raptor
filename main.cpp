#include <EASTL/version.h>
#include <EASTL/allocator.h>
#include <EAStdC/EASprintf.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "Raptor.h"
#include "Window.h"
#include "GPUDevice.h"
#include "ResourceManager.h"
#include "GPUProfiler.h"
#include "Renderer.h"
#include "Texture.h"
#include "Sampler.h"
#include "DebugUI.h"
#include "File.h"
#include "Mesh.h"

// These new operators are required by EASTL
void* __cdecl operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void debug_print_versions()
{
    EA::StdC::Printf("%s version: %s\n", RAPTOR_PROJECT_NAME, RAPTOR_VERSION);
    EA::StdC::Printf("EASTL version: %s\n", EASTL_VERSION);
    EA::StdC::Printf("GLFW version: %s\n", glfwGetVersionString());
    EA::StdC::Printf("Dear ImGui version: %s\n", ImGui::GetVersion());

    uint32_t instanceVersion = VK_API_VERSION_1_0;
    vkEnumerateInstanceVersion(&instanceVersion);
    EA::StdC::Printf("Vulkan version: %d.%d.%d\n", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
}

static uint8* GetBufferData(std::vector<tinygltf::BufferView> buffer_views, uint32 buffer_index, eastl::vector<void*> buffers_data, uint32* buffer_size = nullptr, char** buffer_name = nullptr)
{
    tinygltf::BufferView& buffer = buffer_views[buffer_index];

    uint32 offset = buffer.byteOffset;

    if (buffer_name != nullptr)
    {
        *buffer_name = buffer.name.data();
    }

    if (buffer_size != nullptr)
    {
        *buffer_size = buffer.byteLength;
    }

    uint8* data = (uint8*)buffers_data[buffer.buffer] + offset;

    return data;
}


Raptor::Graphics::BufferHandle                    cube_vb;
Raptor::Graphics::BufferHandle                    cube_ib;
Raptor::Graphics::PipelineHandle                  cube_pipeline;
Raptor::Graphics::BufferHandle                    cube_cb;
Raptor::Graphics::DescriptorSetHandle             cube_rl;
Raptor::Graphics::DescriptorSetLayoutHandle       cube_dsl;

struct UniformData {
    Raptor::Math::mat4s m;
    Raptor::Math::mat4s vp;
    Raptor::Math::vec4s eye;
    Raptor::Math::vec4s light;
};

int main( int argc, char** argv)
{
    debug_print_versions();

    if (argc < 2)
    {
        printf("Usage: %s [path to glTF model]\n", argv[0]);
        return 0;
    }
    
    Raptor::Debug::Log("%s\n", argv[1]);

    using Allocator = eastl::allocator;
    Allocator allocator {};

    Raptor::Graphics::Window window {1920, 1080, "Raptor"};
    Raptor::Graphics::GPUDevice gpu_device {window, allocator};
    Raptor::Core::ResourceManager resource_manager {allocator, nullptr};
    Raptor::Graphics::GPUProfiler gpu_profiler {allocator, 100};
    Raptor::Graphics::Renderer renderer {&gpu_device, &resource_manager, allocator};
    //Raptor::Debug::UI::DebugUI debugUI {window, gpu_device};

    char cwd[Raptor::Core::MAX_FILENAME_LENGTH] {};
    Raptor::Core::CurrentDirectory(cwd);

    char base_path[Raptor::Core::MAX_FILENAME_LENGTH] {};
    memcpy(base_path, argv[1], strlen(argv[1]));
    Raptor::Core::DirectoryFromPath(base_path);

    Raptor::Core::ChangeDirectory(base_path);

    char gltf_file[Raptor::Core::MAX_FILENAME_LENGTH] {};
    memcpy(gltf_file, argv[1], strlen(argv[1]));
    Raptor::Core::FilenameFromPath(gltf_file);

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    
    loader.LoadASCIIFromFile(&model, &err, &warn, gltf_file);

    eastl::vector<Raptor::Graphics::TextureResource> images(model.images.size(), allocator);
    for (uint32 i = 0; i < model.images.size(); i++)
    {
        tinygltf::Image& image = model.images[i];
        Raptor::Graphics::TextureResource* tr = renderer.CreateTexture(image.uri.data(), image.uri.data());
        ASSERT(tr != nullptr);

        images[i] = *tr;
    }

    eastl::vector<Raptor::Graphics::SamplerResource> samplers(model.samplers.size(), allocator);
    for (uint32 i = 0; i < model.samplers.size(); i++)
    {
        tinygltf::Sampler& sampler = model.samplers[i];

        char name[64];
        snprintf(name, 64, "Sampler_%u", i);

        Raptor::Graphics::CreateSamplerParams params {};
        params.min_filter = (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR || sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        params.mag_filter = (sampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR || sampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        params.name = name;

        Raptor::Graphics::SamplerResource* sr = renderer.CreateSampler(params);
        ASSERT(sr != nullptr);

        samplers[i] = *sr;
    }

    eastl::vector<void*> buffers_data(model.buffers.size(), allocator);
    for (uint32 i = 0; i < model.buffers.size(); i++)
    {
        tinygltf::Buffer& buffer = model.buffers[i];

        Raptor::Core::FileReadResult buffer_data = Raptor::Core::FileReadBinary(buffer.uri.data(), &allocator);
        buffers_data[i] = buffer_data.data;
    }

    eastl::vector<Raptor::Graphics::BufferResource> buffers(model.bufferViews.size(), allocator);
    for (uint32 i = 0; i < model.bufferViews.size(); i++)
    {
        char* buffer_name = nullptr;
        uint32 buffer_size = 0;
        uint8* data = GetBufferData(model.bufferViews, i, buffers_data, &buffer_size, &buffer_name);

        VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (buffer_name == nullptr)
        {
            snprintf(buffer_name, 64, "Buffer_%u", i);
        }
        else
        {
            snprintf(buffer_name, 64, "%s_%u", buffer_name, i);
        }

        Raptor::Graphics::BufferResource* br = renderer.CreateBuffer(Raptor::Graphics::ResourceUsageType::Immutable, flags, buffer_size, data, buffer_name);
        ASSERT(br != nullptr);

        buffers[i] = *br;
    }

    Raptor::Core::ChangeDirectory(cwd);

    eastl::vector<Raptor::Graphics::MeshDraw*> mesh_draws(model.meshes.size(), allocator);
    eastl::vector<Raptor::Graphics::BufferHandle> custom_mesh_buffers(8, allocator);

    Raptor::Math::vec4s dummy_data[3];
    Raptor::Graphics::CreateBufferParams buffer_params{};
    buffer_params.usage = Raptor::Graphics::ResourceUsageType::Immutable;
    buffer_params.flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_params.size = sizeof(Raptor::Math::vec4s) * 3;
    buffer_params.data = dummy_data;
    buffer_params.name = "Dummy_Attribute_Buffer";

    Raptor::Graphics::BufferHandle dummy_attribute_buffer = gpu_device.CreateBuffer(buffer_params);

    {
        Raptor::Graphics::CreatePipelineParams pipeline_params;

        // position
        pipeline_params.vertex_input.AddVertexAttribute({0, 0, 0, Raptor::Graphics::VertexComponentFormat::Float3});
        pipeline_params.vertex_input.AddVertexStream({0, 12, Raptor::Graphics::VertexInputRate::PerVertex});
        
        // tangent
        pipeline_params.vertex_input.AddVertexAttribute({1, 1, 0, Raptor::Graphics::VertexComponentFormat::Float4});
        pipeline_params.vertex_input.AddVertexStream({1, 16, Raptor::Graphics::VertexInputRate::PerVertex});
          
        // normal
        pipeline_params.vertex_input.AddVertexAttribute({2, 2, 0, Raptor::Graphics::VertexComponentFormat::Float3});
        pipeline_params.vertex_input.AddVertexStream({2, 12, Raptor::Graphics::VertexInputRate::PerVertex});
       
        // texcoord
        pipeline_params.vertex_input.AddVertexAttribute({3, 3, 0, Raptor::Graphics::VertexComponentFormat::Float2});
        pipeline_params.vertex_input.AddVertexStream({3, 8, Raptor::Graphics::VertexInputRate::PerVertex});
       
        // render pass
        pipeline_params.render_pass = gpu_device.GetSwapchainOutput();

        // depth
        pipeline_params.depth_stencil.SetDepth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

        // shader state
        const char* vs_code = R"FOO(#version 450
uint MaterialFeatures_ColorTexture     = 1 << 0;
uint MaterialFeatures_NormalTexture    = 1 << 1;
uint MaterialFeatures_RoughnessTexture = 1 << 2;
uint MaterialFeatures_OcclusionTexture = 1 << 3;
uint MaterialFeatures_EmissiveTexture =  1 << 4;
uint MaterialFeatures_TangentVertexAttribute = 1 << 5;
uint MaterialFeatures_TexcoordVertexAttribute = 1 << 6;

layout(std140, binding = 0) uniform LocalConstants {
    mat4 m;
    mat4 vp;
    vec4 eye;
    vec4 light;
};

layout(std140, binding = 1) uniform MaterialConstant {
    vec4 base_color_factor;
    mat4 model;
    mat4 model_inv;

    vec3  emissive_factor;
    float metallic_factor;

    float roughness_factor;
    float occlusion_factor;
    uint  flags;
};

layout(location=0) in vec3 position;
layout(location=1) in vec4 tangent;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texCoord0;

layout (location = 0) out vec2 vTexcoord0;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec4 vTangent;
layout (location = 3) out vec4 vPosition;

void main() {
    gl_Position = vp * m * model * vec4(position, 1);
    vPosition = m * model * vec4(position, 1.0);

    if ( ( flags & MaterialFeatures_TexcoordVertexAttribute ) != 0 ) {
        vTexcoord0 = texCoord0;
    }
    vNormal = mat3( model_inv ) * normal;

    if ( ( flags & MaterialFeatures_TangentVertexAttribute ) != 0 ) {
        vTangent = tangent;
    }
}
)FOO";

        const char* fs_code = R"FOO(#version 450
uint MaterialFeatures_ColorTexture     = 1 << 0;
uint MaterialFeatures_NormalTexture    = 1 << 1;
uint MaterialFeatures_RoughnessTexture = 1 << 2;
uint MaterialFeatures_OcclusionTexture = 1 << 3;
uint MaterialFeatures_EmissiveTexture =  1 << 4;
uint MaterialFeatures_TangentVertexAttribute = 1 << 5;
uint MaterialFeatures_TexcoordVertexAttribute = 1 << 6;

layout(std140, binding = 0) uniform LocalConstants {
    mat4 m;
    mat4 vp;
    vec4 eye;
    vec4 light;
};

layout(std140, binding = 1) uniform MaterialConstant {
    vec4 base_color_factor;
    mat4 model;
    mat4 model_inv;

    vec3  emissive_factor;
    float metallic_factor;

    float roughness_factor;
    float occlusion_factor;
    uint  flags;
};

layout (binding = 2) uniform sampler2D diffuseTexture;
layout (binding = 3) uniform sampler2D roughnessMetalnessTexture;
layout (binding = 4) uniform sampler2D occlusionTexture;
layout (binding = 5) uniform sampler2D emissiveTexture;
layout (binding = 6) uniform sampler2D normalTexture;

layout (location = 0) in vec2 vTexcoord0;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec4 vTangent;
layout (location = 3) in vec4 vPosition;

layout (location = 0) out vec4 frag_color;

#define PI 3.1415926538

vec3 decode_srgb( vec3 c ) {
    vec3 result;
    if ( c.r <= 0.04045) {
        result.r = c.r / 12.92;
    } else {
        result.r = pow( ( c.r + 0.055 ) / 1.055, 2.4 );
    }

    if ( c.g <= 0.04045) {
        result.g = c.g / 12.92;
    } else {
        result.g = pow( ( c.g + 0.055 ) / 1.055, 2.4 );
    }

    if ( c.b <= 0.04045) {
        result.b = c.b / 12.92;
    } else {
        result.b = pow( ( c.b + 0.055 ) / 1.055, 2.4 );
    }

    return clamp( result, 0.0, 1.0 );
}

vec3 encode_srgb( vec3 c ) {
    vec3 result;
    if ( c.r <= 0.0031308) {
        result.r = c.r * 12.92;
    } else {
        result.r = 1.055 * pow( c.r, 1.0 / 2.4 ) - 0.055;
    }

    if ( c.g <= 0.0031308) {
        result.g = c.g * 12.92;
    } else {
        result.g = 1.055 * pow( c.g, 1.0 / 2.4 ) - 0.055;
    }

    if ( c.b <= 0.0031308) {
        result.b = c.b * 12.92;
    } else {
        result.b = 1.055 * pow( c.b, 1.0 / 2.4 ) - 0.055;
    }

    return clamp( result, 0.0, 1.0 );
}

float heaviside( float v ) {
    if ( v > 0.0 ) return 1.0;
    else return 0.0;
}

void main() {

    mat3 TBN = mat3( 1.0 );

    if ( ( flags & MaterialFeatures_TangentVertexAttribute ) != 0 ) {
        vec3 tangent = normalize( vTangent.xyz );
        vec3 bitangent = cross( normalize( vNormal ), tangent ) * vTangent.w;

        TBN = mat3(
            tangent,
            bitangent,
            normalize( vNormal )
        );
    }
    else {
        // NOTE(marco): taken from https://community.khronos.org/t/computing-the-tangent-space-in-the-fragment-shader/52861
        vec3 Q1 = dFdx( vPosition.xyz );
        vec3 Q2 = dFdy( vPosition.xyz );
        vec2 st1 = dFdx( vTexcoord0 );
        vec2 st2 = dFdy( vTexcoord0 );

        vec3 T = normalize(  Q1 * st2.t - Q2 * st1.t );
        vec3 B = normalize( -Q1 * st2.s + Q2 * st1.s );

        // the transpose of texture-to-eye space matrix
        TBN = mat3(
            T,
            B,
            normalize( vNormal )
        );
    }

    // vec3 V = normalize(eye.xyz - vPosition.xyz);
    // vec3 L = normalize(light.xyz - vPosition.xyz);
    // vec3 N = normalize(vNormal);
    // vec3 H = normalize(L + V);

    vec3 V = normalize( eye.xyz - vPosition.xyz );
    vec3 L = normalize( light.xyz - vPosition.xyz );
    // NOTE(marco): normal textures are encoded to [0, 1] but need to be mapped to [-1, 1] value
    vec3 N = normalize( vNormal );
    if ( ( flags & MaterialFeatures_NormalTexture ) != 0 ) {
        N = normalize( texture(normalTexture, vTexcoord0).rgb * 2.0 - 1.0 );
        N = normalize( TBN * N );
    }
    vec3 H = normalize( L + V );

    float roughness = roughness_factor;
    float metalness = metallic_factor;

    if ( ( flags & MaterialFeatures_RoughnessTexture ) != 0 ) {
        // Red channel for occlusion value
        // Green channel contains roughness values
        // Blue channel contains metalness
        vec4 rm = texture(roughnessMetalnessTexture, vTexcoord0);

        roughness *= rm.g;
        metalness *= rm.b;
    }

    float ao = 1.0f;
    if ( ( flags & MaterialFeatures_OcclusionTexture ) != 0 ) {
        ao = texture(occlusionTexture, vTexcoord0).r;
    }

    float alpha = pow(roughness, 2.0);

    vec4 base_colour = base_color_factor;
    if ( ( flags & MaterialFeatures_ColorTexture ) != 0 ) {
        vec4 albedo = texture( diffuseTexture, vTexcoord0 );
        base_colour.rgb *= decode_srgb( albedo.rgb );
        base_colour.a *= albedo.a;
    }

    vec3 emissive = vec3( 0 );
    if ( ( flags & MaterialFeatures_EmissiveTexture ) != 0 ) {
        vec4 e = texture(emissiveTexture, vTexcoord0);

        emissive += decode_srgb( e.rgb ) * emissive_factor;
    }

    // https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#specular-brdf
    float NdotH = dot(N, H);
    float alpha_squared = alpha * alpha;
    float d_denom = ( NdotH * NdotH ) * ( alpha_squared - 1.0 ) + 1.0;
    float distribution = ( alpha_squared * heaviside( NdotH ) ) / ( PI * d_denom * d_denom );

    float NdotL = clamp( dot(N, L), 0, 1 );

    if ( NdotL > 1e-5 ) {
        float NdotV = dot(N, V);
        float HdotL = dot(H, L);
        float HdotV = dot(H, V);

        float visibility = ( heaviside( HdotL ) / ( abs( NdotL ) + sqrt( alpha_squared + ( 1.0 - alpha_squared ) * ( NdotL * NdotL ) ) ) ) * ( heaviside( HdotV ) / ( abs( NdotV ) + sqrt( alpha_squared + ( 1.0 - alpha_squared ) * ( NdotV * NdotV ) ) ) );

        float specular_brdf = visibility * distribution;

        vec3 diffuse_brdf = (1 / PI) * base_colour.rgb;

        // NOTE(marco): f0 in the formula notation refers to the base colour here
        vec3 conductor_fresnel = specular_brdf * ( base_colour.rgb + ( 1.0 - base_colour.rgb ) * pow( 1.0 - abs( HdotV ), 5 ) );

        // NOTE(marco): f0 in the formula notation refers to the value derived from ior = 1.5
        float f0 = 0.04; // pow( ( 1 - ior ) / ( 1 + ior ), 2 )
        float fr = f0 + ( 1 - f0 ) * pow(1 - abs( HdotV ), 5 );
        vec3 fresnel_mix = mix( diffuse_brdf, vec3( specular_brdf ), fr );

        vec3 material_colour = mix( fresnel_mix, conductor_fresnel, metalness );

        material_colour = emissive + mix( material_colour, material_colour * ao, occlusion_factor);

        frag_color = vec4( encode_srgb( material_colour ), base_colour.a );
    } else {
        frag_color = vec4( base_colour.rgb * 0.1, base_colour.a );
    }
}
)FOO";

        pipeline_params.shaders.SetName("Cube").AddStage(vs_code, (uint32)strlen(vs_code), VK_SHADER_STAGE_VERTEX_BIT).AddStage(fs_code, (uint32)strlen(fs_code), VK_SHADER_STAGE_FRAGMENT_BIT);

        // descriptor set layout
        Raptor::Graphics::CreateDescriptorSetLayoutParams cube_rll_params {};
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, "LocalConstants"});
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, "MaterialConstant"});
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, 1, "diffuseTexture"});
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, 1, "roughnessMetalnessTexture"});
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 1, "roughnessMetalnessTexture"});
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 1, "emissiveTexture"});
        cube_rll_params.AddBinding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 1, "occlusionTexture"});

        cube_dsl = gpu_device.CreateDescriptorSetLayout(cube_rll_params);
        pipeline_params.AddDescriptorSetLayout(cube_dsl);

        cube_pipeline = gpu_device.CreatePipeline(pipeline_params);

        Raptor::Graphics::CreateBufferParams buffer_params {};
        buffer_params.usage = Raptor::Graphics::ResourceUsageType::Dynamic;
        buffer_params.flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_params.size = sizeof(UniformData);
        buffer_params.name = "cube_cb";
        cube_cb = gpu_device.CreateBuffer(buffer_params);

        tinygltf::Scene& root_gltf_scene = model.scenes[model.defaultScene];

        eastl::vector<int32> node_parents(model.nodes.size(), allocator);
        eastl::vector<uint32> node_stack(allocator);
        eastl::vector<Raptor::Math::mat4s> node_matrix(model.nodes.size(), allocator);

        for (uint32 node_index = 0; node_index < root_gltf_scene.nodes.size(); ++node_index)
        {
            uint32 root_node = root_gltf_scene.nodes[node_index];
            node_parents[root_node] = -1;
            node_stack.push_back(root_node);
        }

        while (node_stack.size() > 0)
        {
            uint32 node_index = node_stack.back();
            node_stack.pop_back();
            tinygltf::Node& node = model.nodes[node_index];

            Raptor::Math::mat4f local_matrix{};

            if (node.matrix.size() > 0)
            {
                for (uint32 idx = 0; idx < 16; idx++)
                {
                    local_matrix.i[idx] = (float)node.matrix[idx];
                }
            }
            else
            {
                Raptor::Math::vec3f node_scale {1.f, 1.f, 1.f};
                if (node.scale.size() > 0)
                {
                    ASSERT(node.scale.size() == 3);
                    node_scale = Raptor::Math::vec3f(node.scale[0], node.scale[1], node.scale[2]);
                }

                Raptor::Math::vec3f node_translation {0.f, 0.f, 0.f};
                if (node.translation.size() > 0)
                {
                    ASSERT(node.translation.size() == 3);
                    node_translation = Raptor::Math::vec3f(node.translation[0], node.translation[1], node.translation[2]);
                }

                Raptor::Math::vec4f node_rotation {0.f, 0.f, 0.f, 1.f};
                if (node.rotation.size() > 0)
                {
                    ASSERT(node.rotation.size() == 4);
                    node_rotation = Raptor::Math::vec4f(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
                }

                Raptor::Math::Transform transform {};
                transform.translation = node_translation;
                transform.scale = node_scale;
                transform.rotation = node_rotation;

                local_matrix = transform.CalcMatrix();

            }
        }


    }



    while (!window.ShouldClose())
    {
        window.PollEvents();
        //debugUI.Update();
        //debugUI.Render();
    }

    return 0;
}

