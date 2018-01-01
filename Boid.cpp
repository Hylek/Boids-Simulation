#include "Boid.h"

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 15.0f;
float Boid::Range_FAlign = 30.0f;
float Boid::Range_FMissileRepel = 4.0f;

float Boid::FAttract_Vmax = 8.0f;

float Boid::FAttract_Factor = 15.0f;
float Boid::FRepel_Factor = 12.0f;
float Boid::FAlign_Factor = 8.0f;
float Boid::FMissileRepel_Factor = 25.0f;

Boid::Boid()
{

}

Boid::~Boid()
{

}

void Boid::Init(ResourceCache* cache, Scene* scene, Vector2 randomBoidPos)
{
	node = scene->CreateChild("Boid");
	node->SetScale(Random(0.5f, 1.15f));
	rb = node->CreateComponent<RigidBody>();
	model = node->CreateComponent<StaticModel>();
	collider = node->CreateComponent<CollisionShape>();

	model->SetModel(cache->GetResource<Model>("Models/Cone.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/Fish.xml"));
	model->SetCastShadows(true);
	collider->SetBox(Vector3::ONE);
	rb->SetUseGravity(false);
	rb->SetCollisionLayer(4);
	rb->SetCollisionMask(2);
	rb->SetMass(2.0f);
	node->SetPosition(Vector3(randomBoidPos.x_, Random(50.0f, 60.0f), randomBoidPos.y_));
}

void Boid::ComputeForce(Boid * boid, Missile * missile)
{
	force = Repel(boid) + Align(boid) + Attract(boid) /*+ MissileDodge(boid, missile)*/;
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
		Vector3 position = boid[i].rb->GetPosition();

		float d = sep.Length();

		if (d < Range_FAttract)
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

Vector3 Boid::Align(Boid * boid)
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

Vector3 Boid::Repel(Boid * boid)
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

Vector3 Boid::MissileDodge(Boid * boid, Missile* missile)
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

void Boid::Update(float lastFrame)
{
	rb->ApplyForce(force);
	Vector3 velocity = rb->GetLinearVelocity();
	float direction = velocity.Length();

	if (direction < 10.0f)
	{
		direction = 10.0f;
		rb->SetLinearVelocity(velocity.Normalized() * direction);
	}
	else if (direction > 100.0f)
	{
		direction = 100.0f;
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
	if (p.x_ < -300.0f)
	{
	p.x_ = -300.0f;
	rb->SetPosition(p);
	}
	else if (p.x_ > 300.0f)
	{
	p.x_ = 300.0f;
	rb->SetPosition(p);
	}
	if (p.z_ < -200.0f)
	{
	p.z_ = -200.0f;
	rb->SetPosition(p);
	}
	else if (p.z_ > 200.0f)
	{
	p.z_ = 200.0f;
	rb->SetPosition(p);
	}
}

BoidSet::BoidSet()
{

}

BoidSet::~BoidSet()
{

}

void BoidSet::Init(ResourceCache * pRes, Scene * scene)
{
	Vector2 randomBoidPos;

	// Create the spatial grid.
	InitGrid();

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		// For each boid set a random position and store it.
		randomBoidPos = Vector2(Random(180.0f) - 90.0f, Random(180.0f) - 90.0f);
		// Create the boids
		boidList[i].Init(pRes, scene, randomBoidPos);

		// Create the grid dimensions based on the random boid positions and divide to create cells.
		int row = (randomBoidPos.x_ + 100.0f) / cellDivideSize;
		int col = (randomBoidPos.y_ + 100.0f) / cellDivideSize;

		// Push back each boid's node into the grid.
		//grid[row][col].push_back(boidList[i].node);
		gridIntTest[row][col].push_back(i);
	}
}

void BoidSet::InitGrid()
{
	for (int i = 0; i < CELL_AMOUNT; i++)
	{
		//grid.push_back(std::vector<std::vector<Node*>>());
		gridIntTest.push_back(std::vector<std::vector<int>>());
		for (int j = 0; j < CELL_AMOUNT; j++)
		{
			//grid[i].push_back(std::vector<Node*>());
			gridIntTest[i].push_back(std::vector<int>());
		}
	}
}

void BoidSet::Update(float tm, Missile* missile)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		int row = (boidList[i].node->GetPosition().x_ + 100.0f) / cellDivideSize;
		int col = (boidList[i].node->GetPosition().z_ + 100.0f) / cellDivideSize;
		gridIntTest[row][col].push_back(i);


		boidList[i].ComputeForce(&boidList[0], missile);
		boidList[i].Update(tm);
	}
	ClearGrid();
}

void BoidSet::ClearGrid()
{
	for (int row = 0; row < CELL_AMOUNT; row++)
	{
		for (int col = 0; col < CELL_AMOUNT; col++)
		{
			gridIntTest[row][col].clear();
		}
	}
}

void BoidSet::SearchGrid()
{

}

void BoidSet::UpdateGrid()
{

}
