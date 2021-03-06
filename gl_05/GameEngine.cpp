#include "GameEngine.h"

//only for debug purposes
ostream& operator<<(ostream& os, const glm::mat4& mx)
{
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
			cout << mx[row][col] << ' ';
		cout << endl;
	}
	return os;
}

ostream& operator<<(ostream& os, const glm::vec4& mx)
{
	for (int row = 0; row < 4; ++row)
	{
		cout << mx[row] << ' ';
	}
	cout << endl;
	return os;
}

ostream& operator<<(ostream& os, const glm::vec3& mx)
{
	for (int row = 0; row < 3; ++row)
	{
		cout << mx[row] << ' ';
	}
	cout << endl;
	return os;
}

ostream& operator<<(ostream& os, const glm::vec2& mx)
{
	for (int row = 0; row < 2; ++row)
	{
		cout << mx[row] << ' ';
	}
	cout << endl;
	return os;
}

GameEngine* GameEngine::instance = nullptr;

GameEngine& GameEngine::getInstance()
{
	if (instance == nullptr)
	{
		instance = new GameEngine();
	}

	return *instance;
}

GameEngine::GameEngine()
{
	cameraPosition = glm::vec3(34.43f, -3.23f, 22.13f);
	cameraRotation = glm::vec2(144.4f, 15);
}

