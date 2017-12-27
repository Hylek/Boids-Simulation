#include "Boid.h"

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 20.0f;
float Boid::Range_FAlign = 5.0f;
float Boid::Range_FMissileRepel = 4.0f;

float Boid::FAttract_Vmax = 5.0f;

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

void Boid::Init(ResourceCache* cache, Scene* scene)
{
	node = scene->CreateChild("Boid");
	rb = node->CreateComponent<RigidBody>();
	model = node->CreateComponent<StaticModel>();
	collider = node->CreateComponent<CollisionShape>();

	model->SetModel(cache->GetResource<Model>("Models/Cone.mdl"));
	model->SetCastShadows(true);
	collider->SetBox(Vector3::ONE);
	rb->SetUseGravity(false);
	rb->SetCollisionLayer(2);
	rb->SetMass(5.0f);
	node->SetPosition(Vector3(Random(180.0f) - 160.0f, 30.0f, Random(180.0f) - 160.0f));
	// rb->SetLinearVelocity(Vector3(Random(20.0f), 0, Random(20.0f)));
}

void Boid::ComputeForce(Boid * boid, Missile * missile)
{
	force = Repel(boid) + Align(boid) + Attract(boid) + MissileDodge(boid, missile);
}

Vector3 Boid::Attract(Boid * boid)
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

Vector3 Boid::MissileDodge(Boid * boid, Missile * missile)
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

void BoidSet::Init(ResourceCache * pRes, Scene * scene)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		boidList[i].Init(pRes, scene);
	}
}

void BoidSet::Update(float tm, Missile * missile)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		boidList[i].ComputeForce(&boidList[0], missile);
		boidList[i].Update(tm);
	}
}
