#include "Main.h"

URHO3D_DEFINE_APPLICATION_MAIN(Main)

static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
static const StringHash PLAYER_ID("IDENTITY");
static const StringHash E_CLIENTISREADY("ClientReadyToStart");
static const StringHash E_SPAWNPLAYER("SpawnPlayer");
static const StringHash E_FIREMISSILE("FireMissile");

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 20.0f;
float Boid::Range_FAlign = 5.0f;
float Boid::Range_FMissileRepel = 4.0f;

float Boid::FAttract_Vmax = 5.0f;

float Boid::FAttract_Factor = 15.0f;
float Boid::FRepel_Factor = 12.0f;
float Boid::FAlign_Factor = 8.0f;
float Boid::FMissileRepel_Factor = 25.0f;

// Ctrl + M + O to collapse ALL functions
// Ctrl + M + P to expand ALL functions

Main::Main(Context* context) : Sample(context), firstPerson(false)
{
	
}

Main::~Main()
{

}

void Main::Start()
{
	Sample::Start();
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	//OpenConsoleWindow();
	CreateLocalScene();
	SubscribeToEvents();
	Sample::InitMouseMode(MM_RELATIVE);
	CreateGameMenu();
	isServer = false;

	boidSet.Init(cache, scene_);
	missile.CreateMissile(cache, scene_);
}

void Main::CreateLocalScene()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

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

	//// Creating a box
	//Node* boxNode = scene_->CreateChild("Box", LOCAL);
	//boxNode->SetPosition(Vector3(0.0f, 10.0f, 50.0f));
	//boxNode->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	//StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
	//boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	//boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	//boxObject->SetCastShadows(true);
	//RigidBody* boxRB = boxNode->CreateComponent<RigidBody>();
	//boxRB->SetCollisionLayer(2);
	//CollisionShape* boxCol = boxNode->CreateComponent<CollisionShape>();
	//boxCol->SetBox(Vector3::ONE);

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
	terrain->SetSpacing(Vector3(2.0f, 0.5f, 2.0f)); // Spacing between vertices and vertical resolution of the height map
	terrain->SetSmoothing(true);
	terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.jpg"));
	terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	terrain->SetOccluder(true);
	terrain->SetCastShadows(true);
	CollisionShape* collider = terrainNode->CreateComponent<CollisionShape>();
	collider->SetTerrain(1);

	// Creating the skybox
	Node* skyNode = scene_->CreateChild("Sky", LOCAL);
	Skybox* skybox = skyNode->CreateComponent<Skybox>();
	skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));
}

void Main::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Main, HandlePostUpdate));
	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Main, HandlePhysicsPreStep));
	SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(Main, HandleClientConnected));
	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(Main, HandleClientDisconnecting));
	SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(Main, HandleClientToServerReadyToStart));
	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(Main, HandleServerToClientObjectID));
	SubscribeToEvent(E_FIREMISSILE, URHO3D_HANDLER(Main, HandleFireMissile));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);
	GetSubsystem<Network>()->RegisterRemoteEvent(E_FIREMISSILE);
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

	//	if (input->GetKeyDown(KEY_W))
	//	{
	//		cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
	//	}
	//	if (input->GetKeyDown(KEY_S))
	//	{
	//		cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
	//	}
	//	if (input->GetKeyDown(KEY_A))
	//	{
	//		cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
	//	}
	//	if (input->GetKeyDown(KEY_D))
	//	{
	//		cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
	//	}
	//	if (input->GetMouseButtonDown(MOUSEB_LEFT))
	//	{
	//		missile.isActive = true;
	//	}
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
	if (isServer)
	{
		boidSet.Update(timeStep, &missile);
		if (missile.isActive)
		{
			missile.model->SetEnabled(true);
			if (swap == 1)
			{
				missile.rb->SetPosition(cameraNode_->GetPosition());
				missile.rb->SetLinearVelocity(cameraNode_->GetDirection().Normalized() * 20.0f);
				swap = 0;
			}
			if (missile.missileTimer <= 0)
			{
				missile.isActive = false;
				missile.model->SetEnabled(false);
				missile.missileTimer = 10;
				swap = 1;
			}
		}
	}

}

void Main::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	UI* ui = GetSubsystem<UI>();
	Input* input = GetSubsystem<Input>();
	ui->GetCursor()->SetVisible(isMenuVisible);
	window->SetVisible(isMenuVisible);
}

void Main::HandleStartServer(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("*HANDLESTARTSERVER CALLED: Starting Server");
	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);
	isMenuVisible = !isMenuVisible;
	isServer = true;
}

