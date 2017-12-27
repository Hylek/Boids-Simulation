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


#include "Sample.h"

using namespace Urho3D;

const static short int NUM_BOIDS = 100;

namespace Urho3D
{
	class Node;
	class Scene;
	class Window;
}

class Missile
{

public:
	Missile();
	~Missile();

	Node* CreateMissile(ResourceCache* cache, Scene* scene);


	Node* node;
	RigidBody* rb;
	CollisionShape* collider;
	StaticModel* model;
	bool isActive = false;
	float missileTimer;

};

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

public:

	Boid();
	~Boid();

	void Init(ResourceCache* cache, Scene* scene);
	void InitShark(ResourceCache* cache, Scene* scene);
	void ComputeForce(Boid* boid, Missile* missile);
	Vector3 Attract(Boid* boid);
	Vector3 Align(Boid* boid);
	Vector3 Repel(Boid* boid);
	Vector3 MissileDodge(Boid* boid, Missile* missile);
	Vector3 Direction(Boid* boid);
	void Update(float lastFrame);

	Vector3 force;
	Node* node;
	RigidBody* rb;
	CollisionShape* collider;
	StaticModel* model;
};

class BoidSet
{

public:
	BoidSet();
	~BoidSet();

	Boid boidList[NUM_BOIDS];

	void Init(ResourceCache *pRes, Scene* scene);
	void Update(float tm, Missile* missile);
};

class Bubbles
{
public:
	Bubbles();
	~Bubbles();
};

class Main : public Sample
{
    URHO3D_OBJECT(Main, Sample);
	
public:

    Main(Context* context);
    ~Main();

	virtual void Start();

	static const unsigned short SERVER_PORT = 2345;

	BoidSet boidSet;
	Missile missile;

	int spatialGrid[50][50];
	bool isServer;

protected:


private:
	unsigned short int swap = 1;
	bool firstPerson;
	bool isMenuVisible = false;
	bool ignoreInputs = false; 
	unsigned clientObjectID = 0;

	int CTRL_FORWARD = 3;
	int CTRL_BACK = 6;
	int CTRL_LEFT = 9;
	int CTRL_RIGHT = 12;
	int CTRL_FIRE = 2046;
	int CTRL_ACTION = 1024;

	SharedPtr<Window> window;
	HashMap<Connection*, WeakPtr<Node> > serverObjects;
	Button* connectButton;
	Button* disconnectButton;
	Button* quitButton;
	Button* startServerButton;
	Button* clientStartGame;
	LineEdit* serverAddressEdit;
	Texture* uiTexture;

	void CreateMenuScene();
	void CreateLocalScene();
	void SubscribeToEvents();
	void CreateGameMenu();
	void CreateSpatialGrid();
	LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow);
	Button* CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow);
	void HandleQuit(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
	void HandleStartServer(StringHash eventType, VariantMap& eventData);
	void HandleConnect(StringHash eventType, VariantMap& eventData);
	void HandleDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleClientConnected(StringHash eventType, VariantMap& eventData);
	void HandleClientDisconnecting(StringHash eventType, VariantMap& eventData);
	void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
	void HandleClientFinishedLoading(StringHash eventType, VariantMap& eventData);
	void HandleClientStartGame(StringHash eventType, VariantMap& eventData);
	void HandleClientToServerReadyToStart(StringHash eventType, VariantMap& eventData);
	void HandleServerToClientObjectID(StringHash eventType, VariantMap& eventData);
	void HandleSpawnPlayer(StringHash eventType, VariantMap& eventData);
	void HandleFireMissile(StringHash eventType, VariantMap& eventData);
	void ProcessClientControls();
	Controls ClientToServerControls();
	Node* CreateControllableObject();
};