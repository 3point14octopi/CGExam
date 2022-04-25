#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"
#include "Gameplay/InputEngine.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class EnemyController : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<EnemyController> Sptr;

	EnemyController();
	~EnemyController() = default;
	
	float xVelocity;
	float yVelocity;
	float zVelocity;

	bool xpos;//s
	bool xneg;//w
	int movingLeft = 0;
	int movingRight = 0;
	bool zpos;//up
	bool zneg;//down

	float _velMultiplier;

	Gameplay::Physics::RigidBody::Sptr _body;

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static EnemyController::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(EnemyController);
};