void Main::HandleConnect(StringHash eventType, VariantMap & eventData)
{
	CreateLocalScene();
	Network* network = GetSubsystem<Network>();
	String address = serverAddressEdit->GetText().Trimmed();

	if (address.Empty())
	{
		address = "localhost";
	}

	network->Connect(address, SERVER_PORT, scene_);
	isMenuVisible = !isMenuVisible;
}

void Main::HandleDisconnect(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("*HANDLEDISCONNECT CALLED: Client has disconnected");

	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	// Running as Client
	if (serverConnection)
	{
		serverConnection->Disconnect();
		scene_->Clear(true, false);
		clientObjectID = 0;
	}
	// Running as a server, stop it
	else if (network->IsServerRunning())
	{
		network->StopServer();
		scene_->Clear(true, false);
	}
}

void Main::HandleClientConnected(StringHash eventType, VariantMap & eventData)
{
	//CreateLocalScene();
	Log::WriteRaw("*HANDLECONNECTEDCLIENT CALLED: A client has connected to the server");
	using namespace ClientConnected;

	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	newConnection->SetScene(scene_);
}

void Main::HandleClientDisconnecting(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("*HANDLECLIENTDISCONNECTING CALLED: A client has disconnected from the server");
	using namespace ClientConnected;
}

void Main::HandlePhysicsPreStep(StringHash eventType, VariantMap & eventData)
{
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	if (serverConnection)	// Client
	{
		serverConnection->SetPosition(cameraNode_->GetPosition());
		serverConnection->SetControls(ClientToServerControls());
	}
	else if (network->IsServerRunning())	// Server
	{
		ProcessClientControls();
	}
}

void Main::HandleClientFinishedLoading(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("Client has completed loading!");
}

void Main::HandleClientStartGame(StringHash eventType, VariantMap & eventData) // Called after pressing client button
{
	Log::WriteRaw("*HANDLECLIENTSTARTGAME CALLED: Client has pressed START GAME \n");
	if (clientObjectID == 0)
	{
		Network* network = GetSubsystem<Network>();
		Connection* serverConnection = network->GetServerConnection();
		if (serverConnection)
		{
			VariantMap remoteEventData;
			remoteEventData[PLAYER_ID] = 0;
			serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
			isMenuVisible = !isMenuVisible;
		}
	}
}

void Main::HandleClientToServerReadyToStart(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("*HANDLECLIENTTOSERVERREADYTOSTART CALLED: Event sent by the Client and running on Server: Client is ready to start the game \n");

	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	Node* newObject = CreateControllableObject();
	serverObjects[newConnection] = newObject;

	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = newObject->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}

void Main::HandleServerToClientObjectID(StringHash eventType, VariantMap & eventData)
{
	clientObjectID = eventData[PLAYER_ID].GetUInt();
	Log::WriteRaw("Client ID : %i \n", clientObjectID);
}

void Main::HandleSpawnPlayer(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("Spawn Player Event Called!");
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Node* player = scene_->CreateChild("Player");
	StaticModel* model = player->CreateComponent<StaticModel>();
	RigidBody* rb = player->CreateComponent<RigidBody>();
	CollisionShape* shape = player->CreateComponent<CollisionShape>();
	model->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	shape->SetBox(Vector3::ONE);
	rb->SetUseGravity(false);
	rb->SetCollisionLayer(2);
}

void Main::HandleFireMissile(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("Client wants to fire missile!");
}

void Main::ProcessClientControls() // Server Controls this function
{
	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();
	// Go through every client connected
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
		Connection* connection = connections[i];
		Node* playerNode = serverObjects[connection];

		if (!playerNode) continue;

		const Controls& controls = connection->GetControls();

		if (controls.buttons_ & CTRL_FORWARD)  playerNode->Translate(Vector3::FORWARD); Log::WriteRaw("Received from Client: Controls buttons FORWARD \n");
		if (controls.buttons_ & CTRL_BACK)     playerNode->Translate(Vector3::BACK);    Log::WriteRaw("Received from Client: Controls buttons BACK \n");
		if (controls.buttons_ & CTRL_LEFT)	   playerNode->Translate(Vector3::LEFT);    Log::WriteRaw("Received from Client: Controls buttons LEFT \n");
		if (controls.buttons_ & CTRL_RIGHT)    playerNode->Translate(Vector3::RIGHT);   Log::WriteRaw("Received from Client: Controls buttons RIGHT \n");
		if (controls.buttons_ & CTRL_ACTION)   Log::WriteRaw("Received from Client: Controls buttons E-ACTION \n");
		if (controls.buttons_ & CTRL_FIRE)	   Log::WriteRaw("Received from Client: Controls buttons FIRE \n");
	}
}

