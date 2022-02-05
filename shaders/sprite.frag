#version 330 core
in vec2 TexCoords;
out vec4 color;
in vec4 VertexPosition;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform int Lightoff;
uniform vec2 PlayerPosition;
void main()
{    
    color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
    float distance = length(VertexPosition.xy - PlayerPosition);
    if(Lightoff == 1 && distance > 150.0)
    {
        color = vec4(0.0, 0.0, 0.0, 0.0);
    }
}  