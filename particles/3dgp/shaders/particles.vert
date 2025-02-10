#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;

void main()
{
	gl_Position = matrixProjection * vec4(0, 0, 0, 0);
}
