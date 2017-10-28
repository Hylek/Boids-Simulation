#include "Main.h"

URHO3D_DEFINE_APPLICATION_MAIN(Main)

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 20.0f;
float Boid::Range_FAlign = 5.0f;

float Boid::FAttract_Vmax = 5.0f;

float Boid::FAttract_Factor = 4.0f;
float Boid::FRepel_Factor = 2.0f;
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
	zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
	zone->SetFogStart(100.0f);
	zone->SetFogEnd(300.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	// Creating a directional light
	Node* lightNode = scene_->CreateChild("DirectionalLight");
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL); // Runtime error here!
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);

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

	boidSet.Init(cache, scene_);
}

void Main::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
	
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

	// Adjust node yaw and pitch via mouse movement and limit pitch between -90 to 90
	Input* input = GetSubsystem<Input>();
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
	node->SetPosition(Vector3(Random(180.0f) - 90.0f, 30.0f, Random(180.0f) - 90.0f));
	rb->SetLinearVelocity(Vector3(Random(20.0f), 0, Random(20.0f)));
}

void Boid::Attract(Boid* boid)
{
	Vector3 centerOfMass;
	int neighbourCount = 0;
	attractForce = Vector3(0, 0, 0);

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
}

void Boid::Align(Boid* boid)
{
	Vector3 direction;
	int neighbourCount = 0;
	alignForce = Vector3(0, 0, 0);

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
}

void Boid::Repel(Boid* boid)
{
	Vector3 neighbourPos;
	int neighbourCount = 0;
	repelForce = Vector3(0, 0, 0);

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
		float distance = sep.Length();

		if (distance < Range_FRepel)
		{
			Vector3 delta = rb->GetPosition() - boid[i].rb->GetPosition();
			//repelForce += (delta / delta * delta);
			repelForce = repelForce - (rb->GetPosition() - boid[i].rb->GetPosition()) * FRepel_Factor;
			neighbourCount++;
		}
	}

	if (neighbourCount > 0)
	{
		//repelForce *= FRepel_Factor;
		//repelForce += ((rb->GetPosition() - neighbourPos) / (rb->GetPosition() - neighbourPos).Normalized() * (rb->GetPosition() - neighbourPos).Normalized()) * FRepel_Factor;
	}
}

void Boid::ComputeForce(Boid* boid)
{
	Attract(boid);
	Align(boid);
	Repel(boid);

	finalForce = Vector3(0, 0, 0);
	finalForce = attractForce + alignForce + repelForce;



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
	rb->ApplyForce(finalForce);
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
	/*if (p.x_ < 10.0f)
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