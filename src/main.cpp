#include <PulsarEngine.hpp>
#include <PulsarMesh.hpp>
#include <PulsarShader.hpp>
#include <PulsarCamera.hpp>
#include "AtmoShader.hpp"
using namespace Pulsar;

#include <iostream>
#include <string>
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

class MainWindow : public Window
{
public:
	MainWindow();
	~MainWindow();
	void render();
	void update();
	void handleEvent(SDL_Event* event);

	int width, height;

protected:
	AtmoShader* atmoShader;
	float sunAngleDeg = 87;
	PerspectiveCamera camera;
};

MainWindow::MainWindow()
{
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
	createWindow(width, width, "PulsarEngine Example");

	bool result = true;
	atmoShader = new AtmoShader;
	result &= atmoShader->loadVertexShader("data/shader/atmo.vs");
	result &= atmoShader->loadFragmentShader("data/shader/atmo.fs");
	result &= atmoShader->compile();
	if(result == false)
	{
		terminate();
		exit(0);
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_FRAMEBUFFER_SRGB);

	camera.fov 			= M_PI * 65.0 / 180.0;
	camera.position 	= vec3(0, 0, -15000e3);
	camera.right 		= vec3(1, 0, 0);
	camera.up			= vec3(0, 1, 0);

	atmoShader->Shader::bind();
	atmoShader->setParameter("WindowSize", vec2(width, height));
	atmoShader->setParameter("sunAngle", float(sunAngleDeg * M_PI / 180.0));
	atmoShader->setParameter("cam_pos", 	camera.position);
	atmoShader->setParameter("cam_right", 	camera.right);
	atmoShader->setParameter("cam_up", 		camera.up);
	atmoShader->setParameter("cam_fov", 	camera.fov);
}

MainWindow::~MainWindow()
{
}

void MainWindow::render()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	atmoShader->render();
}

void MainWindow::update()
{
	// if(getKeyState(Key::Esc) == true)
	// 	exit(0);

	if (getKeyState(Key::X) == true)
	{
		sunAngleDeg ++;
		if (sunAngleDeg > 360)
			sunAngleDeg -= 360;
		atmoShader->Shader::bind();
		atmoShader->setParameter("sunAngle", float(sunAngleDeg * M_PI / 180.0));
		printf("sunAngle: %f\r\n", sunAngleDeg);
	}

	if (getKeyState(Key::Z) == true)
	{
		sunAngleDeg --;
		if (sunAngleDeg < 0)
			sunAngleDeg += 360;
		atmoShader->Shader::bind();
		atmoShader->setParameter("sunAngle", float(sunAngleDeg * M_PI / 180.0));
		printf("sunAngle: %f\r\n", sunAngleDeg);
	}

	if (getKeyState(Key::Up) == true)
	{
		camera.position *= (float)(1.001);
		atmoShader->Shader::bind();
		atmoShader->setParameter("cam_pos", camera.position);
		printf("elevation: %f\r\n", length(camera.position));
	}

	if (getKeyState(Key::Down) == true)
	{
		vec3 newpos = camera.position * 0.999f;
		if (length(newpos) > 6360e3)
			camera.position = newpos;
		atmoShader->Shader::bind();
		atmoShader->setParameter("cam_pos", camera.position);
		printf("elevation: %f\r\n", length(camera.position));
	}

	vec2 mousePos = getMousePos();
	if(getMouseState(Button::ButtonLeft) == true)
	{
	}
}

void MainWindow::handleEvent(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_MOUSEWHEEL:
		{
			printf("wheel: %d, %d\r\n", event->wheel.x, event->wheel.y);
			vec3 newpos = camera.position * (float)(1.0 - event->wheel.y * 0.01);
			if (length(newpos) > 6360e3)
				camera.position = newpos;
			atmoShader->Shader::bind();
			atmoShader->setParameter("cam_pos", camera.position);
			printf("elevation: %f\r\n", length(camera.position));
		} break;
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event->button.button == SDL_BUTTON_LEFT)
			{
				printf("left: %d, %d\r\n", event->button.x, event->button.y);
			}
			else if (event->button.button == SDL_BUTTON_RIGHT)
			{
				printf("right: %d, %d\r\n", event->button.x, event->button.y);
			}
		} break;
		case SDL_MOUSEMOTION:
		{
			if (event->motion.state & SDL_BUTTON_LMASK)
			{
				if (abs(event->motion.xrel) > abs(event->motion.yrel))
				{
					mat4 R = rotate((float) (event->motion.xrel) / (float)500.0, -camera.position);
					camera.right = vec3(R * vec4(camera.right, 1));
					camera.direction = vec3(R * vec4(camera.direction, 1));
					camera.up = normalize(cross(camera.right, camera.direction));
				}
				else
				{
					mat4 R = rotate((float) (event->motion.yrel) / (float)500.0, -camera.right);
					camera.up = vec3(R * vec4(camera.up, 1));
					camera.direction = vec3(R * vec4(camera.direction, 1));
					camera.right = normalize(cross(camera.direction, camera.up));
				}
				atmoShader->setParameter("cam_right", 	camera.right);
				atmoShader->setParameter("cam_up", 		camera.up);
				printf("left move: %d, %d\r\n", event->motion.xrel, event->motion.yrel);
			}
			else if (event->motion.state & SDL_BUTTON_RMASK)
			{
				printf("right move: %d, %d\r\n", event->motion.xrel, event->motion.yrel);
			}
		} break;
	}
}

int main()
{
	MainWindow window;
	window.startLoop();
	window.terminate();

	return 0;
}
