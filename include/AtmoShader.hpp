#ifndef ATMOSHADER_HPP
#define ATMOSHADER_HPP

#include <PulsarShader.hpp>
#include <PulsarTexture.hpp>

typedef GLuint VAO;
typedef GLuint VBO;

namespace Pulsar
{

class AtmoShader : public Shader
{
public:
	AtmoShader();
	~AtmoShader();
	void render();

    VAO vao = 0;
	VBO	vbo = 0;
};

}

#endif
