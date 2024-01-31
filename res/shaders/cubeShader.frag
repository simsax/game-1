#version 430 core
out vec4 FragColor;

in vec3 vColor;
in vec2 texCoord;
const float borderThickness = 0.05;

void main()
{
    float left = step(borderThickness, texCoord.x);
    float bottom = step(borderThickness, texCoord.y);
    float right = step(texCoord.x, 1 - borderThickness);
    float top = step(texCoord.y, 1 - borderThickness);
    vec3 colorMask = vec3(left * bottom * right * top);
    vec4 fcolor = vec4(vColor, 1.0);
    FragColor = fcolor * vec4(colorMask, 1.0);
} 
