#version 330 core

in vec2 vTexCoord;
uniform sampler2D textureMap0;

out vec4 outColor;

void main() {
    outColor = texture(textureMap0, vTexCoord);
}