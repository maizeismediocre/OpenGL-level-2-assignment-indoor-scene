#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;


void main(void) 
{
	outColor = color;
	outColor *= texture(texture0, texCoord0.st);
}
