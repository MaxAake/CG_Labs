#pragma once

#include "core/helpers.hpp"
#include "core/node.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Asteroid
{
public:
	//! \brief Default constructor for a celestial body.
	//!
	//! @param [in] shape Information about the geometry used to model the
	//!             celestial body (more details about it in assignment 2
	//! @param [in] program Shader program used to render the celestial
	//!             body (more details about it in assignment~3)
	//! @param [in] diffuse_texture_id Identifier of the diffuse texture
	//!             used (more details about it also in assignment~3)
	//!

	static const float bounding_radius;
	static const float maximum_speed;

	Asteroid();

	//! \brief Render this celestial body.
	//!
	//! @param [in] elapsed_time Amount of time (in microseconds) between
	//!             two frames
	//! @param [in] view_projection Matrix transforming from world space to
	//!             clip space
	//! @param [in] parent_transform Matrix transforming from the parent’s
	//!             local space to world space
	//! @param [in] show_basis Show a 3D basis transformed by the world matrix
	//!             of this celestial body
	//! @return Matrix transforming from this celestial body’s local space
	//!         to world space
	void render(std::chrono::microseconds elapsed_time,
		glm::mat4 const& view_projection,
		bool show_basis = false);
	struct {
		Node node;
		glm::vec3 position;
		glm::vec3 velocity;
		glm::vec3 axis;
		float angle;
		float spin_speed;
	} _body;

};
