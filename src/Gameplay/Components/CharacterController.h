#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"
#include "Gameplay/InputEngine.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class CharacterController : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<CharacterController> Sptr;

	CharacterController();
	~CharacterController() = default;
	
	float xVelocity;
	float yVelocity;
	float zVelocity;

	bool xpos;//s
	bool xneg;//w
	bool ypos;//d
	bool yneg;//a
	bool zpos;//up
	bool zneg;//down

	float _velMultiplier;

	Gameplay::Physics::RigidBody::Sptr _body;

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static CharacterController::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(CharacterController);
};

