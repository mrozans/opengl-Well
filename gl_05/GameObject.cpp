#include "GameObject.h"

GameObject::GameObject(GameObject* parent) : parent(parent), texture(nullptr)
{
	visible = true;
	if (parent != nullptr)
	{
		parent->addChild(this);
		scene = parent->scene;
		scene->registerObject(this);
	}
}

GameObject::GameObject(GameScene *scene) : scene(scene), texture(nullptr)
{
	visible = true;
	scene->registerObject(this);
}

GameObject::~GameObject()
{

}

void GameObject::renderObject(const glm::mat4 &parentTransform, ShaderProgram* shader, ShaderProgram* shader2, ShaderProgram *shader3, GLuint cubemapTexture)
{
	if (!visible)
		return;
	glm::mat4 transformMatrix = transform.getTransform(parentTransform);
	scene->setTransform(transformMatrix);

	if (texture != nullptr)
		texture->use();

	for (const auto& mesh : meshes) 
	{
		if (mesh->getWater()) shader3->Use();
		else if (mesh->getLight()) shader2->Use();
		else shader->Use();
		mesh->render(cubemapTexture);
	}

	render();

	for (auto &child : children)
		child->renderObject(transformMatrix, shader, shader2, shader3, cubemapTexture);
}

void GameObject::updateObject(float delta, GLint controll)
{
	if (firstUpdate)
	{
		firstUpdate = false;
		startObject();
	}
	update(delta);
	update(delta, controll);

	for (auto &child : children)
	{
		child->updateObject(delta, controll);
	}
}

void GameObject::startObject()
{
	start();

	for (auto &mesh : meshes)
	{
		mesh->start();
	}
}

void GameObject::destroyObject()
{
	for (auto &child : children)
	{
		child->destroyObject();
		delete child;
	}

	parent->removeChild(this);

	for (auto &mesh : meshes)
	{
		mesh->destroy();
		delete mesh;
	}

	destroy();
}

void GameObject::render()
{

}

void GameObject::update(float delta)
{

}

void GameObject::update(float delta, GLint controll)
{

}

void GameObject::start()
{

}

void GameObject::destroy()
{

}

void GameObject::addChild(GameObject *child)
{
	if (child->parent != nullptr)
		child->parent->removeChild(child);

	auto itGameObject = find(children.begin(), children.end(), child);
	if (itGameObject != children.end()) return;

	children.push_back(child);
}

void GameObject::removeChild(GameObject *child)
{
	for (unsigned int i = 0; i < children.size(); ++i)
	{
		if (children[i] == child)
		{
			children[i] = children[children.size() - 1];
			children[children.size() - 1] = child;
			children.pop_back();
			break;
		}
	}
}

MeshRenderer* GameObject::addMesh(MeshRenderer *mesh)
{
	mesh->init();
	meshes.push_back(mesh);
	return mesh;
}

void GameObject::setTexture(const char *fileName)
{
	texture = new Texture(scene->getShader(), fileName);
}

GameObject* GameObject::getParent()
{
	return parent;
}

void GameObject::setVisible(bool isVisible)
{
	visible = isVisible;
}

GameObject* GameObject::getRoot()
{
	return scene->getRoot();
}

void GameObject::setParent(GameObject* aParent)
{
	this->parent = aParent;
	aParent->addChild(this);
}

std::vector<GameObject*>* GameObject::getChildren()
{
	if (children.size() != 0)
		return &children;

	return nullptr;
}

int GameObject::randomInt(int min, int max) const
{
	int d = max - min;
	return rand() % d + min;
}

float GameObject::randomFloat(float min, float max) const
{
	float d = max - min;
	return (float)(rand() % 1000) / 1000.0f * d + min;
}