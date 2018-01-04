#pragma once
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/DebugNew.h>
#include <Urho3D/Math/Color.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include <iostream>
#include <vector>

using namespace Urho3D;

static const short int NUM_BOIDS = 50;

namespace Urho3D
{
	class Node;
	class Scene;
	class Window;
}

class Boid
{
	static float Range_FAttract;
	static float Range_FRepel;
	static float Range_FAlign;
	static float Range_FMissileRepel;
	static float FAttract_Factor;
	static float FRepel_Factor;
	static float FMissileRepel_Factor;
	static float FAlign_Factor;
	static float FAttract_Vmax;
	static float FRange;

public:
	Boid();
	~Boid();

	void Init(ResourceCache* cache, Scene* scene, Vector2 randomPos);
	void ComputeForce(Boid* boid);
	Vector3 Attract(Boid* boid);
	Vector3 Align(Boid* boid);
	Vector3 Repel(Boid* boid);
	void Update(float lastFrame);

	Vector3 force;
	Node* node;
	RigidBody* rb;
	CollisionShape* collider;
	StaticModel* model;

	int row;
	int col;
	int index;
};

class BoidSet
{

public:
	BoidSet();
	~BoidSet();

	Boid boidList[NUM_BOIDS];
	std::vector<Boid> boidListVector;

	void Init(ResourceCache *pRes, Scene* scene, float xPosMin, float xPosMax, float zPosMin, float zPosMax);
	void InitGrid();
	void Update(float tm);


	int counter = 0;
};