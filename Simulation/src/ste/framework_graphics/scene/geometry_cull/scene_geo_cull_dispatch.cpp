// TODO
#include <stdafx.hpp>
//#include <scene_geo_cull_dispatch.hpp>
//
//#include <Scene.hpp>
//#include <extract_projection_planes.hpp>
//
//using namespace ste::graphics;
//using namespace ste::Core;
//
//void scene_geo_cull_dispatch::commit_idbs() const {
//	auto size = s->get_object_group().get_draw_buffers().size();
//	if (size != old_object_group_size) {
//		old_object_group_size = size;
//		s->get_idb().buffer().commit_range(0, size);
//		s->get_shadow_projection_buffers().commit_range(0, size);
//		s->get_directional_shadow_projection_buffers().commit_range(0, size);
//	}
//}
//
//void scene_geo_cull_dispatch::set_context_state() const {
//	auto& draw_buffers = s->get_object_group().get_draw_buffers();
//
//	0_atomic_idx = s->get_culled_objects_counter();
//
//	0_storage_idx = s->get_shadow_projection_buffers().idb.ssbo();
//	1_storage_idx = s->get_directional_shadow_projection_buffers().idb.ssbo();
//
//	8_storage_idx = s->get_shadow_projection_buffers().proj_id_to_light_id_translation_table;
//	9_storage_idx = s->get_directional_shadow_projection_buffers().proj_id_to_light_id_translation_table;
//
//	10_storage_idx = s->get_idb().ssbo();
//
//	draw_buffers.get_mesh_data_buffer().bind_range(Core::shader_storage_layout_binding(14), 0, draw_buffers.size());
//	draw_buffers.get_mesh_draw_params_buffer().bind_range(Core::shader_storage_layout_binding(15), 0, draw_buffers.size());
//
//	ls->bind_lights_buffer(2);
//	4_storage_idx = Core::buffer_object_cast<Core::shader_storage_buffer<std::uint32_t>>(ls->get_active_ll_counter());
//	5_storage_idx = ls->get_active_ll();
//	0_uniform_idx = ls->get_directional_lights_cascades_buffer();
//
//	program.get().bind();
//}
//
//void scene_geo_cull_dispatch::dispatch() const {
//	commit_idbs();
//
//	auto& draw_buffers = s->get_object_group().get_draw_buffers();
//
//	constexpr int jobs = 128;
//	auto size = (draw_buffers.size() + jobs - 1) / jobs;
//
//	s->clear_indirect_command_buffers();
//	gl::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
//	Core::gl::gl_current_context::get()->dispatch_compute(size, 1, 1);
//}
