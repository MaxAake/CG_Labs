#version 410

// Remember how we enabled vertex attributes in assignment 2 and attached some
// data to each of them, here we retrieve that data. Attribute 0 pointed to the
// vertices inside the OpenGL buffer object, so if we say that our input
// variable `vertex` is at location 0, which corresponds to attribute 0 of our
// vertex array, vertex will be effectively filled with vertices from our
// buffer.
// Similarly, normal is set to location 1, which corresponds to attribute 1 of
// the vertex array, and therefore will be filled with normals taken out of our
// buffer.
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;

#define WAVES 2
uniform float amps[WAVES];
uniform float dirs_x[WAVES];
uniform float dirs_z[WAVES];
uniform float freqs[WAVES];
uniform float phases[WAVES];
uniform float sharpnesses[WAVES];
uniform float elapsed_time_s;


uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform vec3 camera_position;
// This is the custom output of this shader. If you want to retrieve this data
// from another shader further down the pipeline, you need to declare the exact
// same structure as in (for input), with matching name for the structure

// members and matching structure type. Have a look at
// shaders/EDAF80/diffuse.frag.
out VS_OUT {
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
} vs_out;


void waveFunction(in float amp, in float dir_x, in float dir_z, in float freq, in float phase, in float sharpness, inout float y, inout float dx, inout float dz)
{
	float a_t = sin(((dir_x * vertex.x) + (dir_z * vertex.z)) * freq + elapsed_time_s * phase) * 0.5 + 0.5;
	y += amp * pow(a_t, sharpness);
	dx += 0.5*sharpness*freq*amp*pow(a_t, sharpness-1) * cos(((dir_x * vertex.x) + (dir_z * vertex.z)) * freq + elapsed_time_s * phase)*dir_x;
	dz += 0.5*sharpness*freq*amp*pow(a_t, sharpness-1) * cos(((dir_x * vertex.x) + (dir_z * vertex.z)) * freq + elapsed_time_s * phase)*dir_z;
}

void main()
{
	vec2 texScale = vec2(8, 4);
	float normalTime = mod(elapsed_time_s, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0.0);
	vs_out.normalCoord0 = texcoords.xy * texScale + normalTime * normalSpeed;
	vs_out.normalCoord1 = texcoords.xy * texScale * 2 + normalTime * normalSpeed * 4;
	vs_out.normalCoord2 = texcoords.xy * texScale * 4 + normalTime * normalSpeed * 8;
	//for (int i = 0; i < WAVES; i++) {
		//float temp_y = 0.0;
		// waveFunction(amps[i], dirs_x[i], dirs_z[i], freqs[i], phases[i], sharpnesses[i], temp_y);
		
		//y = y + temp_y;
	//}
	vs_out.TBN = mat3(vec3(tangent), vec3(binormal), vec3(normal));
	vec4 color_deep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 color_shallow = vec4(0.0, 0.5, 0.5, 1.0);
	float y = 0.0;
	float dx = 0.0;
	float dz = 0.0;
	waveFunction(1, -1, 0, 0.2, 0.5, 2, y, dx, dz);
	waveFunction(0.5, -0.7, 0.7, 0.4, 1.3, 2, y, dx, dz);
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex.x, y, vertex.z, 1.0));
	vs_out.fV = normalize(camera_position - vs_out.vertex);
	vs_out.fN = normalize(vec3(-dx, 1, -dz));
	float facing = 1 - max(dot(vs_out.fV, vs_out.fN), 0);
	vs_out.color = mix(color_deep, color_shallow, facing);
	vs_out.dx = dx;
	vs_out.dz = dz;
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4((vec3(vertex.x, vertex.y, vertex.z) + y*normalize(normal)), 1.0);
}



