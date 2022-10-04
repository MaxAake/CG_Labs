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
	vec4 color;
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
	//for (int i = 0; i < WAVES; i++) {
		//float temp_y = 0.0;
		// waveFunction(amps[i], dirs_x[i], dirs_z[i], freqs[i], phases[i], sharpnesses[i], temp_y);
		
		//y = y + temp_y;
	//}

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
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex.x, y, vertex.z, 1.0);
}



