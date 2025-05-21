#version 330

out vec4 pixelColor; //Output variable. Almost final pixel color.

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

//Varying variables
in vec4 ic;
in vec4 n;
in vec4 l;
in vec4 v;
in vec2 iTexCoord0;

void main(void) {
	//Normalized, interpolated vectors
	vec4 ml = normalize(l);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);
	//Reflected vector
	vec4 mr = reflect(-ml, mn);

	vec4 texColor = texture(textureMap0, iTexCoord0);
	vec4 texColor1 = texture(textureMap1, (mn.xy + 1)/2);

	//Surface parameters
	vec4 kd = mix(texColor, texColor1, 0.9);
	vec4 ks = vec4(1,1,1,1);

	//Lighting model computation
	float nl = clamp(dot(mn, ml), 0, 1);
	float rv = pow(clamp(dot(mr, mv), 0, 1), 50);
	pixelColor = vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb * rv, 0);
}