void GameEngine::init()
{
	if (glfwInit() != GL_TRUE)
		throw exception("GLFW initialization failed");
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void GameEngine::run()
{
	try
	{
		window = glfwCreateWindow(screenWidth, screenHeight, "GKOM - OpenGL 05", nullptr, nullptr);
		if (window == nullptr)
			throw exception("GLFW window not created");
		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwSetKeyCallback(window, key_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetWindowSizeCallback(window, window_size_callback);
		glfwSetCursorPosCallback(window, cursor_position_callback);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			throw exception("GLEW Initialization failed");
		}

		glViewport(0, 0, screenWidth, screenHeight);

		glEnable(GL_DEPTH_TEST);

		// Let's check what are maximum parameters counts
		GLint nrAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		cout << "Max vertex attributes allowed: " << nrAttributes << std::endl;
		glGetIntegerv(GL_MAX_TEXTURE_COORDS, &nrAttributes);
		cout << "Max texture coords allowed: " << nrAttributes << std::endl;

		// Build, compile and link shader program
		theProgram = new ShaderProgram("gl_05.vert", "gl_05.frag");
		lightSrcProgram = new ShaderProgram("lightSrc.vert", "lightSrc.frag");
		waterProgram = new ShaderProgram("water.vert", "water.frag");
		waterProgram->Use();
		glUniform1i(glGetUniformLocation(waterProgram->get_programID(), "skybox"), 0);
		lightIntensity = 0.0f;

		Skybox* skybox = new Skybox();
		gameScene = new WellScene(theProgram, lightSrcProgram, waterProgram);
		Light* light = new Light();
		gameScene->startScene();
		skybox->start();
		cubemapTexture = skybox->getCubemapTexture();

		handleScreenResizeEvent(screenWidth, screenHeight);

		currentFrame = glfwGetTime();
		lastFrame = currentFrame;
		deltaTime = 0;

		// main event loop

		while (!glfwWindowShouldClose(window))
		{
			// Clear the colorbuffer
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			light->update(lightIntensity, theProgram, lightSrcProgram, cameraPosition);

			keyboardManager.nextFrame();
			mouseManager.nextFrame();
			// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
			glfwPollEvents();
			
			handleKeyboardEvent();
			handleMouseEvent();

			updateDeltaTime();

			// Clear the colorbuffer
			glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			std::pair<glm::mat4, glm::mat4> camera = setCamera();	//contains view and projection

			// Draw our first triangle
			waterProgram->Use();
			glUniform3f(glGetUniformLocation(waterProgram->get_programID(), "cameraPos"), cameraPosition.x, cameraPosition.y + 30, cameraPosition.z);
			theProgram->Use();

			gameScene->updateScene((float) deltaTime, controll);
			gameScene->render(cubemapTexture);

			//draw skybox
			skybox->render(camera.first, camera.second);

			theProgram->Use();

			// Swap the screen buffers
			glfwSwapBuffers(window);
		}

		skybox->destroy();
		gameScene->destroyScene();
		delete skybox;
		delete gameScene;

		delete theProgram;
		delete lightSrcProgram;
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
		system("pause");
	}
	glfwTerminate();
}

std::pair<glm::mat4, glm::mat4> GameEngine::setCamera()
{
	glm::mat4 view;
	view = glm::rotate(view, glm::radians(cameraRotation[1]), glm::vec3(1, 0, 0));
	view = glm::rotate(view, glm::radians(cameraRotation[0]), glm::vec3(0, 1, 0));
	view = glm::translate(view, cameraPosition);
	theProgram->Use();
	GLuint viewLoc = glGetUniformLocation(theProgram->get_programID(), "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	lightSrcProgram->Use();
	viewLoc = glGetUniformLocation(lightSrcProgram->get_programID(), "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	waterProgram->Use();
	viewLoc = glGetUniformLocation(waterProgram->get_programID(), "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(70.0f), (float)screenWidth / (float)screenHeight, 0.1f, 300.0f);
	theProgram->Use();
	GLuint projLoc = glGetUniformLocation(theProgram->get_programID(), "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	lightSrcProgram->Use();
	projLoc = glGetUniformLocation(lightSrcProgram->get_programID(), "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	waterProgram->Use();
	projLoc = glGetUniformLocation(waterProgram->get_programID(), "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	return std::pair<glm::mat4, glm::mat4> (view, projection);
}

void GameEngine::updateDeltaTime()
{
	currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void GameEngine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	getInstance().keyboardManager.keyStateChanged(key, action);
}

void GameEngine::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

void GameEngine::window_size_callback(GLFWwindow* window, int width, int height)
{
	getInstance().handleScreenResizeEvent(width, height);
}

void GameEngine::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	getInstance().mouseManager.mousePosChanged(xpos, ypos);
}

void GameEngine::handleKeyboardEvent()
{
	if (keyboardManager.wasPressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, GL_TRUE);

	glm::vec4 cameraMovement = glm::vec4(0.0f);

	if (keyboardManager.isHold(GLFW_KEY_W) || keyboardManager.isHold(GLFW_KEY_UP))
		cameraMovement += glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	
	if (keyboardManager.isHold(GLFW_KEY_S) || keyboardManager.isHold(GLFW_KEY_DOWN))
		cameraMovement += glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

	if (keyboardManager.isHold(GLFW_KEY_A) || keyboardManager.isHold(GLFW_KEY_LEFT))
		cameraMovement += glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

	if (keyboardManager.isHold(GLFW_KEY_D) || keyboardManager.isHold(GLFW_KEY_RIGHT))
		cameraMovement += glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);

	if (keyboardManager.isHold(GLFW_KEY_E))
		cameraMovement += glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

	if (keyboardManager.isHold(GLFW_KEY_Q))
		cameraMovement += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	if (keyboardManager.isHold(GLFW_KEY_O))
		decreaseLight();

	if (keyboardManager.isHold(GLFW_KEY_P))
		increaseLight();

	if (keyboardManager.isHold(GLFW_KEY_I))
		controll = 1;

	if (keyboardManager.isHold(GLFW_KEY_U))
		controll = -1;

	if (!keyboardManager.isHold(GLFW_KEY_I) && !keyboardManager.isHold(GLFW_KEY_U))
		controll = 0;

	glm::normalize(cameraMovement);

	if (keyboardManager.isHold(GLFW_KEY_LEFT_SHIFT) || keyboardManager.isHold(GLFW_KEY_RIGHT_SHIFT))
		cameraMovement *= 5.0f;

	cameraMovement += glm::vec4(0, 0, 0, 1);

	glm::mat4 rotateTransform;
	rotateTransform = glm::rotate(rotateTransform, glm::radians(-cameraRotation[0]), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateTransform = glm::rotate(rotateTransform, glm::radians(-cameraRotation[1]), glm::vec3(1.0f, 0.0f, 0.0f)); 

	cameraMovement = rotateTransform * cameraMovement;

	cameraPosition += glm::vec3(cameraMovement) * 3.0f *(float)deltaTime;
}

void GameEngine::handleMouseEvent()
{
	cameraRotation[0] += (float) mouseManager.getDeltaX() * 0.1f;
	cameraRotation[1] += (float)mouseManager.getDeltaY() * 0.1f;

	if (cameraRotation[0] > 360) cameraRotation[0] -= 360;
	if (cameraRotation[0] < 0) cameraRotation[0] += 360;

	if (cameraRotation[1] > 88) cameraRotation[1] = 88;
	if (cameraRotation[1] < -88) cameraRotation[1] = -88;
}

void GameEngine::handleScreenResizeEvent(int width, int height)
{
	screenWidth = width;
	screenHeight = height;

	glViewport(0, 0, screenWidth, screenHeight);
}

void GameEngine::decreaseLight()
{
	if ((lightIntensity -= 0.01f) <= 0.0f)
		lightIntensity = 0.0f;

}

void GameEngine::increaseLight()
{
	if ((lightIntensity += 0.01f) >= 1.0f)
		lightIntensity = 1.0f;
}