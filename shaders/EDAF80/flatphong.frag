#version 410

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;
uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_map;
uniform bool use_normal_mapping;

in VS_OUT {
	vec3 worldPos;
	flat vec3 fN;
	flat vec3 fV;
	flat vec3 fL;
	flat mat3 normal_model_to_world;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 dx = dFdx(fs_in.worldPos);
	vec3 dy = dFdy(fs_in.worldPos);
	vec3 N = normalize(cross(dx, dy));
	vec3 L = normalize(fs_in.fL);
	vec3 V = normalize(fs_in.fV);
	vec3 R = normalize(reflect(-L, N));
	vec3 diffuse = diffuse_colour*(1-max(dot(-N,L), 0.0));
	vec3 specular = specular_colour*pow(max(dot(R,V), 0.0), shininess_value);
	frag_color = vec4(ambient_colour + diffuse + specular, 1.0);
}
