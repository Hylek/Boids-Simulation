#include "Main.h"

URHO3D_DEFINE_APPLICATION_MAIN(Main)

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 20.0f;
float Boid::Range_FAlign = 5.0f;

float Boid::FAttract_Vmax = 5.0f;

float Boid::FAttract_Factor = 4.0f;
float Boid::FRepel_Factor = 2.5f;
float Boid::FAlign_Factor = 2.0f;

Main::Main(Context* context) : Sample(context), firstPerson(false)
{
	
}

Main::~Main()
{

}

void Main::Start()
{
	Sample::Start();
	Sample::InitMouseMode(MM_RELATIVE);

	CreateScene();
	SubscribeToEvents();
	CreateMainMenu();
}

void Main::CreateScene()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>();
	scene_->CreateComponent<PhysicsWorld>();

	cameraNode_ = new Node(context_);
	Camera* cam = cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	cam->SetFarClip(300.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, cam));

	// Creating ambient light and fog
	Node* zoneNode = scene_->CreateChild("Zone");
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.0f, 0.19f, 0.25f));
	zone->SetFogStart(50.0f);
	zone->SetFogEnd(2000.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	// Creating a directional light
	Node* lightNode = scene_->CreateChild("DirectionalLight");
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);
	light->SetColor(Color(0.5f, 0.85f, 0.75f));

	// Creating the floor
	Node* floorNode = scene_->CreateChild("Floor");
	floorNode->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
	floorNode->SetScale(Vector3(2000.0f, 1.0f, 2000.0f));
	StaticModel* object = floorNode->CreateComponent<StaticModel>();
	object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	RigidBody* body = floorNode->CreateComponent<RigidBody>();
	body->SetCollisionLayer(2);
	CollisionShape* shape = floorNode->CreateComponent<CollisionShape>();
	shape->SetBox(Vector3::ONE);

	// Creating a box
	Node* boxNode = scene_->CreateChild("Box");
	boxNode->SetPosition(Vector3(0.0f, 10.0f, 50.0f));
	boxNode->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
	boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	boxObject->SetCastShadows(true);
	RigidBody* boxRB = boxNode->CreateComponent<RigidBody>();
	boxRB->SetCollisionLayer(2);
	CollisionShape* boxCol = boxNode->CreateComponent<CollisionShape>();
	boxCol->SetBox(Vector3::ONE);

	// Creating the ocean base
	Node* oceanNode = scene_->CreateChild("OceanTop");
	oceanNode->SetPosition(Vector3(0.0f, 250.0f, 0.0f));
	oceanNode->SetScale(Vector3(2000.0f, 0.0f, 2000.0f));
	StaticModel* oceanObject = oceanNode->CreateComponent<StaticModel>();
	oceanObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	oceanObject->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));
	oceanObject->SetCastShadows(true);
	
	boidSet.Init(cache, scene_);
}

void Main::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Main, HandlePostUpdate));
}

void Main::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();
	const float MOVE_SPEED = 40.0f;
	const float MOUSE_SENSITIVITY = 0.1f;

	if (GetSubsystem<UI>()->GetFocusElement())
	{
		return;
	}

	boidSet.Update(timeStep);

	Input* input = GetSubsystem<Input>();
	if (!ignoreInputs)
	{
		// Adjust node yaw and pitch via mouse movement and limit pitch between -90 to 90
		IntVector2 mouseMove = input->GetMouseMove();
		yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
		pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
		pitch_ = Clamp(pitch_, -90.0f, 90.0f);

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
		if (!isMenuVisible)
		{
			if (input->GetKeyPress(KEY_M))
			{
				isMenuVisible = true;
			}
		}
	}

	if (isMenuVisible)
	{
		if (input->GetKeyPress(KEY_M))
		{
			isMenuVisible = true;
		}
	}

	if (!ui->GetCursor()->IsVisible())
	{
		ignoreInputs = false;
	}
	else
	{
		ignoreInputs = true;
	}
}

void Main::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	Input* input = GetSubsystem<Input>();
	ui->GetCursor()->SetVisible(isMenuVisible);
	window->SetVisible(isMenuVisible);
}

void Main::CreateMainMenu()
{
	InitMouseMode(MM_RELATIVE);
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(style);

	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(style);
	ui->SetCursor(cursor);

	window = new Window(context_);
	root->AddChild(window);

	window->SetMinWidth(384);
	window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window->SetAlignment(HA_CENTER, VA_CENTER);
	window->SetName("Window");
	window->SetStyleAuto();

	Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

	CreateButton(font, "Button 1", 24, window);
	CreateButton(font, "Button 2", 24, window);
	CreateButton(font, "Quit", 24, window);

	CreateLineEdit("LineText", 24, window);

	SubscribeToEvent(NULL, E_RELEASED, URHO3D_HANDLER(Main, HandleQuit)); // FIX THE NULL ENTRY
}

