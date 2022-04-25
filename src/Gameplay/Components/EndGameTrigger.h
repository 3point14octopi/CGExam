#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Material.h"

/// <summary>
/// Provides an example behaviour that uses some of the trigger interface to change the material
/// of the game object the component is attached to when entering or leaving a trigger
/// </summary>
class EndGameTrigger : public Gameplay::IComponent {

public:
	typedef std::shared_ptr<EndGameTrigger> Sptr;
	EndGameTrigger();
	virtual ~EndGameTrigger();

	void SetMat(Gameplay::Material::Sptr);

	// Inherited from IComponent

	virtual void OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) override;
	virtual void OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static EndGameTrigger::Sptr FromJson(const nlohmann::json& blob);
	MAKE_TYPENAME(EndGameTrigger);

protected:
	bool _playerInTrigger;
	bool isWinCondition = false;
	Gameplay::Material::Sptr mat = nullptr;
};