Controls Main::ClientToServerControls() // Function used by client
{
	Input* input = GetSubsystem<Input>();
	Controls controls;

	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.Set(CTRL_ACTION, input->GetKeyDown(KEY_E));
	controls.Set(CTRL_FIRE, input->GetMouseButtonDown(MOUSEB_LEFT));

	controls.yaw_ = yaw_;
	return controls;
}

Node* Main::CreateControllableObject() // Called by server as a response of an event sent by the client when pressing the start game button
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	Node* ballNode = scene_->CreateChild("ClientBall");
	ballNode->SetPosition(cameraNode_->GetPosition()); // This needs to be updated as well
	ballNode->SetScale(2.5f);
	StaticModel* ballObject = ballNode->CreateComponent<StaticModel>();
	ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	ballObject->SetMaterial(cache->GetResource<Material>("Materials/StoneSmall.xml"));

	RigidBody* body = ballNode->CreateComponent<RigidBody>();
	body->SetMass(1.0f);
	body->SetUseGravity(false);
	CollisionShape* shape = ballNode->CreateComponent<CollisionShape>();
	shape->SetSphere(1.0f);

	return ballNode;
}

void Main::CreateGameMenu()
{
	InitMouseMode(MM_RELATIVE);

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(style);

	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(style);
	ui->SetCursor(cursor);

	window = new Window(context_);
	root->AddChild(window);

	window->SetMinWidth(384);
	window->SetMinHeight(200);
	window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window->SetAlignment(HA_CENTER, VA_CENTER);
	window->SetName("Window");
	window->SetStyleAuto();

	connectButton = CreateButton("Connect", 24, window);
	serverAddressEdit = CreateLineEdit("localhost", 12, window);
	disconnectButton = CreateButton("Disconnect", 24, window);
	startServerButton = CreateButton("Start Server", 24, window);
	clientStartGame = CreateButton("Client: Start Game", 24, window);
	quitButton = CreateButton("Quit Game", 24, window);

	SubscribeToEvent(quitButton, E_RELEASED, URHO3D_HANDLER(Main, HandleQuit));
	SubscribeToEvent(startServerButton, E_RELEASED, URHO3D_HANDLER(Main, HandleStartServer));
	SubscribeToEvent(connectButton, E_RELEASED, URHO3D_HANDLER(Main, HandleConnect));
	SubscribeToEvent(disconnectButton, E_RELEASED, URHO3D_HANDLER(Main, HandleDisconnect));
	SubscribeToEvent(clientStartGame, E_RELEASED, URHO3D_HANDLER(Main, HandleClientStartGame));
}

Button* Main::CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow)
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
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

LineEdit* Main::CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow)
{
	LineEdit* lineEdit = whichWindow->CreateChild<LineEdit>();
	lineEdit->SetMinHeight(pHeight);
	lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
	lineEdit->SetText(text);
	whichWindow->AddChild(lineEdit);
	lineEdit->SetStyleAuto();
	return lineEdit;
}

void Main::HandleQuit(StringHash eventType, VariantMap& eventData)
{
	engine_->Exit();
}

Boid::Boid()
{
	node = nullptr;
	model = nullptr;
	collider = nullptr;
	rb = nullptr;
}

Boid::~Boid()
{

}

void Boid::Init(ResourceCache* pRes, Scene* scene)
{
	node = scene->CreateChild("Boid");
	rb = node->CreateComponent<RigidBody>();
	model = node->CreateComponent<StaticModel>();
	collider = node->CreateComponent<CollisionShape>();

	model->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	model->SetCastShadows(true);
	collider->SetBox(Vector3::ONE);
	rb->SetUseGravity(false);
	rb->SetCollisionLayer(2);
	rb->SetMass(5.0f);
	node->SetPosition(Vector3(Random(180.0f) - 160.0f, 30.0f, Random(180.0f) - 160.0f));
	// rb->SetLinearVelocity(Vector3(Random(20.0f), 0, Random(20.0f)));
}

