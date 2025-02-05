#include "assignment4.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>

edaf80::Assignment4::Assignment4(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 4", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment4::~Assignment4()
{
	bonobo::deinit();
}

void
edaf80::Assignment4::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(50.0f, 10.0f, 50.0f));
	mCamera.mWorld.LookAt(glm::vec3(0.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "common/fallback.vert" },
	                                           { ShaderType::fragment, "common/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);
	if (skybox_shader == 0u) {
		LogError("Failed to load skybox shader");
		return;
	}

	GLuint waveSphere_shader = 0u;
	program_manager.CreateAndRegisterProgram("waveSphere",
		{ { ShaderType::vertex, "EDAF80/waveSphere.vert" },
		  { ShaderType::fragment, "EDAF80/waveSphere.frag" } },
		waveSphere_shader);
	if (waveSphere_shader == 0u) {
		LogError("Failed to load waveSphere shader");
		return;
	}

	GLuint wave_shader = 0u;
	program_manager.CreateAndRegisterProgram("Wave",
		{ { ShaderType::vertex, "EDAF80/wave.vert" },
		  { ShaderType::fragment, "EDAF80/wave.frag" } },
		wave_shader);

	if (wave_shader == 0u) {
		LogError("Failed to load wave shader");
		return;
	}

	float amps[] = { 1.0, 0.5 };
	float waveAmps[] = { 0.1, 0.05 };
	float dirs_x[] = { -1.0, 0.7 };
	float dirs_z[] = { 0, 0.7 };
	float wavedirs_x[] = { -1.0,  0};
	float wavedirs_z[] = { 0, -1.0 };
	float freqs[] = { 0.2, 0.4 };
	float wavefreqs[] = { 200.0, 400.0 };
	float phases[] = { 0.5, 1.3 };
	float sharpnesses[] = { 2.0, 2.0 };
	float elapsed_time_s = 0.0f;

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	auto const wave_set_uniforms = [&amps, &dirs_x, &dirs_z, &freqs, &phases, &sharpnesses, &elapsed_time_s, &camera_position](GLuint program) {
		glUniform1fv(glGetUniformLocation(program, "amps"), 2, amps);
		glUniform1fv(glGetUniformLocation(program, "dirs_x"), 2, dirs_x);
		glUniform1fv(glGetUniformLocation(program, "dirs_z"), 2, dirs_z);
		glUniform1fv(glGetUniformLocation(program, "freqs"), 2, freqs);
		glUniform1fv(glGetUniformLocation(program, "phases"), 2, phases);
		glUniform1fv(glGetUniformLocation(program, "sharpness"), 2, sharpnesses);
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};

	auto const waveSphere_set_uniforms = [&waveAmps, &wavedirs_x, &wavedirs_z, &wavefreqs, &phases, &sharpnesses, &elapsed_time_s, &camera_position](GLuint program) {
		glUniform1fv(glGetUniformLocation(program, "amps"), 2, waveAmps);
		glUniform1fv(glGetUniformLocation(program, "dirs_x"), 2, wavedirs_x);
		glUniform1fv(glGetUniformLocation(program, "dirs_z"), 2, wavedirs_z);
		glUniform1fv(glGetUniformLocation(program, "freqs"), 2, wavefreqs);
		glUniform1fv(glGetUniformLocation(program, "phases"), 2, phases);
		glUniform1fv(glGetUniformLocation(program, "sharpness"), 2, sharpnesses);
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};
	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	auto skybox_shape = parametric_shapes::createSphere(50.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	Node skybox;
	GLuint cubemap = bonobo::loadTextureCubeMap(config::resources_path("cubemaps/NissiBeach2/posx.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negx.jpg"),
		config::resources_path("cubemaps/NissiBeach2/posy.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negy.jpg"),
		config::resources_path("cubemaps/NissiBeach2/posz.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negz.jpg"));
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&skybox_shader, set_uniforms);
	skybox.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);
	

	

	/*
	auto water_shape = parametric_shapes::createQuad(100u, 100u, 1000u, 1000u);
	if (water_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}
	Node water_node;
	GLuint normalmap = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));
	water_node.set_geometry(water_shape);
	water_node.set_program(&wave_shader, wave_set_uniforms);
	water_node.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);
	water_node.add_texture("normalmap", normalmap, GL_TEXTURE_2D);
	*/
	auto sphere_shape = parametric_shapes::createSphere(5.0f, 100u, 100u);
	if (sphere_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}
	
	Node sphere_node;
	GLuint normalmap = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));
	sphere_node.set_geometry(sphere_shape);
	sphere_node.set_program(&waveSphere_shader, waveSphere_set_uniforms);
	sphere_node.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);
	sphere_node.add_texture("normalmap", normalmap, GL_TEXTURE_2D);

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	//water_node.get_transform().SetTranslate(glm::vec3(0.0, 0.0, 0.0));
	skybox.get_transform().SetTranslate(glm::vec3(50.0, 5.0, 50.0));
	sphere_node.get_transform().SetTranslate(glm::vec3(50.0, 5.0, 50.0));

	auto lastTime = std::chrono::high_resolution_clock::now();

	bool pause_animation = true;
	bool use_orbit_camera = false;
	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	changeCullMode(cull_mode);

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!pause_animation) {
			elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
		}

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		if (use_orbit_camera) {
			mCamera.mWorld.LookAt(glm::vec3(0.0f));
		}
		camera_position = mCamera.mWorld.GetTranslation();

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		//
		// Todo: If you need to handle inputs, you can do it here
		//


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		if (!shader_reload_failed) {
			//water_node.render(mCamera.GetWorldToClipMatrix());
			sphere_node.render(mCamera.GetWorldToClipMatrix());
			//skybox.render(mCamera.GetWorldToClipMatrix());
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//


		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Pause animation", &pause_animation);
			ImGui::Checkbox("Use orbit camera", &use_orbit_camera);
			ImGui::Separator();
			auto const cull_mode_changed = bonobo::uiSelectCullMode("Cull mode", cull_mode);
			if (cull_mode_changed) {
				changeCullMode(cull_mode);
			}
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			ImGui::Separator();
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
		}
		ImGui::End();

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment4 assignment4(framework.GetWindowManager());
		assignment4.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
