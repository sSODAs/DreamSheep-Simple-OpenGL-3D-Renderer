#version 330 core
in vec3 FragPos;
in vec2 ScreenUv;

out vec4 colour;

uniform vec4 bg_color;
uniform float time;

float rand(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

uniform float amplitude = 0.3;

float circle(vec2 center, float radius, vec2 p) {
    return length(p - center) - radius;
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm(in vec2 st) {
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;

    for(int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

void main() {
    vec2 screenUv = ScreenUv;

    float size = 0.08;
    float prob = 0.998;
    vec2 pos = floor(1.0 / size * FragPos.xy);
    float color = 0.0;
    float starValue = rand(pos);
    if(starValue > prob) {
        vec2 center = size * pos + vec2(size, size) * 0.5;
        float t = 0.9 + 0.2 * sin(time * 8.0 + (starValue - prob) / (1.0 - prob) * 45.0);
        color = 1.0 - distance(FragPos.xy, center) / (0.5 * size);
        color = color * t / (abs(FragPos.y - center.y)) * t / (abs(FragPos.x - center.x));
        if(color > 0.8)
            color = 0.8;
    } else if(rand(screenUv.xy / 20.0) > 0.996) {
        float r = rand(screenUv.xy);
        color = r * (0.85 * sin(time * (r * 5.0) + 720.0 * r) + 0.95);
    }
    if(color >= 0.0) {
        colour = vec4(vec3(color), 1.0) + bg_color;
    } else {
        colour = bg_color;
    }

    float noise = noise(FragPos.xy / 30.0);

    float sdf = circle(vec2(0.55, 0.5), 0.16, screenUv) + amplitude * noise;

    if(sdf <= 0.0) {
        colour = mix(colour, vec4(0.043, 0.251, 0.396, min(sdf * -200.0, 1.0)), 0.5);
    }

    colour += vec4(fbm(FragPos.xy * 0.2) * vec3(0.078, 0.133, 0.18), 1.0);
}