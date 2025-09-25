#version 330 core

in vec2 texCoord;
in vec3 pos;

out vec4 FragColor;

uniform sampler2D texture1;

void main() {
	float dist = distance(texCoord, vec2(0.5, 0.5));
	if (dist > 0.5) {
		discard;
	}
	float alpha = mix(0.2, 1.0, dist * 2.0);
	vec4 color = vec4(0.5, 0.5, 0.9, 0.8 * alpha);
	FragColor = color;
}
