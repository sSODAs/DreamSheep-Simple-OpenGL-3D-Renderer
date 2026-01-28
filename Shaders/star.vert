
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 ScreenUv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);

    FragPos = vec3(model * vec4(pos, 1.0));
    ScreenUv = aTexCoord;
}