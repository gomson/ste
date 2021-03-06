
#include <stdafx.hpp>
// TODO
//#include <deferred_composer.hpp>
//#include <gi_renderer.hpp>
//
//#include <microfacet_refraction_fit.hpp>
//#include <microfacet_transmission_fit.hpp>
//
//#include <atmospherics_precompute_scattering.hpp>
//
//#include <Quad.hpp>
//
//#include <sampler.hpp>
//
//#include <gl_current_context.hpp>
//
//#include <Log.hpp>
//#include <attributed_string.hpp>
//#include <attrib.hpp>
//
//#include <fstream>
//
//#include <surface_factory.hpp>
//
//using namespace ste::graphics;
//
//namespace ste {
//namespace graphics {
//namespace deferred_composer_detail {
//
//template <typename T>
//struct load_lut {
//	auto operator()(const char *name) const {
//		using namespace text::attributes;
//
//		std::ifstream ifs(name, std::ios::binary);
//		if (!ifs.good()) {
//			ste_log_error() << text::attributed_string("Can't open \"") + i(name) + "\": " + std::strerror(errno) << std::endl;
//			throw std::runtime_error(lib::string(name) + "not found");
//		}
//
//		auto fit_data = T(ifs).create_lut();
//		ifs.close();
//
//		ste_log() << text::attributed_string("Loaded \"") + i(name) + "\" successfully." << std::endl;
//
//		return fit_data;
//	};
//};
//
//}
//}
//}
//
//deferred_composer::deferred_composer(const ste_engine_control &ctx, gi_renderer *dr, resource::resource_instance<volumetric_scattering_scatter_dispatch> *additional_scatter_program_hack) : program(ctx, lib::vector<lib::string>{ "passthrough.vert", "deferred_compose.frag" }), dr(dr), additional_scatter_program_hack(additional_scatter_program_hack) {
//	vss_storage_connection = lib::allocate_shared<connection<>>([&]() {
//		attach_handles();
//	});
//	shadows_storage_connection = lib::allocate_shared<connection<>>([&]() {
//		attach_handles();
//	});
//	dr->vol_scat_storage.get_storage_modified_signal().connect(vss_storage_connection);
//	dr->shadows_storage.get_storage_modified_signal().connect(shadows_storage_connection);
//}
//
//void deferred_composer::load_microfacet_fit_luts() {
//	static const char *refraction_fit_name = R"(Data/microfacet_ggx_refraction_fit.bin)";
//	static const char *transmission_fit_name = R"(Data/microfacet_ggx_transmission_fit.bin)";
//
//	try {
//		microfacet_refraction_fit_lut = lib::allocate_unique<Core::texture_2d>(deferred_composer_detail::load_lut<microfacet_refraction_fit>()(refraction_fit_name));
//		microfacet_transmission_fit_lut = lib::allocate_unique<Core::texture_2d_array>(deferred_composer_detail::load_lut<microfacet_transmission_fit_v4>()(transmission_fit_name));
//	} catch (const std::exception &err) {
//		using namespace text::attributes;
//		ste_log_error() << text::attributed_string("Can't open Microfacet LUT. Error: \"") + b(err.what()) + "\"." << std::endl;
//
//		throw;
//	}
//
//	auto refraction_handle = microfacet_refraction_fit_lut->get_texture_handle(*Core::sampler::sampler_nearest_clamp());
//	auto transmission_handle = microfacet_transmission_fit_lut->get_texture_handle(*Core::sampler::sampler_nearest_clamp());
//	refraction_handle.make_resident();
//	transmission_handle.make_resident();
//	program.get().set_uniform("microfacet_refraction_fit_lut", refraction_handle);
//	program.get().set_uniform("microfacet_transmission_fit_lut", transmission_handle);
//
//
//	//? TODO: RELOCATE
//	auto ltc_ggx_tab = resource::surface_factory::load_surface_2d(R"(Data/ltc_ggx_fit.dds)", false);
//	auto ltc_ggx_amp = resource::surface_factory::load_surface_2d(R"(Data/ltc_ggx_amplitude.dds)", false);
//	ltc_ggx_fit = lib::allocate_unique<Core::texture_2d>(ltc_ggx_tab);
//	ltc_ggx_amplitude = lib::allocate_unique<Core::texture_2d>(ltc_ggx_amp);
//
//	auto ltc_ggx_fit_handle = ltc_ggx_fit->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//	auto ltc_ggx_amplitude_handle = ltc_ggx_amplitude->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//	ltc_ggx_fit_handle.make_resident();
//	ltc_ggx_amplitude_handle.make_resident();
//	program.get().set_uniform("ltc_ggx_fit", ltc_ggx_fit_handle);
//	program.get().set_uniform("ltc_ggx_amplitude", ltc_ggx_amplitude_handle);
//}
//
//void deferred_composer::load_atmospherics_luts() {
//	static const char *lut_name = R"(Data/atmospherics_lut.bin)";
//
//	try {
//		atmospherics_precompute_scattering lut_loader(lut_name);
//		atmospherics_optical_length_lut = lib::allocate_unique<Core::texture_2d_array>(lut_loader.create_optical_length_lut());
//		atmospherics_scatter_lut = lib::allocate_unique<Core::texture_3d>(lut_loader.create_scatter_lut());
//		atmospherics_mie0_scatter_lut = lib::allocate_unique<Core::texture_3d>(lut_loader.create_mie0_scatter_lut());
//		atmospherics_ambient_lut = lib::allocate_unique<Core::texture_3d>(lut_loader.create_ambient_lut());
//	}
//	catch (const std::exception &err) {
//		using namespace text::attributes;
//		ste_log_error() << text::attributed_string("Can't open Atmospherics Scatter LUT. Error: \"") + b(err.what()) + "\"." << std::endl;
//
//		throw;
//	}
//
//	ste_log() << text::attributed_string("Loaded \"") + text::attributes::i(lut_name) + "\" successfully." << std::endl;
//
//	auto optical_length_handle = atmospherics_optical_length_lut->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//	auto scatter_handle = atmospherics_scatter_lut->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//	auto mie0_scatter_handle = atmospherics_mie0_scatter_lut->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//	auto ambient_handle = atmospherics_ambient_lut->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//	optical_length_handle.make_resident();
//	scatter_handle.make_resident();
//	mie0_scatter_handle.make_resident();
//	ambient_handle.make_resident();
//	program.get().set_uniform("atmospheric_optical_length_lut", optical_length_handle);
//	program.get().set_uniform("atmospheric_scattering_lut", scatter_handle);
//	program.get().set_uniform("atmospheric_mie0_scattering_lut", mie0_scatter_handle);
//	program.get().set_uniform("atmospheric_ambient_lut", ambient_handle);
//
//	//! Hack
//	additional_scatter_program_hack->get().get_program()->set_uniform("atmospheric_optical_length_lut", optical_length_handle);
//	additional_scatter_program_hack->get().get_program()->set_uniform("atmospheric_scattering_lut", scatter_handle);
//	additional_scatter_program_hack->get().get_program()->set_uniform("atmospheric_mie0_scattering_lut", mie0_scatter_handle);
//	additional_scatter_program_hack->get().get_program()->set_uniform("atmospheric_ambient_lut", ambient_handle);
//	//! /Hack
//}
//
//void deferred_composer::attach_handles() const {
//	auto scattering_volume = dr->vol_scat_storage.get_volume_texture();
//	if (scattering_volume) {
//		auto scattering_volume_handle = scattering_volume->get_texture_handle(dr->vol_scat_storage.get_volume_sampler());
//		scattering_volume_handle.make_resident();
//		program.get().set_uniform("scattering_volume", scattering_volume_handle);
//	}
//
//	auto shadow_depth_maps = dr->shadows_storage.get_cubemaps();
//	if (shadow_depth_maps) {
//		auto shadow_depth_maps_handle = shadow_depth_maps->get_texture_handle(dr->shadows_storage.get_shadow_sampler());
//		shadow_depth_maps_handle.make_resident();
//		program.get().set_uniform("shadow_depth_maps", shadow_depth_maps_handle);
//
//		auto shadow_maps_handle = shadow_depth_maps->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//		shadow_maps_handle.make_resident();
//		program.get().set_uniform("shadow_maps", shadow_maps_handle);
//	}
//
//	auto directional_shadow_depth_maps = dr->shadows_storage.get_directional_maps();
//	if (directional_shadow_depth_maps) {
//		auto directional_shadow_depth_maps_handle = directional_shadow_depth_maps->get_texture_handle(dr->shadows_storage.get_shadow_sampler());
//		directional_shadow_depth_maps_handle.make_resident();
//		program.get().set_uniform("directional_shadow_depth_maps", directional_shadow_depth_maps_handle);
//
//		auto directional_shadow_maps_handle = directional_shadow_depth_maps->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//		directional_shadow_maps_handle.make_resident();
//		program.get().set_uniform("directional_shadow_maps", directional_shadow_maps_handle);
//	}
//
//	program.get().set_uniform("cascades_depths", dr->s->properties().lights_storage().get_cascade_depths_array());
//}
//
//void deferred_composer::set_context_state() const {
//	using namespace Core;
//
//	auto &ls = dr->s->properties().lights_storage();
//
//	gl::gl_current_context::get()->enable_state(ste::Core::gl::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);
//	
//	0_tex_unit = *dr->gbuffer.get_backface_depth_target();
//	1_tex_unit = *dr->gbuffer.get_depth_target();
//	2_tex_unit = *dr->gbuffer.get_gbuffer();
//
//	0_storage_idx = dr->s->properties().materials_storage().buffer();
//	1_storage_idx = dr->s->properties().material_layers_storage().buffer();
//
//	ls.bind_lights_buffer(2);
//
//	0_uniform_idx = dr->s->properties().lights_storage().get_directional_lights_cascades_buffer();
//	8_storage_idx = dr->s->properties().lights_storage().get_shaped_lights_points_buffer();
//	dr->lll_storage.bind_lll_buffer();
//
//	// TODO: Fix
////	screen_filling_quad.vao()->bind();
//
//	program.get().bind();
//}
//
//void deferred_composer::dispatch() const {
//	Core::gl::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
//	Core::gl::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
//}
