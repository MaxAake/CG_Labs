#include "assignment5.hpp"
#include "interpolation.hpp"
#include "parametric_shapes.hpp"
#include "Asteroid.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyfiledialogs.h>

#include <clocale>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment5::~Assignment5()
{
	bonobo::deinit();
}

void
edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h

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

	GLuint flatphong_shader = 0u;
	program_manager.CreateAndRegisterProgram("FlatPhong",
		{ { ShaderType::vertex, "EDAF80/flatphong.vert" },
		  { ShaderType::fragment, "EDAF80/flatphong.frag" } },
		flatphong_shader);
	if (flatphong_shader == 0u) {
		LogError("Failed to load FlatPhong shader");
		return;
	}

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	bool use_normal_mapping = false;
	auto camera_position = mCamera.mWorld.GetTranslation();
	auto const phong_set_uniforms = [&use_normal_mapping, &light_position, &camera_position](GLuint program) {
		glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//

	//
	// Todo: Load your geometry
	//

	auto ship_shape = bonobo::loadObjects(config::resources_path("scenes/Test2.obj"));
	bonobo::material_data ship_material;
	ship_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	ship_material.diffuse = glm::vec3(0.8f, 0.0f, 0.0f);
	ship_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	ship_material.shininess = 2.0f;

	Node ship_node;
	ship_node.set_geometry(ship_shape[0]);
	ship_node.set_material_constants(ship_material);
	ship_node.set_program(&flatphong_shader, phong_set_uniforms);
	


	int const num_asteroids = 100;
	Asteroid asteroids[num_asteroids];
	glm::vec3 asteroid_placements[num_asteroids];
	glm::vec3 asteroid_velocities[num_asteroids];
	glm::vec3 asteroid_rotations[num_asteroids];
	glm::vec3 asteroid_rotational_velocities[num_asteroids];
	glm::vec3 asteroid_angles[num_asteroids];
	auto asteroid_shape = parametric_shapes::createSphere(1.0f, 6u, 6u, true);

	struct {
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f) * 0.003f;
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
	} ship;

	glm::mat4 ship_transform = ship.rotationMatrix * glm::scale(glm::mat4(1.0f), ship.scale);

	bonobo::material_data asteroid_material;
	asteroid_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	asteroid_material.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	asteroid_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	asteroid_material.shininess = 2.0f;
	float const maximum_speed = 2.0f;
	float const bounding_radius = 10.0f;

	for (int i = 0; i < num_asteroids; i++) {
		asteroids[i]._body.node.set_geometry(asteroid_shape);
		asteroids[i]._body.node.set_program(&flatphong_shader, phong_set_uniforms);
		asteroids[i]._body.node.set_material_constants(asteroid_material);
		
	}

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	auto lastTime = std::chrono::high_resolution_clock::now();

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		auto const elapsed_time_s = std::chrono::duration<float>(deltaTimeUs).count();

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		camera_position = mCamera.mWorld.GetTranslation();
		glm::mat4 camera_rotation = mCamera.mWorld.GetRotationMatrix();

		glm::vec2 mousePos = inputHandler.GetMousePosition();
		int width = 0;
		int height = 0;
		glfwGetWindowSize(window, &width, &height); 
		
		glm::vec2 mouseDir = glm::normalize(glm::vec2(mousePos.x - width / 2, mousePos.y - height / 2));
		float mouseAmp = sqrt(pow(mousePos.x - width / 2, 2) + pow(mousePos.y - height / 2, 2));
		mouseAmp = std::max(0.05f*height, std::min(mouseAmp, 0.45f * height));
		mouseAmp = (mouseAmp - 0.05f * height) * 9/8 / 0.45 / height;
		


		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), mouseDir[1] * 0.01f * mouseAmp, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -mouseDir[0] * 0.01f * mouseAmp, glm::vec3(0.0f, 1.0f, 0.0f));
		ship.rotationMatrix *= rotation;
		//glm::vec3 rotationAxis = glm::vec3(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * ship.rotationMatrix);
		//rotationAxis[1] = -rotationAxis[1];
		//ship.position = ship.position + rotationAxis * elapsed_time_s * 1.0f;
		//ship.position.x += mouseDir.x * elapsed_time_s * 10.0f
		//std::cout << rotationAxis[1];
		//std::cout << '\n';

		glm::mat4 newMovement = rotation * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f) * elapsed_time_s * 1000.0f) ;
		ship_transform = ship_transform * newMovement;
		mCamera.mWorld.SetTranslate(glm::vec3(ship_transform[3][0], ship_transform[3][1], ship_transform[3][2]) + normalize(glm::vec3(newMovement[3][0], newMovement[3][1], newMovement[3][2])) * 5.0f);
		//mCamera.mWorld.PreRotateX(-mouseDir[1] * 0.01f * mouseAmp);
		//mCamera.mWorld.RotateY(-mouseDir[0] * 0.01f * mouseAmp);
		


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


		if (!shader_reload_failed) {
			for (int i = 0; i < num_asteroids; i++) {
				asteroids[i].render(deltaTimeUs, mCamera.GetWorldToClipMatrix());
			}
			ship_node.render(mCamera.GetWorldToClipMatrix(), ship_transform);
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//
		/*bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);*/
		// if (opened) {
			// ImGui::Checkbox("Show basis", &show_basis);
			// ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			//ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
		/*}
		ImGui::End();*/

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
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
