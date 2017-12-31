#include "Main.h"

URHO3D_DEFINE_APPLICATION_MAIN(Main)

static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
static const StringHash PLAYER_ID("IDENTITY");
static const StringHash MISSILE_ID("MISSILEIDENTITY");
static const StringHash E_CLIENTISREADY("ClientReadyToStart");
static const StringHash E_STARTGAME("StartGame");
static const StringHash E_SCOREUPDATE("ScoreUpdate");
static const StringHash CLIENT_SCORE("ClientScores");
static const StringHash VAR_SCORE("Score");
static const StringHash E_SPAWNPLAYER("SpawnPlayer");
static const StringHash E_HITBOID("HitBoid");
static const StringHash E_GAMEOVER("GameOver");
static const StringHash E_WAITINGONPLAYERS("WaitingOnPlayers");
static const StringHash E_PLAYERSREADY("PlayersAreReady");
static const StringHash E_RESETGAME("ResetGame");

// DON'T FORGET THE SEAWEED PARTICLE IDEA!!!

Main::Main(Context* context) : Sample(context)
{

}

Main::~Main()
{

}

void Main::Start()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();
	Sample::Start();
	Sample::InitMouseMode(MM_RELATIVE);
	CreateInitialScene();
	SubscribeToEvents();
	OpenConsoleWindow();
	CreateGameMenu(cache, context_, ui);
	isServer = false;
	hasGameStarted = false;
	text = ui->GetRoot()->CreateChild<Text>();

	disconnectButton->SetVisible(false);
	clientStartGame->SetVisible(false);
}

void Main::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Main, HandlePostUpdate));
	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Main, PhysicsPreStep));
	SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(Main, ClientConnected));
	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(Main, ClientDisconnecting));
	SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(Main, ClientReadyToStart));
	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(Main, ServerToClientObjectID));
	SubscribeToEvent(E_HITBOID, URHO3D_HANDLER(Main, HitBoid));
	SubscribeToEvent(E_SCOREUPDATE, URHO3D_HANDLER(Main, UpdateClientScore));
	SubscribeToEvent(E_GAMEOVER, URHO3D_HANDLER(Main, GameOver));
	SubscribeToEvent(E_WAITINGONPLAYERS, URHO3D_HANDLER(Main, ServerWaitingOnMorePlayers));
	SubscribeToEvent(E_PLAYERSREADY, URHO3D_HANDLER(Main, PlayersAreReadyToStart));
	//SubscribeToEvent(E_RESETGAME, URHO3D_HANDLER(Main, RestartScene));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_HITBOID);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_SCOREUPDATE);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_GAMEOVER);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_WAITINGONPLAYERS);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_PLAYERSREADY);
	//GetSubsystem<Network>()->RegisterRemoteEvent(E_RESETGAME);

}

void Main::CreateInitialScene()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Graphics* graphics = GetSubsystem<Graphics>();

	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);

	cameraNode_ = new Node(context_);
	Camera* cam = cameraNode_->CreateComponent<Camera>(LOCAL);
	cameraNode_->SetPosition(Vector3(30.0f, 40.0f, 0.0f));
	cam->SetFarClip(1000.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, cam));

	// Creating ambient light and fog
	Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.0f, 0.19f, 0.25f));
	zone->SetFogStart(60.0f);
	zone->SetFogEnd(300.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	// Creating a directional light
	Node* lightNode = scene_->CreateChild("DirectionalLight", LOCAL);
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);
	light->SetColor(Color(0.5f, 0.85f, 0.75f));

	// Creating a treasure chest
	Node* boxNode = scene_->CreateChild("TreasureChest", LOCAL);
	boxNode->SetPosition(Vector3(-140.0f, 17.7f, 50.0f));
	boxNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
	boxNode->SetRotation(Quaternion(0.0f, -100.0f, 0.0f));
	StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
	boxObject->SetModel(cache->GetResource<Model>("Models/TreasureChestShut.mdl"));
	boxObject->SetMaterial(cache->GetResource<Material>("Materials/ChestText.xml"));
	boxObject->SetCastShadows(true);
	RigidBody* boxRB = boxNode->CreateComponent<RigidBody>();
	boxRB->SetCollisionLayer(2);
	CollisionShape* boxCol = boxNode->CreateComponent<CollisionShape>();
	boxCol->SetBox(Vector3::ONE);

	// Creating the ocean water
	Node* oceanNode = scene_->CreateChild("OceanTop", LOCAL);
	oceanNode->SetPosition(Vector3(0.0f, 100.0f, 0.0f));
	oceanNode->SetScale(Vector3(2000.0f, 0.0f, 2000.0f));
	StaticModel* oceanObject = oceanNode->CreateComponent<StaticModel>();
	oceanObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	oceanObject->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));
	oceanObject->SetCastShadows(true);

	// Creating the terrains
	Node* terrainNode = scene_->CreateChild("Terrain", LOCAL);
	terrainNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	Terrain* terrain = terrainNode->CreateComponent<Terrain>();
	terrain->SetPatchSize(64);
	terrain->SetSpacing(Vector3(2.0f, 0.5f, 2.0f));
	terrain->SetSmoothing(true);
	terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.jpg"));
	terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	terrain->SetOccluder(true);
	RigidBody* body = terrainNode->CreateComponent<RigidBody>();
	body->SetCollisionLayer(2);
	CollisionShape* collider = terrainNode->CreateComponent<CollisionShape>();
	collider->SetTerrain();

	// Creating the skybox
	Node* skyNode = scene_->CreateChild("Sky", LOCAL);
	Skybox* skybox = skyNode->CreateComponent<Skybox>();
	skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

	// Create bubble streams
	//for (int i = 0; i < 100; i++)
	//{
	//	bubbles.Init(cache, scene_, graphics, Random(-300.0f, 300.0f), Random(-300.0f, 300.0f));
	//}
}

