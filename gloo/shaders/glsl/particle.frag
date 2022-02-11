#version 330 core

uniform vec3 color1;
uniform vec3 color2;

in float lifespan;
out vec4 frag_color;

void main() {
    vec2 temp = gl_PointCoord - vec2(0.5);
	float f = dot(temp, temp);
	float rough_pct = sqrt(f)*2;
	if (f>0.25) discard;
	else if (rough_pct < .6) frag_color = vec4(color1, (1-rough_pct) * lifespan);
	else frag_color = vec4(color1 * (1-(rough_pct-.6)/.4) + color2 * (rough_pct-.6)/.4, (1-rough_pct) * lifespan);
}

