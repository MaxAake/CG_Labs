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
	vec3 fN;
	vec3 fV;
	vec3 fL;
	vec2 texcoords;
	mat3 TBN;
	mat3 normal_model_to_world;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 N;
	if(use_normal_mapping) {
	vec4 n = (texture(normal_map, fs_in.texcoords))*2-1;
	N = normalize((fs_in.normal_model_to_world * fs_in.TBN * n.xyz));
	}
	else {
		N = normalize(fs_in.fN);
	}
	vec3 L = normalize(fs_in.fL);
	vec3 V = normalize(fs_in.fV);
	vec3 R = normalize(reflect(-L, N));
	vec3 diffuse_texread = texture(diffuse_texture, fs_in.texcoords).xyz;
	vec3 specular_texread = texture(specular_texture, fs_in.texcoords).xyz;
	vec3 diffuse = diffuse_texread*max(dot(N,L), 0.0);
	vec3 specular = specular_texread*pow(max(dot(R,V), 0.0), shininess_value);
	frag_color = vec4(ambient_colour + diffuse + specular, 1.0);
}
