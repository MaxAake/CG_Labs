#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform mat4 normal_model_to_world;
uniform vec3 light_position;
uniform vec3 camera_position;



out VS_OUT {
	vec3 fN;
	vec3 fV;
	vec3 fL;
	vec2 texcoords;
	mat3 TBN;
	mat3 normal_model_to_world;
} vs_out;


void main()
{
	vec3 worldPos = (vertex_model_to_world*vec4(vertex, 1)).xyz;
	vs_out.fN = (normal_model_to_world*vec4(normal,0)).xyz;
	vs_out.fV = (camera_position - worldPos);
	vs_out.fL = (light_position - worldPos);
	vs_out.texcoords = vec2(texcoords.xy);
	vs_out.TBN = mat3(tangent, binormal, normal);
	vs_out.normal_model_to_world = mat3(vec3(normal_model_to_world[0].xyz), vec3(normal_model_to_world[1].xyz), vec3(normal_model_to_world[2].xyz));

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}
