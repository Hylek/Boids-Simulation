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
#include "Boid.h"
#include "Missile.h"
#include "Menu.h"

using namespace Urho3D;

namespace Urho3D
{
	class Node;
	class Scene;
	class Window;
}

class Main : public Sample
{
	URHO3D_OBJECT(Main, Sample);
public:
	Main(Context* context);
	~Main();
private:
	unsigned short int swap = 1;
	bool isServer = false;
	bool hasGameStarted = false;
	bool isMenuVisible = false;
	bool ignoreInputs = false;
	BoidSet boidSet;
	Missile missile;

	virtual void Start();
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
	void CreateInitialScene();
	void CreateLocalScene();
	void SubscribeToEvents();

	// Menu code START
	Button* connectButton;
	Button* disconnectButton;
	Button* quitButton;
	Button* startServerButton;
	Button* clientStartGame;
	LineEdit* serverAddressEdit;
	Texture* uiTexture;
	SharedPtr<Window> window;

	void CreateGameMenu(ResourceCache* cache, Context* context, UI* ui);
	void HandleQuit(StringHash eventType, VariantMap& eventData);
	Button* CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow, ResourceCache* cache);
	LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow, ResourceCache* cache);
	// Menu code END

	// Server code START
	static const unsigned short SERVER_PORT = 2345;
	HashMap<Connection*, WeakPtr<Node> > serverObjects; // Variable to keep track of client controlled server objects.
	unsigned clientObjectID = 0; // ID of client objects present on the server
	void StartServer(StringHash eventType, VariantMap& eventData);
	void Connect(StringHash eventType, VariantMap& eventData);
	void Disconnect(StringHash eventType, VariantMap& eventData);
	void ClientConnected(StringHash eventType, VariantMap & eventData);
	void ClientDisconnecting(StringHash eventType, VariantMap & eventData);
	void ClientStartGame(StringHash eventType, VariantMap& eventData);
	void ServerStartGame(StringHash eventType, VariantMap& eventData);
	void PhysicsPreStep(StringHash eventType, VariantMap & eventData);
	void ClientFinishedLoading(StringHash eventType, VariantMap & eventData);
	void ClientReadyToStart(StringHash eventType, VariantMap & eventData);
	void ServerToClientObjectID(StringHash eventType, VariantMap & eventData);
	Node* CreatePlayer();
	void MoveCamera();

	// Control processing
	int CTRL_FORWARD = 3;
	int CTRL_BACK = 6;
	int CTRL_LEFT = 9;
	int CTRL_RIGHT = 12;
	int CTRL_FIRE = 2046;
	int CTRL_ACTION = 1024;
	void ProcessClientControls();
	Controls ClientToServerControls();

};