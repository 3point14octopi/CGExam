#include "NightVision.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

NightVision::NightVision() :
    PostProcessingLayer::Effect(),
    _noise(nullptr),
    _shader(nullptr)
{
    Name = "Night Vision";
    _format = RenderTargetType::ColorRgb8;

    _shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
        { ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
        { ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/NightVision.glsl" }
    });

    _noise = ResourceManager::CreateAsset<Texture2D>("textures/noise_texture.png");

    Enabled = false;
}

NightVision::~NightVision() = default;

void NightVision::Apply(const Framebuffer::Sptr & gBuffer)
{
    _shader->Bind();
    _noise->Bind(3);
    gBuffer->BindAttachment(RenderTargetAttachment::Depth, 1);
    gBuffer->BindAttachment(RenderTargetAttachment::Color1, 2); // The normal buffer
}

void NightVision::RenderImGui()
{
}

NightVision::Sptr NightVision::FromJson(const nlohmann::json & data)
{
    NightVision::Sptr result = std::make_shared<NightVision>();
    result->Enabled = JsonGet(data, "enabled", false);
    return result;
    
}

nlohmann::json NightVision::ToJson() const
{
    return
    {
        { "enabled", Enabled },
    };

}