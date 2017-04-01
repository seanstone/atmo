#include <PulsarEngine.hpp>
#include <PulsarMesh.hpp>
#include <PulsarShader.hpp>
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
	float fovDeg = 65;
	float elevation = 1000;
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

	atmoShader->Shader::bind();
	atmoShader->setParameter("WindowSize", vec2(width, height));
	atmoShader->setParameter("sunAngle", float(sunAngleDeg * M_PI / 180.0));
	atmoShader->setParameter("fov", float(fovDeg * M_PI / 180.0));
	atmoShader->setParameter("elevation", elevation);
	atmoShader->Shader::unbind();
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
			sunAngleDeg += event->wheel.y * 2;
			if (sunAngleDeg < -120)
				sunAngleDeg = 1200;
			if (sunAngleDeg > 120)
				sunAngleDeg = 120;
			printf("sunAngle: %f\r\n", sunAngleDeg);
			atmoShader->Shader::bind();
			atmoShader->setParameter("sunAngle", float(sunAngleDeg * M_PI / 180.0));
			atmoShader->Shader::unbind();
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event->button.button == SDL_BUTTON_LEFT)
			{
				printf("left: %d, %d\r\n", event->button.x, event->button.y);
				elevation *= 0.9;
				atmoShader->setParameter("elevation", elevation);
				printf("elevation: %f\r\n", elevation);
			}
			else if (event->button.button == SDL_BUTTON_RIGHT)
			{
				printf("right: %d, %d\r\n", event->button.x, event->button.y);
				elevation *= 1.1;
				atmoShader->setParameter("elevation", elevation);
				printf("elevation: %f\r\n", elevation);
			}
		}
	}
}

int main()
{
	MainWindow window;
	window.startLoop();
	window.terminate();

	return 0;
}