void Main::CreateLocalScene()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Graphics* graphics = GetSubsystem<Graphics>();

	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);

	cameraNode_ = new Node(context_);
	Camera* cam = cameraNode_->CreateComponent<Camera>(LOCAL);
	cameraNode_->SetPosition(Vector3(Random(30.0f), 40.0f, Random(30.0f)));
	cam->SetFarClip(1000.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, cam));

	// Creating ambient light and fog
	Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.0f, 0.19f, 0.25f));
	zone->SetFogStart(100.0f);
	zone->SetFogEnd(1000.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	// Creating a directional light
	Node* lightNode = scene_->CreateChild("DirectionalLight", LOCAL);
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);
	light->SetColor(Color(0.5f, 0.85f, 0.75f));

	// Creating a treasure chest
	Node* boxNode = scene_->CreateChild("TreasureChest", LOCAL);
	boxNode->SetPosition(Vector3(-140.0f, 17.7f, 50.0f));
	boxNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
	boxNode->SetRotation(Quaternion(0.0f, -100.0f, 0.0f));
	StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
	boxObject->SetModel(cache->GetResource<Model>("Models/TreasureChestShut.mdl"));
	boxObject->SetMaterial(cache->GetResource<Material>("Materials/ChestText.xml"));
	boxObject->SetCastShadows(true);
	RigidBody* boxRB = boxNode->CreateComponent<RigidBody>();
	CollisionShape* boxCol = boxNode->CreateComponent<CollisionShape>();
	boxCol->SetBox(Vector3::ONE);

	// Creating the ocean water
	Node* oceanNode = scene_->CreateChild("OceanTop", LOCAL);
	oceanNode->SetPosition(Vector3(0.0f, 100.0f, 0.0f));
	oceanNode->SetScale(Vector3(2000.0f, 0.0f, 2000.0f));
	StaticModel* oceanObject = oceanNode->CreateComponent<StaticModel>();
	oceanObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	oceanObject->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));
	oceanObject->SetCastShadows(true);

	// Creating the terrains
	Node* terrainNode = scene_->CreateChild("Terrain", LOCAL);
	terrainNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	Terrain* terrain = terrainNode->CreateComponent<Terrain>();
	terrain->SetPatchSize(64);
	terrain->SetSpacing(Vector3(2.0f, 0.5f, 2.0f));
	terrain->SetSmoothing(true);
	terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.jpg"));
	terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	terrain->SetOccluder(true);
	RigidBody* body = terrainNode->CreateComponent<RigidBody>();
	body->SetCollisionLayer(2);
	CollisionShape* collider = terrainNode->CreateComponent<CollisionShape>();
	collider->SetTerrain();

	// Creating the skybox
	Node* skyNode = scene_->CreateChild("Sky", LOCAL);
	Skybox* skybox = skyNode->CreateComponent<Skybox>();
	skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));
}

