#include "DefaultSceneLayer.h"
#pragma region Includes


// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/Textures/Texture2DArray.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/Light.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"

#include "Gameplay/Components/CharacterController.h"
#include "Gameplay/Components/EnemyController.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Application/Layers/ImGuiDebugLayer.h"
#include "Application/Windows/DebugWindow.h"
#include "Gameplay/Components/ShadowCamera.h"
#include "Gameplay/Components/ShipMoveBehaviour.h"
#pragma endregion

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
#pragma region Shaders

		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");

#pragma endregion
#pragma region LoadMeshes

		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Models/Monkey.obj");
		MeshResource::Sptr darkBrickMesh = ResourceManager::CreateAsset<MeshResource>("Models/DarkBricks.obj");
		MeshResource::Sptr lightBrickMesh = ResourceManager::CreateAsset<MeshResource>("Models/LightBricks.obj");
		MeshResource::Sptr platformMesh = ResourceManager::CreateAsset<MeshResource>("Models/Platform.obj");

		//Link
		MeshResource::Sptr m_clHands = ResourceManager::CreateAsset<MeshResource>("Models/Link/clHands.obj");
		MeshResource::Sptr m_ears = ResourceManager::CreateAsset<MeshResource>("Models/Link/ears.obj");
		MeshResource::Sptr m_hair = ResourceManager::CreateAsset<MeshResource>("Models/Link/hair.obj");
		MeshResource::Sptr m_headh = ResourceManager::CreateAsset<MeshResource>("Models/Link/headh.obj");
		MeshResource::Sptr m_headl = ResourceManager::CreateAsset<MeshResource>("Models/Link/headL.obj");
		MeshResource::Sptr m_shield = ResourceManager::CreateAsset<MeshResource>("Models/Link/Shield.obj");
		MeshResource::Sptr m_sword = ResourceManager::CreateAsset<MeshResource>("Models/Link/Sword.obj");

#pragma endregion
#pragma region LoadTextures
#pragma region Normal Map
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;
		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

#pragma endregion

		// Load in some textures
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    boxTex = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    smileyTex = ResourceManager::CreateAsset<Texture2D>("textures/light_projection.png");
		Texture2D::Sptr    lightBrickTex = ResourceManager::CreateAsset<Texture2D>("textures/lightbrick.png");
		lightBrickTex->SetMagFilter(MagFilter::Nearest);
		lightBrickTex->SetMinFilter(MinFilter::NearestMipNearest);
		Texture2D::Sptr    darkBrickTex = ResourceManager::CreateAsset<Texture2D>("textures/darkbrick.png");
		darkBrickTex->SetMagFilter(MagFilter::Nearest);
		darkBrickTex->SetMinFilter(MinFilter::NearestMipNearest);

		Texture2DArray::Sptr particleTex = ResourceManager::CreateAsset<Texture2DArray>("textures/particles.png", 2, 2);

		//Link
		Texture2D::Sptr    t_clHands = ResourceManager::CreateAsset<Texture2D>("Models/Link/ClHands.png");
		Texture2D::Sptr    t_ears = ResourceManager::CreateAsset<Texture2D>("Models/Link/ears.png");
		Texture2D::Sptr    t_hair = ResourceManager::CreateAsset<Texture2D>("Models/Link/hair.png");
		Texture2D::Sptr    t_headh = ResourceManager::CreateAsset<Texture2D>("Models/Link/headh.png");
		Texture2D::Sptr    t_headl = ResourceManager::CreateAsset<Texture2D>("Models/Link/headl.png");
		Texture2D::Sptr    t_shield = ResourceManager::CreateAsset<Texture2D>("Models/Link/shield.png");
		Texture2D::Sptr    n_shield = ResourceManager::CreateAsset<Texture2D>("Models/Link/shieldn.png");
		Texture2D::Sptr    t_sword = ResourceManager::CreateAsset<Texture2D>("Models/Link/sword.png");
		Texture2D::Sptr    n_sword = ResourceManager::CreateAsset<Texture2D>("Models/Link/swordn.png");

#pragma endregion

		//DebugWindow::Sptr debugWindow = app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>();
		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>();  

#pragma region Skybox Shit

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" } 
		});
		  
		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

