#version 450 core
layout (location = 0) in vec3 aPos;
out vec4 vertexColor;

void main()
{
    vertexColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
