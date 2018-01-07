#include "Particles.h"

Particles::Particles()
{

}

Particles::~Particles()
{

}

void Particles::InitBubbles(ResourceCache* cache, Scene* scene, Graphics* graphics, float xPos, float zPos)
{
	Sprite2D* sprite = cache->GetResource<Sprite2D>("Urho2D/bubble.png");

	for (unsigned i = 0; i < 50; i++)
	{
		float scaleAmount = Random(0.2f);
		SharedPtr<Node> spriteNode(scene->CreateChild("Bubble", LOCAL));
		spriteNode->SetPosition(Vector3(Random(xPos, xPos + 1.0f), Random(-100.0f), zPos));
		spriteNode->SetRotation(Quaternion(0.0f, Random(0.0f, 360.0f), 0.0f));
		spriteNode->SetScale(Vector3(scaleAmount, scaleAmount, scaleAmount));

		StaticSprite2D* staticSprite = spriteNode->CreateComponent<StaticSprite2D>();

		staticSprite->SetBlendMode(BLEND_ALPHA);
		staticSprite->SetSprite(sprite);

		bubbleNodes.Push(spriteNode);
	}
}

void Particles::InitWeeds(ResourceCache* cache, Scene* scene, Graphics* graphics, float xPos)
{
	Sprite2D* sprite = cache->GetResource<Sprite2D>("Urho2D/seaweed.png");

	for (unsigned i = 0; i < 50; i++)
	{
		float scaleAmount = Random(0.2f);
		SharedPtr<Node> spriteNode(scene->CreateChild("SeaWeed", LOCAL));
		spriteNode->SetPosition(Vector3(Random(xPos, xPos + 1.0f), Random(90.0f, -100.0f), Random(-300.0f, 300.0f)));
		spriteNode->SetRotation(Quaternion(0.0f, Random(0.0f, 360.0f), 0.0f));
		spriteNode->SetScale(Vector3(scaleAmount, scaleAmount, scaleAmount));

		StaticSprite2D* staticSprite = spriteNode->CreateComponent<StaticSprite2D>();

		staticSprite->SetBlendMode(BLEND_ALPHA);
		staticSprite->SetSprite(sprite);

		weedNodes.Push(spriteNode);
	}
}

void Particles::Update(float timeStep)
{
	// Loop through each bubblenode and make them float upwards reset their positons when the reach the surface
	for (unsigned i = 0; i < bubbleNodes.Size(); i++)
	{
		SharedPtr<Node> node = bubbleNodes[i];

		Vector3 pos = node->GetPosition() + Vector3::UP * timeStep * 2;
		if (pos.x_ <= 0.0f)
		{
			pos.x_ = node->GetPosition().x_;
		}
		if (pos.y_ > 90.0f)
		{
			pos.y_ = Random(-25.0f);
		}
		node->SetPosition(pos);
	}
}

void Particles::UpdateSeaWeed(float timeStep)
{
	// Loop through each seaweednode and make them float upwards and reset their positons when the reach the surface
	for (unsigned i = 0; i < weedNodes.Size(); i++)
	{
		SharedPtr<Node> node = weedNodes[i];

		Vector3 pos = node->GetPosition() + Vector3::UP  * timeStep * 2;
		if (pos.y_ > 90.0f)
		{
			pos.y_ = Random(-25.0f);
		}
		node->SetPosition(pos);
	}
}

