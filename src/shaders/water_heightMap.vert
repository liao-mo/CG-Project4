#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const float pi = 3.14159;
uniform float time;
uniform vec3 EyePos;
uniform sampler2D heightMap;

float waveHeight(float x, float y) {
    float height = 0.0;
//    for (int k = 0; k < numWaves; k++)
//        height += wave(k, x, y);
    return height;
}


vec3 waveNormal(float x, float y) {
//    float dx = 0.0;
//    float dy = 0.0;
//    for (int i = 0; i < numWaves; ++i) {
//        dx += dWavedx(i, x, y);
//        dy += dWavedy(i, x, y);
//    }
    vec3 n = vec3(1.0, 1.0, 1.0);
    return normalize(n);
}

void main()
{
    vec3 temp_pos;


    temp_pos = aPos;
    temp_pos.y = waveHeight(temp_pos.x,temp_pos.z);
    FragPos = vec3(model * vec4(temp_pos, 1.0));
    Normal = waveNormal(temp_pos.x,temp_pos.z);
    Normal = mat3(transpose(inverse(model))) * Normal;

    gl_Position = projection * view * model * vec4(temp_pos, 1.0);
}