#pragma endregion
#pragma region LoadLUTS

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Loading in a color lookup table
		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");

		// Configure the color correction LUT
		scene->SetColorLUT(lut);

#pragma endregion
#pragma region Materials

		// Create our materials
		

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->Set("u_Material.AlbedoMap", monkeyTex);
			monkeyMaterial->Set("u_Material.NormalMap", normalMapDefault);
			monkeyMaterial->Set("u_Material.Shininess", 0.5f);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			boxMaterial->Name = "BoxMat";
			boxMaterial->Set("u_Material.AlbedoMap", smileyTex);
			boxMaterial->Set("u_Material.NormalMap", normalMapDefault);
			boxMaterial->Set("u_Material.Shininess", 0.5f);
		}

		Material::Sptr lightBrickMat = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			lightBrickMat->Name = "lightBrickMat";
			lightBrickMat->Set("u_Material.AlbedoMap", lightBrickTex);
			lightBrickMat->Set("u_Material.NormalMap", normalMapDefault);
			lightBrickMat->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr darkBrickMat = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			darkBrickMat->Name = "darkBrickMat";
			darkBrickMat->Set("u_Material.AlbedoMap", darkBrickTex);
			darkBrickMat->Set("u_Material.NormalMap", normalMapDefault);
			darkBrickMat->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr matClHands = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matClHands->Name = "clhands";
			matClHands->Set("u_Material.AlbedoMap", t_clHands);
			matClHands->Set("u_Material.NormalMap", normalMapDefault);
			matClHands->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr matEars = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matEars->Name = "ears";
			matEars->Set("u_Material.AlbedoMap", t_ears);
			matEars->Set("u_Material.NormalMap", normalMapDefault);
			matEars->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr matHair = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matHair->Name = "hair";
			matHair->Set("u_Material.AlbedoMap", t_hair);
			matHair->Set("u_Material.NormalMap", normalMapDefault);
			matHair->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr matHeadh = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matHeadh->Name = "eyes";
			matHeadh->Set("u_Material.AlbedoMap", t_headh);
			matHeadh->Set("u_Material.NormalMap", normalMapDefault);
			matHeadh->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr matheadl = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matheadl->Name = "mouth";
			matheadl->Set("u_Material.AlbedoMap", t_headl);
			matheadl->Set("u_Material.NormalMap", normalMapDefault);
			matheadl->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr matshield = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matshield->Name = "shyield";
			matshield->Set("u_Material.AlbedoMap", t_shield);
			matshield->Set("u_Material.NormalMap", n_shield);
			matshield->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr matSword = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			matSword->Name = "Sword";
			matSword->Set("u_Material.AlbedoMap", t_sword);
			matSword->Set("u_Material.NormalMap", n_sword);
			matSword->Set("u_Material.Shininess", 1.0f);
		}

#pragma endregion
#pragma region Lighting
		// Create some lights for our scene
		GameObject::Sptr light = scene->CreateGameObject("Light");
		light->SetPostion({ 0,-7.3f,7.8f });

		Light::Sptr lightComponent = light->Add<Light>();
		lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
		lightComponent->SetRadius(4.5f);
		lightComponent->SetIntensity(16.7f);
#pragma endregion

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

#pragma region GameObjects

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			scene->MainCamera->SetOrthoEnabled(true);
			scene->MainCamera->SetOrthoVerticalScale(5);
			camera->SetPostion({ 0, -3.7f, 1.8f });
			camera->SetRotation({ 90,0,0 });
			scene->MainCamera->FocalDepth = 2.1f;
			scene->MainCamera->LensDepth = 0.65f;
			scene->MainCamera->Aperture = 14.6f;

			//camera->Add<SimpleCameraControl>();

			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}


		 //Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			//RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			//renderer->SetMesh(tiledMesh);
			//renderer->SetMaterial(boxMaterial);

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-1 });
		}

		

		GameObject::Sptr Link = scene->CreateGameObject("Link");
		{
			// Set position in the scene
			Link->SetPostion(glm::vec3(1.5f, 0.0f, 1.0f));
			Link->SetScale({ 0.5,0.5,0.5 });


			// Create and attach a renderer for the monkey
			/*RenderComponent::Sptr renderer = Link->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);*/

			RigidBody::Sptr physics = Link->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(BoxCollider::Create(glm::vec3(1.f)));

			//player movement
			CharacterController::Sptr movement = Link->Add<CharacterController>();
		}

