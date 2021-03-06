//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_image_base.hpp>
#include <image_layout.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_copy_image : public command {
private:
	std::reference_wrapper<const device_image_base> src_image;
	image_layout src_image_layout;
	std::reference_wrapper<const device_image_base> dst_image;
	image_layout dst_image_layout;

	lib::vector<VkImageCopy> ranges;

public:
	cmd_copy_image(const device_image_base &src_image,
				   const image_layout &src_image_layout,
				   const device_image_base &dst_image,
				   const image_layout &dst_image_layout,
				   const lib::vector<VkImageCopy> &ranges = {})
		: src_image(src_image), src_image_layout(src_image_layout),
		dst_image(dst_image), dst_image_layout(dst_image_layout),
		ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			assert(false);
		}
	}
	virtual ~cmd_copy_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyImage(command_buffer, src_image.get().get_image_handle(), static_cast<VkImageLayout>(src_image_layout),
					   dst_image.get().get_image_handle(), 
					   static_cast<VkImageLayout>(dst_image_layout),
					   static_cast<std::uint32_t>(ranges.size()), 
					   ranges.data());
	}
};

}
}
