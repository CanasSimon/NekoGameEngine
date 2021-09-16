#include "ray/draw_system.h"

#include "engine/resource_locations.h"
#include "math/transform.h"

#include "vk/vk_resources.h"

namespace neko
{
DrawSystem::DrawSystem()
{
    BasicEngine::GetInstance()->RegisterSystem(textureManager_);
    BasicEngine::GetInstance()->RegisterSystem(modelManager_);
    BasicEngine::GetInstance()->RegisterSystem(particlesSystem_);
    BasicEngine::GetInstance()->RegisterOnDrawUi(*this);
}

void DrawSystem::Init()
{
	Camera3D camera;
	camera.Init();
	camera.position = Vec3f::forward * 5.0f + Vec3f::up * 2.0f;
	camera.WorldLookAt(Vec3f::zero);
	camera_.SetCameras(camera);
	sdl::MultiCameraLocator::provide(&camera_);

	const Configuration& config = BasicEngine::GetInstance()->GetConfig();
	planeModelId_ = modelManager_.LoadModel(GetModelsFolderPath() + "plane/plane.obj");
	cubeModelId_  = modelManager_.LoadModel(GetModelsFolderPath() + "cube/cube.obj");

	auto& cmdBuffer = vk::VkResources::Inst->modelCommandBuffers[0];
    cmdBuffer.AddModelInstanceIndex(planeModelId_);
    cmdBuffer.AddModelInstanceIndex(cubeModelId_);

    DirectionalLight::Instance = &dirLight_;

	ColorGradient::ColorMark mark1;
	mark1.position = 0.5f;
	mark1.color    = Color::white;

	ColorGradient::ColorMark mark2;
	mark2.position = 1.0f;
	mark2.color    = Color::clear;

	vk::ParticleSystem particleSystem1;
	particleSystem1.position     = Vec3f::up * 2.0f;
	particleSystem1.minSpeed     = 0.2f;
	particleSystem1.maxSpeed     = 0.5f;
	particleSystem1.rateOverTime = 10'000.0f;
	particleSystem1.maxParticles = 10'000;
	particleSystem1.lifetime     = 20.0f;
	particleSystem1.maxLifetime  = 20.0f;
	particleSystem1.materialID   = materialManager_.AddNewMaterial(vk::MaterialType::PARTICLE);
	particleSystem1.colorOverLifetime.SetMarks({mark1, mark2});

	ColorGradient::ColorMark mark3;
	mark3.position = 0.5f;
	mark3.color    = Color::red;

	vk::ParticleSystem particleSystem2;
	particleSystem2.position     = Vec3f::up + Vec3f::right * 5.0f;
	particleSystem2.minSpeed     = 0.2f;
	particleSystem2.maxSpeed     = 2.0f;
	particleSystem2.rateOverTime = 3.0f;
	particleSystem2.materialID   = materialManager_.AddNewMaterial(vk::MaterialType::PARTICLE);
	particleSystem2.colorOverLifetime.SetMarks({mark3, mark2});

	const vk::ResourceHash texId = textureManager_.AddTexture(GetSpritesFolderPath() + "white.png");
	auto& particleMat1           = materialManager_.GetParticleMaterial(particleSystem1.materialID);
	auto& particleMat2           = materialManager_.GetParticleMaterial(particleSystem2.materialID);
	particleMat1.SetDiffuse(*textureManager_.GetTexture(texId));
	particleMat2.SetDiffuse(*textureManager_.GetTexture(texId));

	particlesSystem_.AddSystem(particleSystem1);
	//particlesSystem_.AddSystem(particleSystem2);
}

void DrawSystem::Update(seconds dt)
{
	dt_ = dt;
    timeSinceUpdate_ += dt.count();
	camera_.Update(dt);

    if (timeSinceUpdate_ > 0.5f)
    {
        timeSinceUpdate_ = 0.0f;
        fpsCache_ = 1.0f / dt_.count();
		std::cout << "FPS: " << 1.0f / dt_.count() << '\n';
    }

	RendererLocator::get().Render(this);
}

void DrawSystem::Render()
{
    if (!modelManager_.IsLoaded(cubeModelId_) || !modelManager_.IsLoaded(planeModelId_)) return;

	auto& cmdBuffer = vk::VkResources::Inst->modelCommandBuffers[0];

	const Mat4f& modelMat1 = Transform3d::Scale(Mat4f::Identity, Vec3f(20.0f));
	cmdBuffer.AddMatrix(PLANE, modelMat1);

	const Mat4f& modelMat2 = Transform3d::Transform(Vec3f::up * 2.0f, EulerAngles(), Vec3f(0.2f));
	cmdBuffer.AddMatrix(CUBE, modelMat2);
}

void DrawSystem::Destroy() { camera_.Destroy(); }

void DrawSystem::OnEvent(const SDL_Event& event) { camera_.OnEvent(event); }

void DrawSystem::DrawImGui()
{
	using namespace ImGui;
	ImGuiIO io = GetIO();

	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = GetMainViewport();
	SetNextWindowPos(viewport->WorkPos);
	SetNextWindowSize(viewport->WorkSize);
	SetNextWindowViewport(viewport->ID);
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
	               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	Begin("Dockspace", reinterpret_cast<bool*>(true), windowFlags);
	{
		PopStyleVar();
		PopStyleVar(2);

		const ImGuiID dockspaceId = GetID("Dockspace");
		DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), kDockspaceFlags);

		//Editor Menu
		BeginMenuBar();
		{
			if (timeSinceUpdate_ > 0.5f)
            {
				timeSinceUpdate_ = 0.0f;
				fpsCache_ = 1.0f / dt_.count();
			}

			const auto fpsText  = fmt::format("{:.0f} FPS", fpsCache_);
			const float spacing = GetStyle().ItemSpacing.x + GetStyle().FramePadding.x;
			const float nextPos = GetWindowWidth() - CalcTextSize(fpsText.c_str()).x - spacing;
			SetCursorPosX(nextPos);
			Text("%s", fpsText.c_str());

			EndMenuBar();
		}

		End();
	}
}
}    // namespace neko
