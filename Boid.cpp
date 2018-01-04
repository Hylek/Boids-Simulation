#include "Boid.h"

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 20.0f;
float Boid::Range_FAlign = 15.0f;
float Boid::Range_FMissileRepel = 4.0f;
float Boid::FRange = 50.0f;

float Boid::FAttract_Vmax = 3.0f;

float Boid::FAttract_Factor = 15.0f;
float Boid::FRepel_Factor = 13.0f;
float Boid::FAlign_Factor = 12.0f;
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
	node->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));
	node->SetScale(Random(0.5f, 1.15f));
	rb = node->CreateComponent<RigidBody>();
	model = node->CreateComponent<StaticModel>();
	collider = node->CreateComponent<CollisionShape>();
	model->SetModel(cache->GetResource<Model>("Models/Fish.mdl"));

	int rand = Random(1, 5);
	switch (rand)
	{
	case 1: model->SetMaterial(cache->GetResource<Material>("Materials/fishcolour1.xml")); break;
	case 2: model->SetMaterial(cache->GetResource<Material>("Materials/fishcolour2.xml")); break;
	case 3: model->SetMaterial(cache->GetResource<Material>("Materials/fishcolour3.xml")); break;
	case 4: model->SetMaterial(cache->GetResource<Material>("Materials/fishcolour4.xml")); break;
	}
	model->SetCastShadows(true);
	collider->SetBox(Vector3(1.0f, 1.5f, 1.0f));
	rb->SetUseGravity(false);
	rb->SetCollisionLayer(4);
	rb->SetAngularFactor(Vector3(0, 1, 0));
	rb->SetCollisionMask(2);
	rb->SetMass(2.0f);
	node->SetPosition(Vector3(randomBoidPos.x_, Random(60.0f, 70.0f), randomBoidPos.y_));
}

void Boid::ComputeForce(Boid * boid)
{
	force = Repel(boid) + Align(boid) + Attract(boid);
}

Vector3 Boid::Attract(Boid* boid)
{
	Vector3 centerOfMass;
	float neighbourCount = 0;
	Vector3 attractForce = Vector3(0, 0, 0);

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		if (this == &boid[i]) continue;

		if (neighbourCount < 5)
		{
			Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
			Vector3 position = boid[i].rb->GetPosition();

			float d = sep.Length();

			if (d < Range_FAttract)
			{
				centerOfMass += boid[i].rb->GetPosition();
				neighbourCount++;
			}
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

		if (neighbourCount < 15)
		{
			Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
			float distance = sep.Length();

			if (distance < Range_FAlign)
			{
				direction += boid[i].rb->GetLinearVelocity();
				neighbourCount++;
			}
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

		if (neighbourCount < 10)
		{
			Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
			float distance = sep.Length();

			if (distance < Range_FRepel)
			{
				Vector3 delta = (rb->GetPosition() - boid[i].rb->GetPosition());
				repelForce += (delta / delta.Length());
				neighbourCount++;
			}
		}
	}
	if (neighbourCount > 0)
	{
		repelForce *= FRepel_Factor;
	}
	return repelForce;
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
	if (p.x_ < -500.0f)
	{
	p.x_ = -500.0f;
	rb->SetPosition(p);
	}
	else if (p.x_ > 500.0f)
	{
	p.x_ = 500.0f;
	rb->SetPosition(p);
	}
	if (p.z_ < -500.0f)
	{
	p.z_ = -500.0f;
	rb->SetPosition(p);
	}
	else if (p.z_ > 500.0f)
	{
	p.z_ = 500.0f;
	rb->SetPosition(p);
	}
}

BoidSet::BoidSet()
{

}

BoidSet::~BoidSet()
{

}

void BoidSet::Init(ResourceCache * pRes, Scene * scene, float xPosMin, float xPosMax, float zPosMin, float zPosMax)
{
	Vector2 randomBoidPos;
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		// For each boid set a random position and store it.
		randomBoidPos = Vector2(Random(xPosMin) - xPosMax, Random(zPosMin) - zPosMax);
		// Create the boids
		boidList[i].Init(pRes, scene, randomBoidPos);
	}
}

void BoidSet::Update(float tm)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		boidList[i].ComputeForce(&boidList[0]);
		boidList[i].Update(tm);
	}
}