void Main::AddObjects()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Graphics* graphics = GetSubsystem<Graphics>();

	// Create objects
	boidSet.Init(cache, scene_);

	missile.CreateMissile(cache, scene_);

	for (int i = 0; i < 100; i++)
	{
		bubbles.Init(cache, scene_, graphics, Random(-300.0f, 300.0f), Random(-300.0f, 300.0f));
	}
}

void Main::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	if (GetSubsystem<UI>()->GetFocusElement()) return;

	UI* ui = GetSubsystem<UI>();
	Input* input = GetSubsystem<Input>();
	const float MOVE_SPEED = 40.0f;
	const float MOUSE_SENSITIVITY = 0.1f;

	// Adjust node yaw and pitch via mouse movement and limit pitch between -90 to 90
	IntVector2 mouseMove = input->GetMouseMove();
	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);

	if (!ignoreInputs)
	{
		// Make new camera orientation for the camera scene node, roll is 0
		cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

		if (input->GetKeyDown(KEY_W))
		{
			cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
		}
		if (input->GetKeyDown(KEY_S))
		{
			cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
		}
		if (input->GetKeyDown(KEY_A))
		{
			cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
		}
		if (input->GetKeyDown(KEY_D))
		{
			cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
		}
		//if (input->GetMouseButtonDown(MOUSEB_LEFT))
		//{
		//	missile.isActive = true;
		//}
	}
	if (input->GetKeyPress(KEY_M))
	{
		isMenuVisible = !isMenuVisible;
	}
	if (missile.missileTimer > 0)
	{
		missile.missileTimer -= timeStep;
	}
	if (!ui->GetCursor()->IsVisible())
	{
		ignoreInputs = false;
	}
	else
	{
		ignoreInputs = true;
	}
	//if (hasGameStarted)
	//{
	//	if (missile.isActive)
	//	{
	//		missile.model->SetEnabled(true);
	//		if (swap == 1)
	//		{
	//			missile.rb->SetPosition(cameraNode_->GetPosition());
	//			missile.rb->SetLinearVelocity(cameraNode_->GetDirection().Normalized() * 20.0f);
	//			swap = 0;
	//		}
	//		if (missile.missileTimer <= 0)
	//		{
	//			missile.isActive = false;
	//			missile.model->SetEnabled(false);
	//			missile.missileTimer = 10;
	//			swap = 1;
	//		}
	//	}
	//}

	if (isServer)
	{
		boidSet.Update(timeStep, &missile);

		if (gameTimer > 0 && clientCount >= 2)
		{
			gameTimer -= timeStep;
			text->SetVisible(false);
			std::cout << gameTimer << std::endl;
		}
		if (gameTimer <= 0)
		{
			std::cout << "GAME OVER" << std::endl;
			text->SetVisible(true);
			// GAME OVER
		}
		if (clientCount < 2)
		{
			gameTimer = 60;
		}
		if (isGameOver && resetTimer > 0)
		{
			resetTimer -= timeStep;
		}
	}
	bubbles.Update(timeStep);
}

void Main::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	UI* ui = GetSubsystem<UI>();
	Input* input = GetSubsystem<Input>();
	ui->GetCursor()->SetVisible(isMenuVisible);
	window->SetVisible(isMenuVisible);
	MoveCamera();
}

void Main::HandleCollision(StringHash eventType, VariantMap& eventData)
{
	printf("Collision detected\n");
	unsigned missileID = eventData[MISSILE_ID].GetUInt();
	std::cout << "MISSILE ID OF COLLISION: " << missileID << std::endl;

}

//
// MENU CODE START
//
void Main::CreateGameMenu(ResourceCache* cache, Context* context, UI* ui)
{
	UIElement* root = ui->GetRoot();
	XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(style);

	SharedPtr<Cursor> cursor(new Cursor(context));
	cursor->SetStyleAuto(style);
	ui->SetCursor(cursor);

	window = new Window(context);
	root->AddChild(window);

	window->SetMinWidth(384);
	window->SetMinHeight(200);
	window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window->SetAlignment(HA_CENTER, VA_CENTER);
	window->SetName("Window");
	window->SetStyleAuto();

	connectButton = CreateButton("Connect", 24, window, cache);
	serverAddressEdit = CreateLineEdit("localhost", 12, window, cache);
	disconnectButton = CreateButton("Disconnect", 24, window, cache);
	startServerButton = CreateButton("Start Server", 24, window, cache);
	clientStartGame = CreateButton("Client: Start Game", 24, window, cache);
	quitButton = CreateButton("Quit Game", 24, window, cache);

	SubscribeToEvent(quitButton, E_RELEASED, URHO3D_HANDLER(Main, HandleQuit));
	SubscribeToEvent(startServerButton, E_RELEASED, URHO3D_HANDLER(Main, StartServer));
	SubscribeToEvent(connectButton, E_RELEASED, URHO3D_HANDLER(Main, Connect));
	SubscribeToEvent(disconnectButton, E_RELEASED, URHO3D_HANDLER(Main, Disconnect));
	SubscribeToEvent(clientStartGame, E_RELEASED, URHO3D_HANDLER(Main, ClientStartGame));
}

