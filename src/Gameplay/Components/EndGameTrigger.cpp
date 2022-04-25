#include "Gameplay/Components/EndGameTrigger.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

EndGameTrigger::EndGameTrigger() :
	IComponent()
{ }
EndGameTrigger::~EndGameTrigger() = default;


void EndGameTrigger::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body)
{
	if (body->GetGameObject()->Name == "Link") {
		(GetGameObject()->GetScene()->FindObjectByName("Feedback Plane"))->Get<RenderComponent>()->SetMaterial(mat);
		(GetGameObject()->GetScene()->FindObjectByName("Feedback Plane"))->SetRotation(glm::vec3(90.f, 0.f, 0.f));
		(GetGameObject()->GetScene()->FindObjectByName("Feedback Plane"))->SetPostion( glm::vec3(0, 1.f, 1.8f)
	}
	
	
}

void EndGameTrigger::OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) {
	
}

void EndGameTrigger::RenderImGui() { }

nlohmann::json EndGameTrigger::ToJson() const {
	return {
		{"feedback_mat", (mat != nullptr) ? mat->GetGUID().str() : "null"},
	};
}

EndGameTrigger::Sptr EndGameTrigger::FromJson(const nlohmann::json& blob) {
	EndGameTrigger::Sptr result = std::make_shared<EndGameTrigger>();
	result->mat = ResourceManager::Get<Gameplay::Material>(Guid(blob["feedback_mat"]));
	return result;
}

void EndGameTrigger::SetMat(Gameplay::Material::Sptr m) {
	mat = m;
}