#include "Ground.h"

Ground::Ground(GameObject* parent) : GameObject(parent)
{

}

void Ground::start() 
{
	setTexture("textures/grass.png");
	addMesh(new PlaneMesh(1000.0f,1000.0f,1.45f,glm::vec2(-30.0f,-20.0f),1000.0f,1000.0f));
}

void Ground::update(float delta)
{
	//just stay where you are
}