void Main::HandleQuit(StringHash eventType, VariantMap& eventData)
{
	engine_->Exit();
}

Button* Main::CreateButton(const String & text, int pHeight, Urho3D::Window * whichWindow, ResourceCache* cache)
{
	Font* font = cache->GetResource<Font>("Fonts/Roboto-Light.ttf");
	Button* button = whichWindow->CreateChild<Button>();
	button->SetMinHeight(pHeight);
	button->SetStyleAuto();
	Text* buttonText = button->CreateChild<Text>();
	buttonText->SetFont(font, 12);
	buttonText->SetAlignment(HA_CENTER, VA_CENTER);
	buttonText->SetText(text);
	whichWindow->AddChild(button);
	return button;
}

LineEdit* Main::CreateLineEdit(const String & text, int pHeight, Urho3D::Window * whichWindow, ResourceCache* cache)
{
	LineEdit* lineEdit = whichWindow->CreateChild<LineEdit>();
	lineEdit->SetMinHeight(pHeight);
	lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
	lineEdit->SetText(text);
	whichWindow->AddChild(lineEdit);
	lineEdit->SetStyleAuto();
	return lineEdit;
}
//
// MENU CODE END
//

//
// NETWORK CODE START
//

// Start a new server // DEFAULT
void Main::StartServer(StringHash eventType, VariantMap& eventData)
{
	Log::WriteRaw("Server is starting...");

	AddObjects();
	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);

	isMenuVisible = !isMenuVisible;
	gameTimer = 60;
	isServer = true;
	connectButton->SetVisible(false);
	disconnectButton->SetVisible(true);
	startServerButton->SetVisible(false);
	serverAddressEdit->SetVisible(false);
}

// Connect a client to the server in SPECTATOR MODE // CLIENT FUNCTION
void Main::Connect(StringHash eventType, VariantMap& eventData)
{
	Network* network = GetSubsystem<Network>();
	String address = serverAddressEdit->GetText().Trimmed();

	// If the linedit is empty, just use "localhost"
	if (address.Empty())
	{
		address = "localhost";
	}

	// Connect to the server and hide the menu.
	network->Connect(address, SERVER_PORT, scene_);

	isMenuVisible = !isMenuVisible;
	connectButton->SetVisible(false);
	startServerButton->SetVisible(false);
	clientStartGame->SetVisible(true);
	serverAddressEdit->SetVisible(false);
}

// When a client has connected, refresh the scene and create a new connection. // SERVER FUNCTION
void Main::ClientConnected(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("*HANDLECONNECTEDCLIENT CALLED: A client has connected to the server");
	using namespace ClientConnected;

	score.push_back(0);

	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	newConnection->SetScene(scene_);
}

// Disconnect a client from the server OR if server, stop the server // SERVER AND CLIENT FUNCTION
void Main::Disconnect(StringHash eventType, VariantMap& eventData)
{
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	// If Running as Client, disconnect from the server
	if (serverConnection)
	{
		Log::WriteRaw("Disconnecting from the server...");
		serverConnection->Disconnect();
		scene_->Clear(true, false);
		hasGameStarted = false;
		clientObjectID = 0;
		serverAddressEdit->SetVisible(true);
		disconnectButton->SetVisible(false);
		startServerButton->SetVisible(true);
		connectButton->SetVisible(true);
		text->SetVisible(false);
	}
	// If Running as a server, stop the server
	else if (network->IsServerRunning())
	{
		Log::WriteRaw("Stopping server...");
		network->StopServer();
		scene_->Clear(true, false);
		isServer = false;
		hasGameStarted = false;
		CreateInitialScene();
		startServerButton->SetVisible(true);
		disconnectButton->SetVisible(false);
		serverAddressEdit->SetVisible(true);
		text->SetVisible(false);
	}
	connectButton->SetVisible(true);
}

