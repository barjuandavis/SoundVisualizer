#version 330 core

layout(location=0) out vec4 vFragColor;
in vec3 customColor;

uniform float time;
const float PI = 3.14159;

void main() {
	vFragColor = vec4(customColor+0.5,1);
}
