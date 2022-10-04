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

// This is the custom output of this shader. If you want to retrieve this data
// from another shader further down the pipeline, you need to declare the exact
// same structure as in (for input), with matching name for the structure
// members and matching structure type. Have a look at
// shaders/EDAF80/diffuse.frag.
out VS_OUT {
	vec3 vertex;
} vs_out;


void waveFunction(in float amp, in float dir_x, in float dir_z, in float freq, in float phase, in float sharpness, inout float y)
{
	y += amp * pow(sin(((dir_x * vertex.x) + (dir_z * vertex.z)) * freq + elapsed_time_s * phase) * 0.5 + 0.5, sharpness);
}

void main()
{
	//for (int i = 0; i < WAVES; i++) {
		//float temp_y = 0.0;
		// waveFunction(amps[i], dirs_x[i], dirs_z[i], freqs[i], phases[i], sharpnesses[i], temp_y);
		
		//y = y + temp_y;
	//}

	float y = 0.0;
	waveFunction(1, -1, 0, 0.2, 0.5, 2, y);
	waveFunction(0.5, -0.7, 0.7, 0.4, 1.3, 2, y);

	float temp_y = 2 * pow(sin(((-1 * vertex.x) + (0 * vertex.z)) * 0.2 + elapsed_time_s * 0.5) * 0.5 + 0.5, 2);
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex.x, y, vertex.z, 1.0));

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex.x, y, vertex.z, 1.0);
}



