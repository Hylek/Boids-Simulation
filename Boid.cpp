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

//float Boid::Range_FAttract = 25.0f;
//float Boid::Range_FRepel = 8.0f;
//float Boid::Range_FAlign = 15.0f;
//float Boid::Range_FMissileRepel = 4.0f;
//float Boid::FRange = 50.0f;
//
//float Boid::FAttract_Vmax = 5.0f;
//
//float Boid::FAttract_Factor = 15.0f;
//float Boid::FRepel_Factor = 2.5f;
//float Boid::FAlign_Factor = 2.0f;
//float Boid::FMissileRepel_Factor = 25.0f;

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
	node->SetPosition(Vector3(randomBoidPos.x_, Random(60.0f, 70.0f), randomBoidPos.y_));
}

void Boid::ComputeForce(Boid * boid, Missile * missile)
{
	force = Repel(boid) + Align(boid) + Attract(boid);

	//Vector3 centerOfMass;
	//Vector3 direction;
	//float neighbourCount = 0;
	//Vector3 attractForce = Vector3(0, 0, 0);
	//Vector3 alignForce = Vector3(0, 0, 0);
	//Vector3 repelForce = Vector3(0, 0, 0);
	//force = Vector3(0, 0, 0);

	//for (int i = 0; i < NUM_BOIDS; i++)
	//{
	//	if (this == &boid[i]) continue;

	//	Vector3 sep = rb->GetPosition() - boid[i].rb->GetPosition();
	//	Vector3 position = boid[i].rb->GetPosition();

	//	float d = sep.Length();

	//	if (d < Range_FAttract)
	//	{
	//		centerOfMass += boid[i].rb->GetPosition();
	//		neighbourCount++;
	//	}
	//	if (d < Range_FAlign)
	//	{
	//		direction += boid[i].rb->GetLinearVelocity();
	//		neighbourCount++;
	//	}
	//	if (d < Range_FRepel)
	//	{
	//		Vector3 delta = (rb->GetPosition() - boid[i].rb->GetPosition());
	//		repelForce += (delta / delta.Length());
	//		repelForce *= FRepel_Factor;
	//		neighbourCount++;
	//	}
	//}
	//if (neighbourCount > 0)
	//{
	//	// Attract
	//	centerOfMass /= neighbourCount;
	//	Vector3 dir = (centerOfMass - rb->GetPosition()).Normalized();
	//	Vector3 vDesired = dir * FAttract_Vmax;
	//	attractForce += (vDesired - rb->GetLinearVelocity()) * FAttract_Factor;

	//	// Align
	//	direction /= neighbourCount;
	//	alignForce = (direction - rb->GetLinearVelocity()) * FAlign_Factor;

	//	// Repel


	//	force += repelForce + alignForce + attractForce;
	//}
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

		if (neighbourCount < 10)
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

		if (neighbourCount < 8)
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

void BoidSet::Init(ResourceCache * pRes, Scene * scene)
{
	Vector2 randomBoidPos;

	// Create the spatial grid.
	//InitGrid();

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		// For each boid set a random position and store it.
		randomBoidPos = Vector2(Random(180.0f) - 90.0f, Random(180.0f) - 90.0f);
		// Create the boids
		boidList[i].Init(pRes, scene, randomBoidPos);

		// Create the grid dimensions based on the random boid positions and divide to create cells.
		//int row = (randomBoidPos.x_ + 100.0f) / cellDivideSize;
		//int col = (randomBoidPos.y_ + 100.0f) / cellDivideSize;

		// Set the index of every Boid in the boidSet.

		// Push back each boid's node into the grid.
		//grid[row][col].push_back(boidList[i]);
		//gridIntTest[row][col].push_back(i);
		//ClearGrid();
	}
	//ClearGrid();
}

void BoidSet::InitGrid()
{
	//grid.push_back(std::vector<std::vector<Boid>>());
	//gridIntTest.resize(10);
	////std::cout << gridIntTest.size() << std::endl;
	//for (int i = 0; i < 10; i++)
	//{
	//	//grid[i].push_back(std::vector<Boid>());
	//	gridIntTest[i].resize(10);
	//	std::cout << gridIntTest[i].size() << std::endl;
	//}

	for (int i = 0; i < 10; i++)
	{
		gridIntTest.push_back(std::vector<std::vector<int>>());
		for (int j = 0; j < 10; j++)
		{
			gridIntTest[i].push_back(std::vector<int>());
		}
	}
}

