// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <gpu_dispatchable.hpp>

#include <ste_engine_control.hpp>
#include <gl_current_context.hpp>

#include <sampler.hpp>

#include <glsl_program.hpp>
#include <deferred_gbuffer.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace graphics {

class gbuffer_downsample_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class resource::resource_loading_task<gbuffer_downsample_depth_dispatch>;
	friend class resource::resource_instance<gbuffer_downsample_depth_dispatch>;

private:
	const deferred_gbuffer *gbuffer;
	resource::resource_instance<resource::glsl_program> program;

	lib::shared_ptr<connection<>> gbuffer_depth_target_connection;

private:
	void attach_handles() const {
		auto depth_target = gbuffer->get_depth_target();
		if (depth_target) {
			auto depth_target_handle = depth_target->get_texture_handle(*Core::sampler::sampler_nearest_clamp());
			depth_target_handle.make_resident();
			program.get().set_uniform("depth_target", depth_target_handle);
		}

		auto downsampled_depth_target = gbuffer->get_downsampled_depth_target();
		if (downsampled_depth_target) {
			auto downsampled_depth_target_handle = downsampled_depth_target->get_texture_handle(*Core::sampler_mipmapped::mipmapped_sampler_nearest_clamp());
			downsampled_depth_target_handle.make_resident();
			program.get().set_uniform("downsampled_depth_target", downsampled_depth_target_handle);
		}
	}

public:
	gbuffer_downsample_depth_dispatch(const ste_engine_control &ctx,
									  const deferred_gbuffer *gbuffer) : gbuffer(gbuffer),
									  									 program(ctx, "gbuffer_downsample_depth.comp") {
		gbuffer_depth_target_connection = lib::allocate_shared<connection<>>([&]() {
			attach_handles();
		});
		gbuffer->get_depth_target_modified_signal().connect(gbuffer_depth_target_connection);
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace resource {

template <>
class resource_loading_task<graphics::gbuffer_downsample_depth_dispatch> {
	using R = graphics::gbuffer_downsample_depth_dispatch;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
			// TODO: Fix
		}).then/*_on_main_thread*/([object]() {
			object->attach_handles();
		});
	}
};

}
}
