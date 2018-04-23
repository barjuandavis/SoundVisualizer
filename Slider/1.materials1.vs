#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

const float amplitude = 0.125;
const float frequency = 4;
const float PI = 3.14159;

varying float y;
uniform float t;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    float distance = length(aPos);

    y = amplitude*sin(-PI*distance*frequency+t)/(((abs(aPos.x)+abs(aPos.z)))+0.25f);

    float x = amplitude*sin(-PI*distance*frequency+t)/(((abs(aPos.y)+abs(aPos.z)))+0.25f);

    float z = amplitude*sin(-PI*distance*frequency+t)/(((abs(aPos.y)+abs(aPos.x)))+0.25f);

    FragPos = vec3(aPos.x-x, aPos.y-y, aPos.z);

    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view *model* vec4(FragPos, 1.0);
    gl_PointSize = 3.0f;
}
