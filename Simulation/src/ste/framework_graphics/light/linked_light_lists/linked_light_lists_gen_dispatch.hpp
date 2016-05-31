// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "glsl_program.hpp"

#include "linked_light_lists.hpp"
#include "light_storage.hpp"

#include "glsl_program.hpp"
#include "Texture2D.hpp"
#include "Sampler.hpp"

namespace StE {
namespace Graphics {

class linked_light_lists_gen_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	light_storage *ls;
	linked_light_lists *lll;
	Resource::resource_instance<Resource::glsl_program> program;

	Core::SamplerMipmapped depth_sampler;

	Core::Texture2D *depth_map;

public:
	linked_light_lists_gen_dispatch(const StEngineControl &ctx,
									light_storage *ls,
									linked_light_lists *lll) : ls(ls), lll(lll),
															   program(ctx, std::vector<std::string>{ "passthrough.vert", "linked_light_lists_gen.frag" }),
															   depth_sampler(Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest,
															   				 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {}

	void set_depth_map(Core::Texture2D *dm) { depth_map = dm; }

protected:
	virtual void set_context_state() const override;
	virtual void dispatch() const override;
};

}
}
