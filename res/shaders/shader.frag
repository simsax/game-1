#version 450 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

// uniform vec4 uColor;
uniform sampler2D ourTexture;

void main()
{
    //FragColor = vec4(ourColor, 1.0);
    //FragColor = uColor;
    FragColor = texture(ourTexture, texCoord);
} 
