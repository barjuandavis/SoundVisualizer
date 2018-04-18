#version 330 core

layout(location=0) in vec3 vVertex;		//object space vertex position

out vec3 customColor;
//uniforms
uniform mat4 MVP;		//combined modelview projection matrix
uniform float time;		//elapsed time

//shader constants
const float amplitude = 0.3;
const float frequency = 2;
const float PI = 3.14159;


void main()
{
	float distance = length(vVertex);
	float y = amplitude*cos(-PI*distance*frequency+log(time*(sin(time)/2+0.5)));
	gl_Position = MVP*vec4(vVertex.x, y, vVertex.z,1);
	customColor = vec3(sin(time)/2*vVertex.x,sin(time+2*PI/3)/2*y,sin(time-2*PI/3)/2*vVertex.z);

}