// When the client is disconnecting, do stuff... // SERVER FUNCTION
void Main::ClientDisconnecting(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("A client is disconnecting from the server.");
	using namespace ClientConnected;
	Connection* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	if (clientCount > 0)
	{
		Node* playerNode = serverObjects[connection];
		playerNode->SetEnabled(false);
		clientCount--;
	}
}

// Add the client into the game when the player presses Start Game // CLIENT FUNCTION
void Main::ClientStartGame(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("The client is starting the a new game");
	if (clientObjectID == 0)
	{
		Network* network = GetSubsystem<Network>();
		Connection* serverConnection = network->GetServerConnection(); // Get server connection
		if (serverConnection)
		{
			VariantMap remoteEventData;
			remoteEventData[PLAYER_ID] = 0;
			serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
			isMenuVisible = !isMenuVisible;
		}
	}
	clientStartGame->SetVisible(false);
	disconnectButton->SetVisible(true);
	serverAddressEdit->SetVisible(false);

	// Add the bubbles after the server has connected, as they do not affect the game and can be made locally.
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Graphics* graphics = GetSubsystem<Graphics>();
	for (int i = 0; i < 100; i++)
	{
		bubbles.Init(cache, scene_, graphics, Random(-300.0f, 300.0f), Random(-300.0f, 300.0f));
	}
}

// Handle the processing of controls by client and server // SERVER AND CLIENT FUNCTION
void Main::PhysicsPreStep(StringHash eventType, VariantMap & eventData)
{
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	if (serverConnection)	// If the game is the client
	{
		serverConnection->SetPosition(cameraNode_->GetPosition()); // Send client camera position to server
		serverConnection->SetControls(ClientToServerControls()); // Set and send client controls to the server
	}
	else if (network->IsServerRunning())	// If the game is the server and it is running
	{
		ProcessClientControls(); // Process incoming controls from clients
	}
}

// Handle a client that has finished loading // SERVER FUNCTION
void Main::ClientFinishedLoading(StringHash eventType, VariantMap & eventData)
{
	// This function is useful for initing objects after the client has loaded.
	Log::WriteRaw("A client has completed loading!");

}

// Client is ready to start, establish a new connection and create a new client controlled object // CLIENT FUNCTION
void Main::ClientReadyToStart(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("Event sent by the Client and running on Server: Client is ready to start the game.");

	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	Node* playerNode = CreatePlayer();
	serverObjects[newConnection] = playerNode;
	clientCount++;

	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = playerNode->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}

// Game over is declared, inform the clients // CLIENT FUNCTION
void Main::GameOver(StringHash eventType, VariantMap & eventData)
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();

	text->SetVisible(true);
	// Construct new Text object, set string to display and font to use
	text->SetText("THE GAME IS NOW OVER!");
	text->SetFont(cache->GetResource<Font>("Fonts/Roboto-Light.ttf"), 15);

	// Position the text relative to the screen center
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_CENTER);
	text->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}

// Create client controlled object on the server // SERVER FUNCTION
Node* Main::CreatePlayer()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	Node* playerNode = scene_->CreateChild("Player");
	playerNode->SetPosition(cameraNode_->GetPosition());
	playerNode->SetScale(0.5f);
	playerNode->SetRotation(Quaternion(0.0f, 0.0f, 270.0f));
	StaticModel* model = playerNode->CreateComponent<StaticModel>();
	model->SetModel(cache->GetResource<Model>("Models/Sub.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/StoneSmall.xml"));

	RigidBody* body = playerNode->CreateComponent<RigidBody>();
	body->SetLinearDamping(0.65f);
	body->SetAngularDamping(0.65f);
	body->SetCollisionMask(4);
	body->SetCollisionMask(6);
	body->SetMass(5.0f);
	body->SetUseGravity(false);
	CollisionShape* shape = playerNode->CreateComponent<CollisionShape>();
	shape->SetBox(Vector3(3.0f, 5.0f, 15.0f));

	return playerNode;
}

