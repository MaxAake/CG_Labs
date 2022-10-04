#version 410

uniform vec3 light_position;
uniform samplerCube cubemap;

in VS_OUT {
	vec3 vertex;
	vec3 fN;
	vec3 fV;
	vec4 color;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 n = normalize(fs_in.fN);
	vec3 v = normalize(fs_in.fV);
	vec3 r = reflect(-v, n);
	vec4 reflection = texture(cubemap, r);
	frag_color = fs_in.color + reflection;
}
