#include "Particles.h"

Particles::Particles()
{

}

Particles::~Particles()
{

}

void Particles::Init(ResourceCache* cache, Scene* scene, Graphics* graphics, float xPos, float zPos)
{
	Sprite2D* sprite = cache->GetResource<Sprite2D>("Urho2D/bubble.png");

	if (!sprite) return;

	for (unsigned i = 0; i < NUM_SPRITES; ++i)
	{
		float scaleAmount = Random(0.2f);
		SharedPtr<Node> spriteNode(scene->CreateChild("Bubble", LOCAL));
		spriteNode->SetPosition(Vector3(Random(xPos, xPos + 1.0f), Random(-50.0f), zPos));
		spriteNode->SetScale(Vector3(scaleAmount, scaleAmount, scaleAmount));

		StaticSprite2D* staticSprite = spriteNode->CreateComponent<StaticSprite2D>();

		// Set blend mode
		staticSprite->SetBlendMode(BLEND_ALPHA);
		// Set sprite
		staticSprite->SetSprite(sprite);

		// Add to sprite node vector
		spriteNodes_.Push(spriteNode);

		// Set move speed
		spriteNode->SetVar(VAR_MOVESPEED, Vector3(Random(-2.0f, 2.0f), Random(2.0f, 2.0f), 0.0f));
	}
}

void Particles::InitGroup(ResourceCache * cache, Scene * scene, Graphics * graphics, int number, float xPos, float yPos)
{
	for (int i = 0; i < number; i++)
	{
		Init(cache, scene, graphics, xPos, yPos);
	}
}

void Particles::Update(float timeStep)
{
	for (unsigned i = 0; i < spriteNodes_.Size(); ++i)
	{
		SharedPtr<Node> node = spriteNodes_[i];

		Vector3 position = node->GetPosition();
		Vector3 moveSpeed = node->GetVar(VAR_MOVESPEED).GetVector3();
		Vector3 newPosition = position + moveSpeed * timeStep;
		if (newPosition.x_ < 0.0f || newPosition.x_ > 4.0f)
		{
			newPosition.x_ = position.x_;
			moveSpeed.x_ = -moveSpeed.x_;
			node->SetVar(VAR_MOVESPEED, moveSpeed);
		}
		if (newPosition.y_ > 90.0f)
		{
			newPosition.y_ = Random(-25.0f);
		}
		node->SetPosition(newPosition);
	}
}