// Create the missile object. // SERVER FUNCTION
Node* Main::CreateMissile()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	Node* node = scene_->CreateChild("Missile");
	node->SetScale(Vector3(0.5f, 0.5f, 0.5f));
	RigidBody* rb = node->CreateComponent<RigidBody>();
	StaticModel* model = node->CreateComponent<StaticModel>();
	CollisionShape* collider = node->CreateComponent<CollisionShape>();

	model->SetModel(cache->GetResource<Model>("Models/TeaPot.mdl"));
	//model->SetMaterial(cache->GetResource<Material>("Materials/Missile.xml"));
	collider->SetBox(Vector3::ONE);
	rb->SetCollisionLayer(2);
	rb->SetCollisionMask(4);
	rb->SetCollisionMask(6);
	rb->SetUseGravity(false);
	rb->SetMass(5.0f);

	return node;
}

// Client has requested a missile, create one and fire it. // SERVER FUNCTION
void Main::ShootMissile(Connection* playerConnection, unsigned i, VariantMap client)
{
	Node* newNode = CreateMissile();
	missileVector.emplace_back(newNode);
	std::cout << missileVector.size() << std::endl;

	Node* playerNode = serverObjects[playerConnection];
	newNode->SetVar("ID", playerConnection->GetIdentity());
	newNode->SetPosition(Vector3(playerNode->GetPosition().x_, playerNode->GetPosition().y_ + 1.0f, playerNode->GetPosition().z_ + 1.0f));
	newNode->GetComponent<RigidBody>()->ApplyImpulse(playerNode->GetWorldDirection() * 500.0f);

	SubscribeToEvent(newNode, E_NODECOLLISION, URHO3D_HANDLER(Main, HandleCollision));

	VariantMap remoteEventData;
	remoteEventData[MISSILE_ID] = newNode->GetID();

	if (newNode->GetVar("ID") == client) // Detecting which missile belongs to which client
	{
		playerConnection->SendRemoteEvent(E_HITBOID, true, remoteEventData);
	}
}

// Process the collisions for the missiles // SERVER FUNCTION
void Main::ProcessCollisions(Connection* connection)
{
	for (int j = 0; j < missileVector.size(); j++)
	{
		Ray cameraRay(missileVector[j]->GetPosition(), missileVector[j]->GetWorldDirection() * Vector3::FORWARD * 100.0f);
		PhysicsRaycastResult result;
		scene_->GetComponent<PhysicsWorld>()->SphereCast(result, cameraRay, 5, 5, 4);
		if (result.body_)
		{
			Node* boid = result.body_->GetNode();

			if (boid->GetName() == "Boid") // Update this for any future boids
			{
				boid->SetEnabled(false);
				missileVector[j]->SetEnabled(false);
				missileVector[j]->GetComponent<RigidBody>()->SetEnabled(false);

				VariantMap remoteEventData;
				remoteEventData[CLIENT_SCORE];
				connection->SendRemoteEvent(E_SCOREUPDATE, true, remoteEventData);
			}
		}
	}
}

// When there is only 1 client ready, wait for another to start the game // SERVER FUNCTION
void Main::ServerWaitingOnMorePlayers(StringHash eventType, VariantMap & eventData)
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();

	text->SetVisible(true);
	// Construct new Text object, set string to display and font to use
	text->SetText("Waiting on one more player!");
	text->SetFont(cache->GetResource<Font>("Fonts/Roboto-Light.ttf"), 15);

	// Position the text relative to the screen center
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_CENTER);
	text->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}

// There are enough players to start the game, hide the text // SERVER FUNCTION
void Main::PlayersAreReadyToStart(StringHash eventType, VariantMap & eventData)
{
	text->SetVisible(false);
}

// Client recieves it's score from the server // CLIENT FUNCTION
void Main::UpdateClientScore(StringHash eventType, VariantMap & eventData)
{
	int score = eventData[CLIENT_SCORE].GetInt();
	std::cout << "CLIENT SCORE IS: " << score << std::endl;
	std::string scoreString = std::to_string(score);

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();

	Text* textScore = ui->GetRoot()->CreateChild<Text>();
	// Construct new Text object, set string to display and font to use
	textScore->SetFont(cache->GetResource<Font>("Fonts/Roboto-Light.ttf"), 15);

	// Position the text relative to the screen center
	textScore->SetHorizontalAlignment(HA_CENTER);
	textScore->SetVerticalAlignment(VA_CENTER);
	textScore->SetPosition(0, ui->GetRoot()->GetHeight() * 6);


}

