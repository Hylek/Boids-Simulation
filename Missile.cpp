#include "Missile.h"

Missile::Missile()
{

}

Missile::~Missile()
{

}

Node* Missile::CreateMissile(ResourceCache * cache, Scene * scene)
{
	node = scene->CreateChild("Missile");
	rb = node->CreateComponent<RigidBody>();
	model = node->CreateComponent<StaticModel>();
	collider = node->CreateComponent<CollisionShape>();

	model->SetModel(cache->GetResource<Model>("Models/TeaPot.mdl"));
	model->SetEnabled(false);
	rb->SetUseGravity(false);
	rb->SetMass(5.0f);
	missileTimer = 5;

	return node;
}
