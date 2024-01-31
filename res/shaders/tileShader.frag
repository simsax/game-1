#version 430 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 vColor;
const float borderThickness = 0.05;

void main()
{
    float left = step(borderThickness, texCoord.x);
    float bottom = step(borderThickness, texCoord.y);
    float right = step(texCoord.x, 1 - borderThickness);
    float top = step(texCoord.y, 1 - borderThickness);

    vec3 mask = vec3(left * bottom * right * top);

    FragColor = vec4(vColor, 1) * vec4(mask, 1);

    // debug texCoords
    // FragColor = vec4(texCoord, 0, 1);
} 