#pragma region LinkModel
		GameObject::Sptr o_clhands = scene->CreateGameObject("clhands");
		{
			// Set position in the scene
			o_clhands->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_clhands->SetScale({ 1,1,1 });
			o_clhands->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_clhands->Add<RenderComponent>();
			renderer->SetMesh(m_clHands);
			renderer->SetMaterial(matClHands);
			Link->AddChild(o_clhands);
		}

		GameObject::Sptr o_ears = scene->CreateGameObject("ears");
		{
			// Set position in the scene
			o_ears->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_ears->SetScale({ 1,1,1 });
			o_ears->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_ears->Add<RenderComponent>();
			renderer->SetMesh(m_ears);
			renderer->SetMaterial(matEars);
			Link->AddChild(o_ears);
		}

		GameObject::Sptr o_hair = scene->CreateGameObject("hair");
		{
			// Set position in the scene
			o_hair->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_hair->SetScale({ 1,1,1 });
			o_hair->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_hair->Add<RenderComponent>();
			renderer->SetMesh(m_hair);
			renderer->SetMaterial(matHair);
			Link->AddChild(o_hair);
		}

		GameObject::Sptr o_headh = scene->CreateGameObject("eyes");
		{
			// Set position in the scene
			o_headh->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_headh->SetScale({ 1,1,1 });
			o_headh->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_headh->Add<RenderComponent>();
			renderer->SetMesh(m_headh);
			renderer->SetMaterial(matHeadh);
			Link->AddChild(o_headh);
		}

		GameObject::Sptr o_headl = scene->CreateGameObject("mouth");
		{
			// Set position in the scene
			o_headl->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_headl->SetScale({ 1,1,1 });
			o_headl->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_headl->Add<RenderComponent>();
			renderer->SetMesh(m_headl);
			renderer->SetMaterial(matheadl);
			Link->AddChild(o_headl);
		}

		GameObject::Sptr o_shield = scene->CreateGameObject("shield");
		{
			// Set position in the scene
			o_shield->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_shield->SetScale({ 1,1,1 });
			o_shield->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_shield->Add<RenderComponent>();
			renderer->SetMesh(m_shield);
			renderer->SetMaterial(matshield);
			Link->AddChild(o_shield);
		}

		GameObject::Sptr o_sword = scene->CreateGameObject("sword");
		{
			// Set position in the scene
			o_sword->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));
			o_sword->SetScale({ 1,1,1 });
			o_sword->SetRotation({ 90,0,90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = o_sword->Add<RenderComponent>();
			renderer->SetMesh(m_sword);
			renderer->SetMaterial(matSword);
			Link->AddChild(o_sword);
		}

#pragma endregion



		GameObject::Sptr Knight = scene->CreateGameObject("Monkey 2");
		{
			// Set position in the scene
			Knight->SetPostion(glm::vec3(4.5f, 0.0f, 1.0f));
			Knight->SetScale({ 0.5,0.5,0.5 });


			// Create and attach a renderer for the Knight
			RenderComponent::Sptr renderer = Knight->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);

			RigidBody::Sptr physics = Knight->Add<RigidBody>(RigidBodyType::Dynamic);
			BoxCollider::Sptr collider = BoxCollider::Create();
			collider->SetPosition({ 0,0,-0.6 });
			physics->AddCollider(collider);

			//enemy movement
			EnemyController::Sptr movement = Knight->Add<EnemyController>();
		}

		GameObject::Sptr lightBricks = scene->CreateGameObject("Stage");
		{
			// Set position in the scene
			lightBricks->SetPostion(glm::vec3(0.f, 0.0f, 0.92f));
			lightBricks->SetScale({ 0.1,0.1,0.1 });
			lightBricks->SetRotation({ 90,0,-90 });


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = lightBricks->Add<RenderComponent>();
			renderer->SetMesh(lightBrickMesh);
			renderer->SetMaterial(lightBrickMat);
		}

		GameObject::Sptr darkBricks = scene->CreateGameObject("Stage Bricks (Dark)");
		{
			// Set position in the scene
			darkBricks->SetPostion(glm::vec3(0.f, 0.0f, 0.0f));


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = darkBricks->Add<RenderComponent>();
			renderer->SetMesh(darkBrickMesh);
			renderer->SetMaterial(darkBrickMat);

			lightBricks->AddChild(darkBricks);
		}

		GameObject::Sptr platform = scene->CreateGameObject("Stage Platform");
		{
			// Set position in the scene
			platform->SetPostion(glm::vec3(0.f, 8.0f, -16.0f));


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = platform->Add<RenderComponent>();
			renderer->SetMesh(platformMesh);
			renderer->SetMaterial(lightBrickMat);

			lightBricks->AddChild(platform);
		}

		{
//GameObject::Sptr ship = scene->CreateGameObject("Fenrir");
		//{
		//	// Set position in the scene
		//	ship->SetPostion(glm::vec3(1.5f, 0.0f, 4.0f));
		//	ship->SetScale(glm::vec3(0.1f));

		//	// Create and attach a renderer for the monkey
		//	RenderComponent::Sptr renderer = ship->Add<RenderComponent>();
		//	renderer->SetMesh(shipMesh);
		//	renderer->SetMaterial(grey);

		//	GameObject::Sptr particles = scene->CreateGameObject("Particles");
		//	ship->AddChild(particles);
		//	particles->SetPostion({ 0.0f, -7.0f, 0.0f});

		//	ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();
		//	particleManager->Atlas = particleTex;

		//	particleManager->_gravity = glm::vec3(0.0f);

		//	ParticleSystem::ParticleData emitter;
		//	emitter.Type = ParticleType::SphereEmitter;
		//	emitter.TexID = 2;
		//	emitter.Position = glm::vec3(0.0f);
		//	emitter.Color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		//	emitter.Lifetime = 1.0f / 50.0f;
		//	emitter.SphereEmitterData.Timer = 1.0f / 50.0f;
		//	emitter.SphereEmitterData.Velocity = 0.5f;
		//	emitter.SphereEmitterData.LifeRange = { 1.0f, 3.0f };
		//	emitter.SphereEmitterData.Radius = 0.5f;
		//	emitter.SphereEmitterData.SizeRange = { 0.5f, 1.0f };

		//	ParticleSystem::ParticleData emitter2;
		//	emitter2.Type = ParticleType::SphereEmitter;
		//	emitter2.TexID = 2;
		//	emitter2.Position = glm::vec3(0.0f);
		//	emitter2.Color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
		//	emitter2.Lifetime = 1.0f / 40.0f;
		//	emitter2.SphereEmitterData.Timer = 1.0f / 40.0f;
		//	emitter2.SphereEmitterData.Velocity = 0.1f;
		//	emitter2.SphereEmitterData.LifeRange = { 0.5f, 1.0f };
		//	emitter2.SphereEmitterData.Radius = 0.25f;
		//	emitter2.SphereEmitterData.SizeRange = { 0.25f, 0.5f };

		//	particleManager->AddEmitter(emitter);
		//	particleManager->AddEmitter(emitter2);

		//	ShipMoveBehaviour::Sptr move = ship->Add<ShipMoveBehaviour>();
		//	move->Center = glm::vec3(0.0f, 0.0f, 4.0f);
		//	move->Speed = 180.0f;
		//	move->Radius = 6.0f;
		//}


/*	GameObject::Sptr particles = scene->CreateGameObject("Particles"); 
		{
			particles->SetPostion({ -2.0f, 0.0f, 2.0f });

			ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();  
			particleManager->Atlas = particleTex;

			ParticleSystem::ParticleData emitter;
			emitter.Type = ParticleType::SphereEmitter;
			emitter.TexID = 2;
			emitter.Position = glm::vec3(0.0f);
			emitter.Color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			emitter.Lifetime = 0.0f;
			emitter.SphereEmitterData.Timer = 1.0f / 50.0f;
			emitter.SphereEmitterData.Velocity = 0.5f;
			emitter.SphereEmitterData.LifeRange = { 1.0f, 4.0f };
			emitter.SphereEmitterData.Radius = 1.0f;
			emitter.SphereEmitterData.SizeRange = { 0.5f, 1.5f };

			particleManager->AddEmitter(emitter);
		}*/

		}
		

		

		GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion(glm::vec3(3.0f, 3.0f, 5.0f));
			shadowCaster->LookAt(glm::vec3(0.0f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 100.0f));
		}
#pragma endregion

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
