#include "AtmoShader.hpp"

using namespace Pulsar;

AtmoShader::AtmoShader()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);

    vec2 vertexData[6] =
    {
        vec2(-1, -1),
        vec2(1, -1),
        vec2(-1, 1),
        vec2(1, -1),
        vec2(-1, 1),
        vec2(1, 1)
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vec2), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

AtmoShader::~AtmoShader()
{
	if (vao)           glDeleteVertexArrays(1, &vao);
	if (vbo)           glDeleteBuffers(1, &vbo);
}

void AtmoShader::render()
{
    glUseProgram(program);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}
