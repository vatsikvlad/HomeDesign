#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord0;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec2 vTexCoord;

void main() {
    gl_Position = P * V * M * vertex;
    vTexCoord = texCoord0;
}