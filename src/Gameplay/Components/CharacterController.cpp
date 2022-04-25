#include "Gameplay/Components/CharacterController.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/InputEngine.h"

CharacterController::CharacterController() :IComponent(), _velMultiplier(3.f) {

}

void CharacterController::Awake() {
	_body = GetGameObject()->Get<Gameplay::Physics::RigidBody>();
}

void CharacterController::Update(float deltaTime) {
	zpos = InputEngine::GetKeyState(GLFW_KEY_W) == ButtonState::Pressed;
	xpos = InputEngine::GetKeyState(GLFW_KEY_D) == ButtonState::Down;
	xneg = InputEngine::GetKeyState(GLFW_KEY_A) == ButtonState::Down;

	xVelocity = (xpos * _velMultiplier) - (xneg * _velMultiplier);
	zVelocity = (zpos * _velMultiplier)*2.5;

	_body->SetLinearVelocity(glm::vec3(xVelocity, _body->GetLinearVelocity().y, zVelocity + _body->GetLinearVelocity().z));
	_body->SetAngularVelocity(glm::vec3(0.f, 0.f, 0.f));
	GetGameObject()->SetRotation(glm::vec3(0.f, 0.f, 0.f));

}

void CharacterController::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Velocity", &_velMultiplier);
}

nlohmann::json CharacterController::ToJson() const {
	return {
		{ "velocity_multiplier", _velMultiplier }
	};

}

CharacterController::Sptr CharacterController::FromJson(const nlohmann::json& blob) {
	CharacterController::Sptr result = std::make_shared<CharacterController>();
	result->_velMultiplier = blob["velocity_multiplier"];

	return result;
}