void BoidSet::Update(float tm, Missile* missile)
{
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		// Calculate the new cell based on the now current position.// THIS LINE CAUSES THE VECTOR SUBSCRIPT OUT OF RANGE ERROR
		//std::cout << gridIntTest[row][col].size() << std::endl;

		//int row = (boidList[i].rb->GetPosition().x_ + 100.0f) / cellDivideSize;
		//int col = (boidList[i].rb->GetPosition().z_ + 100.0f) / cellDivideSize;

		//// If any boid's position causes it to leave the grid, snap them back to grid 9/0
		//if (row > 9)
		//{
		//	row = 9;
		//}
		//if (row < 0)
		//{
		//	row = 0;
		//}
		//if (col > 9)
		//{
		//	col = 9;
		//}
		//if (col < 0)
		//{
		//	col = 0;
		//}

		////std::cout << "ROW: " << row << std::endl;
		////std::cout << "COL: " << col << std::endl;

		//gridIntTest[row][col].push_back(i);

		//// Loop through the cell the current boid is in, compute force for all other boids in this cell
		////// Then search through the neighbouring cells and check for boids in those and apply forces.
		//std::vector<int> pos = gridIntTest[row][col];

		//for (int j = 0; j < gridIntTest[row][col].size(); j++)
		//{
		//	if (i != gridIntTest[row][col][j])
		//	{
		//		boidList[i].ComputeForce(&boidList[0], missile);
		//	}
		//}
		//if (row + 1 < 9)
		//{
		//	for (int j = 0; j < gridIntTest[row + 1 < 9][col].size(); j++)
		//	{
		//		if (row + 1 < 9 && i != gridIntTest[row + 1 < 9][col][j])
		//		{
		//			boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
		//		}
		//	}
		//}
		//if (col + 1 < 9)
		//{
		//	for (int j = 0; j < gridIntTest[row][col + 1 < 9].size(); j++)
		//	{
		//		if (col + 1 < 9 && i != gridIntTest[row][col][j])
		//		{
		//			boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
		//		}
		//	}
		//}

		//if (row - 1 > 0)
		//{
		//	for (int j = 0; j < gridIntTest[row - 1 < 0][col].size(); j++)
		//	{
		//		if (row + 1 > 0 && i != gridIntTest[row - 1 > 0][col][j])
		//		{
		//			boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
		//		}
		//	}
		//}

		//if (col + 1 < 9)
		//{
		//	for (int j = 0; j < gridIntTest[row][col + 1 < 9].size(); j++)
		//	{
		//		if (col + 1 < 9 && i != gridIntTest[row][col + 1 < 9][j])
		//		{
		//			std::cout << gridIntTest[row][col + 1 < 9].size() << std::endl;
		//			boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
		//		}
		//	}
		//}

		//if (row < 9 && col < 9)
		//{
		//	for (int j = 0; j < gridIntTest[row + 1 < 9][col + 1 < 9].size(); j++)
		//	{
		//		if (i != gridIntTest[row + 1 < 9][col + 1 < 9][j])
		//		{
		//			boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
		//		}
		//	}
		//}


			//if (i != gridIntTest[row][col + 1][j] && col < 10)
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}
			//if (i != gridIntTest[row][col - 1][j] && col > 0)
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}
			//if (row > 0 && i != gridIntTest[row - 1][col][j])
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}
			//if (i != gridIntTest[row + 1][col + 1][j] && col < 10 && row < 10)
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}
			//if (i != gridIntTest[row - 1][col - 1][j] && row > 0 && col > 0)
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}
			//if (i != gridIntTest[row - 1][col + 1][j] && row > 0 && col < 10)
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}
			//if (i != gridIntTest[row + 1][col - 1][j] && col > 0 && row < 10)
			//{
			//	boidList[i].ComputeForce(&boidList[0], missile, gridIntTest[row][col][j]);
			//}

		//std::cout << "SIZE: " << gridIntTest[row][col].size() << std::endl;
		boidList[i].ComputeForce(&boidList[0], missile);
		boidList[i].Update(tm);

		// After the update cycle, recalculate the cell based on the new boid's position
		//int newRow = (boidList[i].node->GetPosition().x_ + 100.0f) / cellDivideSize;
		//int newCol = (boidList[i].node->GetPosition().z_ + 100.0f) / cellDivideSize;

		//if (newRow != boidList[i].row || newCol != boidList[i].col)
		//{
		//	std::cout << "Change in cell!" << std::endl;
		//	grid[boidList[i].row][boidList[i].col].erase(grid[boidList[i].row][boidList[i].col].begin() + boidList[i].index - 1);
		//	grid[newRow][newCol].push_back(boidList[i]);
		//}
	}
	//ClearGrid();
}

void BoidSet::ClearGrid()
{
	for (int row = 0; row < 10; row++)
	{
		for (int col = 0; col < 10; col++)
		{
			gridIntTest[row][col].clear();
		}
	}
}

bool BoidSet::CellChange()
{

	return false;
}

void BoidSet::SearchGrid()
{

}

void BoidSet::UpdateGrid()
{

}
