// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <gpu_dispatchable.hpp>

#include <ste_engine_control.hpp>
#include <gl_current_context.hpp>

#include <Sampler.hpp>

#include <glsl_program.hpp>
#include <deferred_gbuffer.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace graphics {

class gbuffer_clear_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class resource::resource_loading_task<gbuffer_clear_dispatch>;
	friend class resource::resource_instance<gbuffer_clear_dispatch>;

private:
	mutable deferred_gbuffer *gbuffer;

public:
	gbuffer_clear_dispatch(const ste_engine_control &ctx,
						   deferred_gbuffer *gbuffer) : gbuffer(gbuffer) {}

	void set_context_state() const override final {}
	void dispatch() const override final;
};

}
}
