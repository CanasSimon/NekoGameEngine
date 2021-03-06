#include "aer/aer_engine.h"

#include "engine/resource_locations.h"

#ifdef NEKO_VULKAN
#include "vk/vk_resources.h"
#endif

#ifdef NEKO_PROFILE
#include <easy/profiler.h>
#endif

namespace neko::aer
{
DrawSystem::DrawSystem(AerEngine& engine)
   : engine_(engine), cContainer_(engine.GetComponentManagerContainer())
{
	if (engine.GetMode() != ModeEnum::TEST)
	{
		engine.RegisterSystem(camera_);
		engine.RegisterOnEvent(camera_);

#ifdef NEKO_OPENGL
		gizmosRenderer_ = std::make_unique<GizmoRenderer>(&camera_.GetCamera(0));
		engine.RegisterSystem(*gizmosRenderer_);
#endif
		uiManager_ = std::make_unique<UiManager>(engine_);
		engine.RegisterSystem(*uiManager_);
		engine.RegisterOnEvent(*uiManager_);
	}
}

void DrawSystem::Init()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DrawSystem::Init");
#endif
	Camera3D camera;
	camera.position         = Vec3f::forward * 2.0f + Vec3f::up * 2.0f;
	camera.reverseDirection = Vec3f::forward;
	camera.fovY             = degree_t(70.0f);
	camera.nearPlane        = 0.1f;
	camera.farPlane         = 1'000.0f;
	camera_.SetCameras(camera);
	sdl::MultiCameraLocator::provide(&camera_);

#ifdef NEKO_OPENGL
	gizmosRenderer_->SetCamera(&camera_.GetCamera(0));

	// Create Skybox
	preRender_ = Job(
		[this]
		{
			skybox_.Init();
			skyboxShader_.LoadFromFile(
				GetGlShadersFolderPath() + "skybox.vert", GetGlShadersFolderPath() + "skybox.frag");

			std::vector<std::string> skyboxFacesPaths {
				GetSpritesFolderPath() + skyboxFolder_ + "px.png",
				GetSpritesFolderPath() + skyboxFolder_ + "nx.png",
				GetSpritesFolderPath() + skyboxFolder_ + "py.png",
				GetSpritesFolderPath() + skyboxFolder_ + "ny.png",
				GetSpritesFolderPath() + skyboxFolder_ + "pz.png",
				GetSpritesFolderPath() + skyboxFolder_ + "nz.png",
			};

			skyboxTexture_ = gl::LoadCubemap(skyboxFacesPaths, engine_.GetFilesystem());
		});

	RendererLocator::get().AddPreRenderJob(&preRender_);
#endif
}

void DrawSystem::Update(seconds dt)
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DrawSystem::Update");
#endif

	const Camera3D& camera = camera_.GetCamera(0);
	const Vec3f position   = camera.position;
#ifdef NEKO_FMOD
	FMOD_3D_ATTRIBUTES attributes;
	attributes.position = fmod::Vec3ToFmod(Vec3f::zero);
	attributes.forward  = fmod::Vec3ToFmod(Vec3f::forward);
	attributes.up       = fmod::Vec3ToFmod(Vec3f::up);
	engine_.GetFmodEngine().SetAudioListener(attributes);
#endif

	RendererLocator::get().Render(this);
}

void DrawSystem::Render()
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DrawSystem::Render");
#endif

#ifdef NEKO_OPENGL
	gl::Shader& shader = cContainer_.renderManager.GetShader();
	const Vec2i size   = Vec2i(BasicEngine::GetInstance()->GetConfig().windowSize);
	switch (playerNum_)
	{
		case 0:
		case 1:
		{
			camera_.SetAspects(static_cast<float>(size.x), static_cast<float>(size.y));

			camera_.Bind(0, shader);
			glViewport(0, 0, size.x, size.y);
			RenderScene(0);
			break;
		}
		case 2:
		{
			camera_.SetAspects(static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y));

			// Left
			camera_.Bind(0, shader);
			glViewport(0, 0, size.x / 2, size.y);
			RenderScene(0);

			// Right
			camera_.Bind(1, shader);
			glViewport(size.x / 2, 0, size.x / 2, size.y);
			RenderScene(1);
			break;
		}
		case 3:
		{
			camera_.SetAspects(
				static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y) / 2.0f);

			// Top Left
			camera_.Bind(0, shader);
			glViewport(0, size.y / 2, size.x / 2, size.y / 2);
			RenderScene(0);

			// Top Right
			camera_.Bind(1, shader);
			glViewport(size.x / 2, size.y / 2, size.x / 2, size.y / 2);
			RenderScene(1);

			// Bottom Left
			camera_.Bind(2, shader);
			glViewport(0, 0, size.x / 2, size.y / 2);
			RenderScene(2);
			break;
		}
		case 4:
		{
			camera_.SetAspects(
				static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y) / 2.0f);

			// Top Left
			camera_.Bind(0, shader);
			glViewport(0, size.y / 2, size.x / 2, size.y / 2);
			RenderScene(0);

			// Top Right
			camera_.Bind(1, shader);
			glViewport(size.x / 2, size.y / 2, size.x / 2, size.y / 2);
			RenderScene(1);

			// Bottom Left
			camera_.Bind(2, shader);
			glViewport(0, 0, size.x / 2, size.y / 2);
			RenderScene(2);

			// Bottom Right
			camera_.Bind(3, shader);
			glViewport(size.x / 2, 0, size.x / 2, size.y / 2);
			RenderScene(3);
			break;
		}
		default: LogError("Invalid Player number!!"); break;
	}

	uiManager_->Render(playerNum_);
	gizmosRenderer_->Clear();
#elif NEKO_VULKAN
	vk::VkResources::Inst->SetViewportCount(playerNum_);
	const Vec2f size = Vec2f(BasicEngine::GetInstance()->GetConfig().windowSize);
	switch (playerNum_)
	{
		case 1: camera_.SetAspects(size.x, size.y); break;
		case 2: camera_.SetAspects(size.x / 2.0f, size.y); break;

		case 3:
		case 4: camera_.SetAspects(size.x / 2.0f, size.y / 2.0f); break;

		case 0:
		default: LogError("Invalid Player number!!"); break;
	}

	engine_.GetComponentManagerContainer().renderManager.Render();
#endif
}

#ifdef NEKO_OPENGL
void DrawSystem::RenderScene(const std::size_t playerNum)
{
#ifdef NEKO_PROFILE
	EASY_BLOCK("DrawSystem::RenderScene");
#endif

	auto& cManagerContainer = engine_.GetComponentManagerContainer();
	cManagerContainer.renderManager.Render();

	gizmosRenderer_->SetCamera(&camera_.GetCamera(playerNum));
	gizmosRenderer_->Render();

	glCheckError();
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);
	skyboxShader_.Bind();
	skyboxShader_.SetCubemap("tex", skyboxTexture_);
	skybox_.Draw();
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glCheckError();
}
#endif

void DrawSystem::Destroy() {}

void DrawSystem::DrawImGui() {}

void DrawSystem::OnEvent(const SDL_Event& event) { camera_.OnEvent(event); }
}    // namespace neko::aer
