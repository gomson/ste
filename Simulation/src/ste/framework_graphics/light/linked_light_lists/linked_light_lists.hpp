// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <resource_old.hpp>

#include <texture_2d.hpp>

#include <buffer_usage.hpp>
#include <shader_storage_buffer.hpp>
#include <atomic_counter_buffer_object.hpp>

#include <gl_current_context.hpp>

#include <lib/unique_ptr.hpp>
#include <limits>

namespace ste {
namespace graphics {

class linked_light_lists {
private:
	struct lll_element {
		glm::uvec2 data;
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t virt_size = 2147483648 / 2;

	using lll_type = Core::shader_storage_buffer<lll_element, usage>;

public:
	static constexpr int lll_image_res_multiplier = 8;

private:
	lll_type lll;
	Core::shader_storage_buffer<std::uint32_t> lll_counter;
	lib::unique_ptr<Core::texture_2d> lll_heads;
	lib::unique_ptr<Core::texture_2d> lll_low_detail_heads;
	lib::unique_ptr<Core::texture_2d> lll_size;
	lib::unique_ptr<Core::texture_2d> lll_low_detail_size;

	glm::ivec2 size;

	static std::size_t virtual_lll_size() {
		return (virt_size / lll_type::page_size() / sizeof(lll_element) + 1) * lll_type::page_size();
	}

public:
	linked_light_lists(glm::ivec2 size) : lll(virtual_lll_size()), lll_counter(1) { resize(size); }

	void resize(glm::ivec2 size);
	auto& get_size() const { return size; }

	void clear() {
		std::uint32_t zero = 0;
		lll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	}

	void bind_readwrite_lll_buffers() const;
	void bind_lll_buffer(bool low_detail = false) const;
};

}
}
