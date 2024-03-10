#version 430 core

out vec4 FragColor;
in vec3 vColor;

void main()
{
    FragColor = vec4(vColor, 0.5f);
} 
