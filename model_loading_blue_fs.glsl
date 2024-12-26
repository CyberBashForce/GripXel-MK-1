#version 330

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

void main()
{
	FragColor = vec4(0.0f,0.8f,1.0f,1.0f);
}