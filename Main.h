#pragma once
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/DebugNew.h>

#include "Sample.h"


using namespace Urho3D;

const static int NUM_BOIDS = 100;

namespace Urho3D
{

	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;

}

class Boid
{
	static float Range_FAttract;
	static float Range_FRepel;
	static float Range_FAlign;
	static float FAttract_Factor;
	static float FRepel_Factor;
	static float FAlign_Factor;
	static float FAttract_Vmax;

public:

	Boid();
	~Boid();

	void Init(ResourceCache* cache, Scene* scene);
	void ComputeForce(Boid* boid);
	void Update(float lastFrame);

	Vector3 force;
	Node* node;
	RigidBody* rb;
	CollisionShape* collider;
	StaticModel* model;
};

class BoidSet : public Object
{
	URHO3D_OBJECT(BoidSet, Object);

public:
	BoidSet();
	~BoidSet();

	Boid boidList[NUM_BOIDS];

	void Init(ResourceCache *pRes, Scene* scene);
	void Update(float tm);
};

class Main : public Sample
{
    URHO3D_OBJECT(Main, Sample);
	
public:

    Main(Context* context);
    ~Main();

	virtual void Start();
	void SubscribeToEvents();

	BoidSet boidSet;


protected:


private:

	bool firstPerson;
	

	void CreateScene();
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
};