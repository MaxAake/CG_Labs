#version 410

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;

in VS_OUT {
	vec3 fN;
	vec3 fV;
	vec3 fL;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 N = normalize(fs_in.fN);
	vec3 L = normalize(fs_in.fL);
	vec3 V = normalize(fs_in.fV);
	vec3 R = normalize(reflect(-L, N));
	vec3 diffuse = diffuse_colour*max(dot(N,L), 0.0);
	vec3 specular = specular_colour*pow(max(dot(R,V), 0.0), shininess_value);
	frag_color = vec4(ambient_colour + diffuse + specular, 1.0);
}
