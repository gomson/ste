
#include <stdafx.hpp>
// TODO
//#include <directional_shadowmap_projector.hpp>
//
//using namespace ste::graphics;
//
//void directional_shadowmap_projector::set_context_state() const {
//	using namespace Core;
//
//	gl::gl_current_context::get()->enable_depth_test();
//	gl::gl_current_context::get()->color_mask(false, false, false, false);
//	gl::gl_current_context::get()->enable_state(gl::BasicStateName::CULL_FACE);
//	//gl::gl_current_context::get()->cull_face(GL_FRONT);
//	
//	auto size = shadow_map->get_directional_maps()->get_size();
//	gl::gl_current_context::get()->viewport(0, 0, size.x, size.y);
//
//	s->get_directional_shadow_projection_buffers().idb.buffer().bind();
//	s->bind_buffers();
//	lights->bind_lights_buffer(2);
//
//	0_uniform_idx = lights->get_directional_lights_cascades_buffer();
//	8_storage_idx = s->get_directional_shadow_projection_buffers().proj_id_to_light_id_translation_table;
//
//	shadow_gen_program.get().bind();
//	shadow_map->get_directional_maps_fbo()->bind();
//}
//
//void directional_shadowmap_projector::dispatch() const {
//	Core::gl::gl_current_context::get()->clear_framebuffer(false, true);
//
//	Core::gl::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
//	s->draw_object_group();
//}