// When a boid is hit server reports back to the client and update the score UI // CLIENT FUNCTION
void Main::HitBoid(StringHash eventType, VariantMap & eventData)
{
	printf("YOU FIRED A MISSILE\n");
	unsigned value = eventData[MISSILE_ID].GetUInt();
	std::cout << value << std::endl;
}

// Move the camera for the client with it's controlled object on the server // CLIENT FUNCTION
void Main::MoveCamera()
{
	// Only move the camera if we have a controllable object  
	if (clientObjectID)
	{
		Node* playerNode = this->scene_->GetNode(clientObjectID);
		if (playerNode)
		{
			const float CAMERA_DISTANCE = 20.0f;
			cameraNode_->SetPosition(playerNode->GetPosition() + cameraNode_->GetRotation() * Vector3::BACK * CAMERA_DISTANCE);
		}
	}
}

// Get client objects on the server and report their ID back // SERVER FUNCTION
void Main::ServerToClientObjectID(StringHash eventType, VariantMap & eventData)
{
	clientObjectID = eventData[PLAYER_ID].GetUInt();
	std::cout << "Client Player ID: " << clientObjectID << std::endl;
}

// Process controls from clients // SERVER FUNCTION
void Main::ProcessClientControls()
{	
	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();

	// If there are less than 2 players connected and ready, wait until there is enough players.
	if (clientCount < 2)
	{
		VariantMap remoteEventData;
		network->BroadcastRemoteEvent(E_WAITINGONPLAYERS, true, remoteEventData);
	}

	// Loop through every client connected to the server
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
		Connection* connection = connections[i];
		Node* playerNode = serverObjects[connection];
		VariantMap client = connection->GetIdentity();

		if (!playerNode) continue;

		const Controls& controls = connection->GetControls();
		Quaternion rotation(0.0f, controls.yaw_, 0.0f);
		clientDirection = Vector3(0, 0, rotation.x_);

		if (gameTimer > 0 && clientCount >= 2)
		{
			if (controls.buttons_ & CTRL_FORWARD) playerNode->GetComponent<RigidBody>()->ApplyForce(playerNode->GetWorldDirection() * 180.0f);   //Log::WriteRaw("Received from Client: Controls buttons FORWARD \n");
			if (controls.buttons_ & CTRL_BACK)    playerNode->GetComponent<RigidBody>()->ApplyForce(-playerNode->GetWorldDirection() * 180.0f);   //Log::WriteRaw("Received from Client: Controls buttons BACK \n");
			if (controls.buttons_ & CTRL_LEFT)	  playerNode->GetComponent<RigidBody>()->ApplyTorque(rotation * Vector3::DOWN * 10.0f);			//Log::WriteRaw("Received from Client: Controls buttons LEFT \n");
			if (controls.buttons_ & CTRL_RIGHT)   playerNode->GetComponent<RigidBody>()->ApplyTorque(rotation * Vector3::UP * 10.0f);			//Log::WriteRaw("Received from Client: Controls buttons RIGHT \n");
			if (controls.buttons_ & CTRL_FIRE)    ShootMissile(connection, i, client); // Maybe add something to do with ID shit here?
			if (controls.buttons_ & 16)			  playerNode->GetComponent<RigidBody>()->ApplyForce(Vector3::UP * 40.0f);
			if (controls.buttons_ & 32)			  playerNode->GetComponent<RigidBody>()->ApplyForce(Vector3::DOWN * 40.0f);
		}

		if (clientCount >= 2)
		{
			ProcessCollisions(connection);
			VariantMap remoteEventData;
			network->BroadcastRemoteEvent(E_PLAYERSREADY, true, remoteEventData);
		}
		VariantMap remoteEventData;
		if (gameTimer <= 0)
		{
			isGameOver = true;
			network->BroadcastRemoteEvent(E_GAMEOVER, true, remoteEventData);
		}
		if (resetTimer <= 0)
		{
		}
	}
}

// Package controls inputted and send it to the server for processing // CLIENT FUNCTION
Controls Main::ClientToServerControls()
{
	Input* input = GetSubsystem<Input>();
	Controls controls;

	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.Set(CTRL_FIRE, input->GetKeyDown(KEY_E));
	controls.Set(16, input->GetKeyDown(KEY_R));
	controls.Set(32, input->GetKeyDown(KEY_F));


	controls.yaw_ = yaw_;
	return controls;
}
