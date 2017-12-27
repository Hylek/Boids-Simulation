#include "Particles.h"

Particles::Particles()
{

}

Particles::~Particles()
{

}

void Particles::Init(ResourceCache * cache, Scene * scene, Graphics* graphics)
{
	Sprite2D* sprite = cache->GetResource<Sprite2D>("Urho2D/bubble.png");

	if (!sprite) return;

	for (unsigned i = 0; i < NUM_SPRITES; ++i)
	{
		SharedPtr<Node> spriteNode(scene->CreateChild("StaticSprite2D", LOCAL));
		spriteNode->SetPosition(Vector3(Random(10.0f), Random(10.0f), 0.0f));
		spriteNode->SetScale(Vector3(0.2f, 0.2f, 0.2f));

		StaticSprite2D* staticSprite = spriteNode->CreateComponent<StaticSprite2D>();

		// Set random color
		//staticSprite->SetColor(Color(Random(1.0f), Random(1.0f), Random(1.0f), 1.0f));

		// Set blend mode
		staticSprite->SetBlendMode(BLEND_ALPHA);
		// Set sprite
		staticSprite->SetSprite(sprite);

		// Add to sprite node vector
		spriteNodes_.Push(spriteNode);
	}
}

void Particles::Update(StringHash eventType, VariantMap & eventData, SharedPtr<Node> spriteNode)
{
	// Set move speed
	spriteNode->SetVar(VAR_MOVESPEED, Vector3(Random(-2.0f, 2.0f), Random(-2.0f, 2.0f), 0.0f));
	// Set rotate speed
	spriteNode->SetVar(VAR_ROTATESPEED, Random(-90.0f, 90.0f));
}
