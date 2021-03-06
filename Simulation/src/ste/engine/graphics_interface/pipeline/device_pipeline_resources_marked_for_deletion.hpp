#//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_pipeline_layout.hpp>
#include <vk_pipeline.hpp>
#include <pipeline_binding_set_layout.hpp>
#include <pipeline_binding_set.hpp>

#include <lib/vector.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

struct device_pipeline_resources_marked_for_deletion {
	lib::vector<pipeline_binding_set_layout> binding_set_layouts;
	lib::vector<pipeline_binding_set> binding_sets;

	lib::unique_ptr<vk::vk_pipeline_layout> pipeline_layout;
	optional<vk::vk_pipeline> pipeline;

	operator bool() const {
		return binding_set_layouts.size() ||
			binding_sets.size() ||
			pipeline_layout != nullptr ||
			pipeline;
	}
};

}
}