Button* Main::CreateButton(Font* font, const String& text, int pHeight, Urho3D::Window* whichWindow)
{
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
	rb->SetUseGravity(false);
	rb->SetMass(5.0f);
	node->SetPosition(Vector3(Random(180.0f) - 160.0f, 30.0f, Random(180.0f) - 160.0f));
	// rb->SetLinearVelocity(Vector3(Random(20.0f), 0, Random(20.0f)));
}

Vector3 Boid::Attract(Boid* boid)
{
	Vector3 centerOfMass;
	int neighbourCount = 0;
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
	int neighbourCount = 0;
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
	Vector3 neighbourPos;
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
		//repelForce += ((rb->GetPosition() - neighbourPos) / (rb->GetPosition() - neighbourPos).Normalized() * (rb->GetPosition() - neighbourPos).Normalized()) * FRepel_Factor;
	}
	return repelForce;
}

Vector3 Boid::Direction(Boid* boid)
{
	Vector3 desiredDirection = Vector3(0, 0, 0);

	return desiredDirection;
}

void Boid::ComputeForce(Boid* boid)
{
	//force = Vector3(0, 0, 0);
	force = Repel(boid) + Align(boid) + Attract(boid);

	/* //Vector3 CoMAttract;
	//Vector3 CoMAlign;
	//Vector3 CoMRepulse;
	//int neighbourCount = 0;
	//force = Vector3(0, 0, 0);
	//Vector3 attractForce = Vector3(0, 0, 0);
	//Vector3 alignForce = Vector3(0, 0, 0);
	//Vector3 repelForce = Vector3(0, 0, 0);
	//Vector3 repelDelta = Vector3(0, 0, 0);


	//for (int i = 0; i < NUM_BOIDS; i++)
	//{
	//	if (this == &boid[i]) continue;

	//	Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
	//	float d = sep.Length();

	//	if (d < Range_FAttract)
	//	{
	//		CoMAttract += boid[i].rb->GetPosition();
	//		neighbourCount++;
	//	}

	//	if (d < Range_FAlign)
	//	{
	//		CoMAlign += boid[i].rb->GetLinearVelocity();
	//		neighbourCount++;
	//	}

	//	// Separation Force
	//	if (d < Range_FRepel)
	//	{
	//		repelDelta = (rb->GetPosition() - boid[i].rb->GetPosition());
	//		repelDelta / repelDelta.LengthSquared() * repelDelta.LengthSquared();
	//		neighbourCount++;
	//	}
	//}


	//if (neighbourCount > 0)
	//{
	//	// Attraction Force
	//	CoMAttract /= neighbourCount;
	//	Vector3 direction = (CoMAttract - rb->GetPosition()).Normalized();
	//	Vector3 vDesired = direction * FAttract_Vmax;
	//	attractForce += (vDesired - rb->GetLinearVelocity()) * FAttract_Factor;

	//	// Align Force
	//	CoMAlign /= neighbourCount;
	//	alignForce += (CoMAlign - rb->GetLinearVelocity()) * FAlign_Factor;

	//	// Repel Force
	//	repelForce += repelDelta * FRepel_Factor;
	//} */
	/* Vector3 repelForce;
	Vector3 attractForce;
	Vector3 alignForce;
	Vector3 finalForce;
	Vector3 posMean;
	Vector3 velMean;

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
		float d = sep.Length();
		posMean += boid[i].rb->GetPosition();
		velMean += boid[i].rb->GetLinearVelocity();

		if (d < Range_FRepel)
		{
			Vector3 delta = rb->GetPosition() - boid[i].rb->GetPosition();
			repelForce += (delta / delta * delta);
		}
	}

	repelForce *= FRepel_Factor;

	posMean /= NUM_BOIDS;
	attractForce = FAttract_Factor * ((((posMean - rb->GetPosition()) / ((posMean - rb->GetPosition()).Normalized()) * FAttract_Vmax) - rb->GetLinearVelocity()));

	velMean /= NUM_BOIDS;
	alignForce = FAlign_Factor * (velMean - rb->GetLinearVelocity());


	force = repelForce + attractForce + alignForce; */
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
	else if (p.y_ > 50.0f)
	{
		p.y_ = 50.0f;
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

BoidSet::BoidSet() : Object(context_)
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

void BoidSet::Update(float frameTime)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		boidList[i].ComputeForce(&boidList[0]);
		boidList[i].Update(frameTime);
	}
}