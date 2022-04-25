#include "Gameplay/Components/EnemyController.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/InputEngine.h"

EnemyController::EnemyController() :IComponent(), _velMultiplier(1.8f) {

}

void EnemyController::Awake() {
	_body = GetGameObject()->Get<Gameplay::Physics::RigidBody>();
}

void EnemyController::Update(float deltaTime) {
	zpos = !(rand()%500);
	if (!movingLeft) {
		xpos = !(rand() % 50);
		movingLeft = (xpos) ? 10 : 0;
	}
	else {
		movingLeft--;
		xpos = true;
	}
	
	if (!movingRight) {
		xneg = !(rand() % 50);
		movingRight = (xneg)? 10 : 0;
	}
	else {
		movingRight--;
		xneg = true;
	}

	xVelocity = ((xpos * _velMultiplier) - (xneg * _velMultiplier)) * 3.f;
	zVelocity = (zpos * _velMultiplier)*2.5;

	_body->SetLinearVelocity(glm::vec3(xVelocity, _body->GetLinearVelocity().y, zVelocity + _body->GetLinearVelocity().z));
	_body->SetAngularVelocity(glm::vec3(0.f, 0.f, 0.f));
	GetGameObject()->SetRotation(glm::vec3(0.f, 0.f, 0.f));

	if (zpos) {
		std::cout << zpos;
	}
}

void EnemyController::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Velocity", &_velMultiplier);
}

nlohmann::json EnemyController::ToJson() const {
	return {
		{ "velocity_multiplier", _velMultiplier }
	};

}

EnemyController::Sptr EnemyController::FromJson(const nlohmann::json& blob) {
	EnemyController::Sptr result = std::make_shared<EnemyController>();
	result->_velMultiplier = blob["velocity_multiplier"];

	return result;
}

