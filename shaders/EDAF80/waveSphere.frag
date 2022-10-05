#version 410

uniform vec3 light_position;
uniform samplerCube cubemap;
uniform sampler2D normalmap;
uniform float elapsed_time_s;

uniform mat4 normal_model_to_world;

in VS_OUT {
	vec3 vertex;
	vec3 fN;
	vec3 fV;
	float dx;
	float dz;
	vec4 color;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
	mat3 TBN;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 n = normalize(vec3(-fs_in.dx, 1.0, -fs_in.dz));
	vec3 v = normalize(fs_in.fV);
	vec3 t = normalize(vec3(1.0, fs_in.dx, 0.0));
	vec3 b = normalize(vec3(0.0, fs_in.dz, 1.0));
	mat3 tbn = mat3(t, b, n);
	vec3 n0 = texture(normalmap, fs_in.normalCoord0).xyz * 2 - 1;
	vec3 n1 = texture(normalmap, fs_in.normalCoord1).xyz * 2 - 1;
	vec3 n2 = texture(normalmap, fs_in.normalCoord2).xyz * 2 - 1;
	vec3 n_bump = normalize(n0 + n1 + n2);
	vec3 mappedNormal = mat3(normal_model_to_world) * fs_in.TBN * tbn * n_bump;
	float fresnel = 0.02037 + (1 - 0.02037) * pow((1 - dot(v, mappedNormal)), 5);
	vec3 r = reflect(-v, mappedNormal);
	vec4 refraction = texture(cubemap, refract(-v, mappedNormal, 1.0/1.33));
	vec4 reflection = texture(cubemap, r);
	frag_color = fs_in.color + reflection * fresnel + refraction * (1-fresnel);
}
