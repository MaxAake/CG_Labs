#include "Asteroid.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/helpers.hpp"
#include "core/Log.h"

const float Asteroid::bounding_radius = 10.0f;
const float Asteroid::maximum_speed = 2.0f;


Asteroid::Asteroid()
{
	_body.position = glm::vec3(rand() / (RAND_MAX + 1.0f) - 0.5f, rand() / (RAND_MAX + 1.0f) - 0.5f, rand() / (RAND_MAX + 1.0f) - 0.5f) * bounding_radius * sqrt(2.42f) * 2.0f;
	_body.velocity = glm::vec3(rand() / (RAND_MAX + 1.0f) - 0.5f, rand() / (RAND_MAX + 1.0f) - 0.5f, rand() / (RAND_MAX + 1.0f) - 0.5f) * maximum_speed;
	_body.axis = glm::normalize(glm::vec3(rand() / (RAND_MAX + 1.0f), rand() / (RAND_MAX + 1.0f), rand() / (RAND_MAX + 1.0f)) * glm::two_pi<float>());
	_body.angle = 0.0f;
	_body.spin_speed = rand() / (RAND_MAX + 1.0f);

	
}


void Asteroid::render(std::chrono::microseconds elapsed_time,
	glm::mat4 const& view_projection,
	bool show_basis)
{
	// Convert the duration from microseconds to seconds.
	auto const elapsed_time_s = std::chrono::duration<float>(elapsed_time).count();
	_body.position += _body.velocity * elapsed_time_s;
	_body.angle += elapsed_time_s * _body.spin_speed;
	//asteroids[i].get_transform().SetRotate(0.0f, asteroid_rotations[i]); this do not work rn
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), _body.position);
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), _body.angle, _body.axis);

	glm::mat4 world = translate * rot;
	//std::cout << _body.position[0];
	//std::cout << _body.position[1];
	//std::cout << _body.position[2];
	// Note: The second argument of `node::render()` is supposed to be the
	// parent transform of the node, not the whole world matrix, as the
	// node internally manages its local transforms. However in our case we
	// manage all the local transforms ourselves, so the internal transform
	// of the node is just the identity matrix and we can forward the whole
	// world matrix.
	_body.node.render(view_projection, world);

}
