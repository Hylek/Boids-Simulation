#include "Particles.h"

Particles::Particles()
{

}

Particles::~Particles()
{

}

void Particles::InitPlayerTag(ResourceCache* cache, Scene* scene, Node* playerObject, int clientCount)
{
	Sprite2D* playerOne = cache->GetResource<Sprite2D>("Urho2D/playerOne.png");
	Sprite2D* playerTwo = cache->GetResource<Sprite2D>("Urho2D/playerTwo.png");

	SharedPtr<Node> tagNode(scene->CreateChild("Tag"));
	tagNode->SetPosition(Vector3(playerObject->GetPosition().x_, playerObject->GetPosition().y_ + 5.0f, playerObject->GetPosition().z_));
	tagNode->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));
	tagNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));

	StaticSprite2D* staticSprite = tagNode->CreateComponent<StaticSprite2D>();

	// Set blend mode
	staticSprite->SetBlendMode(BLEND_ALPHA);

	if (clientCount < 2)
	{
		staticSprite->SetSprite(playerOne);
	}
	if (clientCount == 2)
	{
		staticSprite->SetSprite(playerTwo);
	}
	newTagNode = tagNode;
}

void Particles::InitBubbles(ResourceCache* cache, Scene* scene, Graphics* graphics, float xPos, float zPos)
{
	// Urho3D Sample Project 24
	Sprite2D* sprite = cache->GetResource<Sprite2D>("Urho2D/bubble.png");

	if (!sprite) return;

	for (unsigned i = 0; i < NUM_SPRITES; ++i)
	{
		float scaleAmount = Random(0.2f);
		SharedPtr<Node> spriteNode(scene->CreateChild("Bubble", LOCAL));
		spriteNode->SetPosition(Vector3(Random(xPos, xPos + 1.0f), Random(-100.0f), zPos));
		spriteNode->SetRotation(Quaternion(0.0f, Random(0.0f, 360.0f), 0.0f));
		spriteNode->SetScale(Vector3(scaleAmount, scaleAmount, scaleAmount));

		StaticSprite2D* staticSprite = spriteNode->CreateComponent<StaticSprite2D>();

		// Set blend mode
		staticSprite->SetBlendMode(BLEND_ALPHA);

		// Set sprite
		staticSprite->SetSprite(sprite);

		// Add to sprite node vector
		bubbleNodes.Push(spriteNode);
	}
}

void Particles::InitWeeds(ResourceCache* cache, Scene* scene, Graphics* graphics, float xPos)
{
	Sprite2D* sprite = cache->GetResource<Sprite2D>("Urho2D/seaweed.png");

	if (!sprite) return;

	for (unsigned i = 0; i < NUM_SPRITES; ++i)
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
	for (unsigned i = 0; i < bubbleNodes.Size(); ++i)
	{
		SharedPtr<Node> node = bubbleNodes[i];

		Vector3 position = node->GetPosition();
		Vector3 newPosition = position + Vector3(Vector3::UP) * timeStep * 2;
		if (newPosition.x_ < 0.0f || newPosition.x_ > 4.0f)
		{
			newPosition.x_ = position.x_;
		}
		if (newPosition.y_ > 90.0f)
		{
			newPosition.y_ = Random(-25.0f);
		}
		node->SetPosition(newPosition);
	}
}

void Particles::UpdateSeaWeed(float timeStep)
{
	for (unsigned i = 0; i < weedNodes.Size(); ++i)
	{
		SharedPtr<Node> node = weedNodes[i];

		Vector3 newPosition = node->GetPosition() + Vector3::UP  * timeStep * 2;
		if (newPosition.y_ > 90.0f)
		{
			newPosition.y_ = Random(-25.0f);
		}
		node->SetPosition(newPosition);
	}
}

void Particles::UpdateTags(Node* playerObject)
{
	newTagNode->SetPosition(Vector3(playerObject->GetPosition().x_, playerObject->GetPosition().y_ + 5.0f, playerObject->GetPosition().z_));
	newTagNode->SetRotation(Quaternion(0.0f, 0.0f, playerObject->GetRotation().z_));
}