void Boid::InitShark(ResourceCache * cache, Scene * scene)
{
	node = scene->CreateChild("Shark");
	node->SetScale(Vector3(5.0f, 5.0f, 5.0f));
	rb = node->CreateComponent<RigidBody>();
	model = node->CreateComponent<StaticModel>();
	collider = node->CreateComponent<CollisionShape>();

	model->SetModel(cache->GetResource<Model>("Models/Cone.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	model->SetCastShadows(true);
	collider->SetBox(Vector3::ONE);
	rb->SetUseGravity(false);
	rb->SetCollisionLayer(2);
	rb->SetMass(15.0f);
	node->SetPosition(Vector3(Random(180.0f) - 160.0f, 30.0f, Random(180.0f) - 160.0f));
}

Vector3 Boid::Attract(Boid* boid)
{
	Vector3 centerOfMass;
	float neighbourCount = 0;
	Vector3 attractForce = Vector3(0, 0, 0);

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
		float distance = sep.Length();

		if (distance < Range_FAttract)
		{
			centerOfMass += boid[i].rb->GetPosition();
			neighbourCount++;
		}
	}
	if (neighbourCount > 0)
	{
		centerOfMass /= neighbourCount;
		Vector3 direction = (centerOfMass - rb->GetPosition()).Normalized();
		Vector3 vDesired = direction * FAttract_Vmax;
		attractForce += (vDesired - rb->GetLinearVelocity()) * FAttract_Factor;
	}
	return attractForce;
}

Vector3 Boid::Align(Boid* boid)
{
	Vector3 direction;
	float neighbourCount = 0;
	Vector3 alignForce = Vector3(0, 0, 0);

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
		float distance = sep.Length();

		if (distance < Range_FAlign)
		{
			direction += boid[i].rb->GetLinearVelocity();
			neighbourCount++;
		}
	}
	if (neighbourCount > 0)
	{
		direction /= neighbourCount;
		Vector3 vDesired = direction;
		alignForce += (vDesired - rb->GetLinearVelocity()) * FAlign_Factor;
	}
	return alignForce;
}

Vector3 Boid::Repel(Boid* boid)
{
	int neighbourCount = 0;
	Vector3 repelForce = Vector3(0, 0, 0);

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
		float distance = sep.Length();

		if (distance < Range_FRepel)
		{
			Vector3 delta = (rb->GetPosition() - boid[i].rb->GetPosition());
			//repelForce += (delta / delta * delta);
				//repelForce = repelForce - (rb->GetPosition() - boid[i].rb->GetPosition()) * FRepel_Factor;
			repelForce += (delta / delta.Length());
			neighbourCount++;
		}
	}
	if (neighbourCount > 0)
	{
		repelForce *= FRepel_Factor;
	}
	return repelForce;
}

Vector3 Boid::MissileDodge(Boid* boid, Missile* missile)
{
	int neighbourCount = 0;
	Vector3 dodgeForce;

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		Vector3 sep = rb->GetPosition() - missile->rb->GetPosition();
		float distance = sep.Length();

		if (distance < Range_FMissileRepel)
		{
			Vector3 delta = (rb->GetPosition() - missile->rb->GetPosition());
			dodgeForce += (delta / delta.Length());
			neighbourCount++;
		}
	}
	if (neighbourCount > 0)
	{
		dodgeForce *= FMissileRepel_Factor;
	}
	return dodgeForce;
}

void Boid::ComputeForce(Boid* boid, Missile* missile)
{
	force = Repel(boid) + Align(boid) + Attract(boid) + MissileDodge(boid, missile);
}

void Boid::Update(float frameTime)
{
	rb->ApplyForce(force);
	Vector3 velocity = rb->GetLinearVelocity();
	float direction = velocity.Length();

	if (direction < 10.0f)
	{
		direction = 10.0f;
		rb->SetLinearVelocity(velocity.Normalized() * direction);
	}
	else if (direction > 50.0f)
	{
		direction = 50.0f;
		rb->SetLinearVelocity(velocity.Normalized() * direction);
	}

	Vector3 vn = velocity.Normalized();
	Vector3 cp = -vn.CrossProduct(Vector3(0.0f, 1.0f, 0.0f));
	float dp = cp.DotProduct(vn);
	rb->SetRotation(Quaternion(Acos(dp), cp));

	Vector3 p = rb->GetPosition();
	if (p.y_ < 10.0f)
	{
		p.y_ = 10.0f;
		rb->SetPosition(p);
	}
	else if (p.y_ > 90.0f)
	{
		p.y_ = 90.0f;
		rb->SetPosition(p);
	}
	/* if (p.x_ < 10.0f)
	{
		p.x_ = 10.0f;
		rb->SetPosition(p);
	}
	else if (p.x_ > 100.0f)
	{
		p.x_ = 100.0f;
		rb->SetPosition(p);
	}
	if (p.z_ < 10.0f)
	{
		p.z_ = 10.0f;
		rb->SetPosition(p);
	}
	else if (p.z_ > 100.0f)
	{
		p.z_ = 100.0f;
		rb->SetPosition(p);
	} */
}

BoidSet::BoidSet()
{
	
}

BoidSet::~BoidSet()
{

}

void BoidSet::Init(ResourceCache *pRes, Scene* scene)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		boidList[i].Init(pRes, scene);
	}
}

void BoidSet::Update(float frameTime, Missile* missile)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		boidList[i].ComputeForce(&boidList[0], missile);
		boidList[i].Update(frameTime);
	}
}

Missile::Missile()
{

}

Missile::~Missile()
{

}

Node* Missile::CreateMissile(ResourceCache* cache, Scene* scene)
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
