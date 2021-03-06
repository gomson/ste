// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <signal.hpp>

#include <resource_instance.hpp>
#include <resource_loading_task.hpp>

#include <gpu_dispatchable.hpp>

#include <glsl_program.hpp>
#include <texture_2d.hpp>
#include <texture_2d_array.hpp>
#include <texture_3d.hpp>

#include <lib/unique_ptr.hpp>


//! TODO: Remove hack
#include <volumetric_scattering_scatter_dispatch.hpp>


namespace ste {
namespace graphics {

class gi_renderer;

class deferred_composer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class resource::resource_loading_task<deferred_composer>;
	friend class resource::resource_instance<deferred_composer>;

private:
	resource::resource_instance<resource::glsl_program> program;
	gi_renderer *dr;

	lib::shared_ptr<connection<>> vss_storage_connection;
	lib::shared_ptr<connection<>> shadows_storage_connection;

	lib::unique_ptr<Core::texture_2d> microfacet_refraction_fit_lut;
	lib::unique_ptr<Core::texture_2d_array> microfacet_transmission_fit_lut;

	lib::unique_ptr<Core::texture_2d_array> atmospherics_optical_length_lut;
	lib::unique_ptr<Core::texture_3d> atmospherics_scatter_lut;
	lib::unique_ptr<Core::texture_3d> atmospherics_mie0_scatter_lut;
	lib::unique_ptr<Core::texture_3d> atmospherics_ambient_lut;

	lib::unique_ptr<Core::texture_2d> ltc_ggx_fit, ltc_ggx_amplitude;

	resource::resource_instance<volumetric_scattering_scatter_dispatch> *additional_scatter_program_hack;

private:
	void load_microfacet_fit_luts();
	void load_atmospherics_luts();
	void attach_handles() const;

private:
	deferred_composer(const ste_engine_control &ctx, gi_renderer *dr, resource::resource_instance<volumetric_scattering_scatter_dispatch> *additional_scatter_program_hack);

public:
	~deferred_composer() noexcept {}

	auto& get_program() { return program.get(); }

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace resource {

template <>
class resource_loading_task<graphics::deferred_composer> {
	using R = graphics::deferred_composer;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
			object->additional_scatter_program_hack->wait();
			// TODO: Fix
		}).then/*_on_main_thread*/([object]() {
			object->attach_handles();
			object->load_microfacet_fit_luts();
			object->load_atmospherics_luts();
		});
	}
};

}
}
