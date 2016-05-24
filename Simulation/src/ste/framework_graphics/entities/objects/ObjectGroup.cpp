
#include "stdafx.hpp"
#include "ObjectGroup.hpp"

#include "mesh_descriptor.hpp"

#include "glsl_programs_pool.hpp"
#include "gl_current_context.hpp"

#include <algorithm>

using namespace StE::Graphics;

ObjectGroup::~ObjectGroup() {
	remove_all();
}

void ObjectGroup::add_object(const std::shared_ptr<Object> &obj) {
	auto connection = std::make_shared<signal_connection_type>(
		[this](Object* obj) {
			this->signalled_objects.push_back(obj);
		}
	);
	obj->signal_model_change().connect(connection);

	auto &ind = obj->get_mesh().get_indices();
	auto &vertices = obj->get_mesh().get_vertices();

	objects.insert(std::make_pair(obj,
								  object_information{ objects.size(), connection }));

	draw_buffers.get_vbo_stack().push_back(vertices);
	draw_buffers.get_indices_stack().push_back(ind);

	mesh_descriptor md;
	md.model_transform_matrix = glm::transpose(obj->get_model_transform());
	md.tangent_transform_quat = obj->get_orientation();
	md.mat_idx = mat_storage->index_of(obj->get_material());
	md.bounding_sphere = obj->get_mesh().bounding_sphere().sphere();

	mesh_draw_params mdp;
 	mdp.count = ind.size();
 	mdp.first_index = total_indices;
 	mdp.base_vertex = total_vertices;

	obj->md = md;

	draw_buffers.get_mesh_data_stack().push_back(std::move(md));
	draw_buffers.get_mesh_draw_params_stack().push_back(std::move(mdp));

 	total_vertices += vertices.size();
 	total_indices += ind.size();
}

void ObjectGroup::remove_all() {
	for (auto &o : objects)
		o.first->signal_model_change().disconnect(o.second.connection);
	objects.clear();
	signalled_objects.clear();
}

void ObjectGroup::update_dirty_buffers() const {
	for (auto obj_ptr : signalled_objects) {
		auto it = std::find_if(objects.begin(), objects.end(), [&](const objects_map_type::value_type &v) -> bool {
			return v.first.get() == obj_ptr;
		});
		if (it == objects.end()) {
			assert(false);
			continue;
		}
		object_information info = it->second;

		mesh_descriptor md = obj_ptr->md;
		md.model_transform_matrix = glm::transpose(obj_ptr->get_model_transform());
		md.tangent_transform_quat = obj_ptr->get_orientation();
		md.mat_idx = mat_storage->index_of(obj_ptr->get_material());
		draw_buffers.get_mesh_data_stack().overwrite(info.index, md);
	}

	signalled_objects.clear();
}
