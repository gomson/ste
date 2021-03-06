//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_render_pass_attachment.hpp>
#include <vk_render_pass_subpass_descriptor.hpp>
#include <vk_render_pass_subpass_dependency.hpp>

#include <optional.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_render_pass : public allow_type_decay<vk_render_pass, VkRenderPass> {
private:
	optional<VkRenderPass> render_pass;
	alias<const vk_logical_device> device;

public:
	vk_render_pass(const vk_logical_device &device,
				   const lib::vector<vk_render_pass_attachment> &attachments,
				   const lib::vector<vk_render_pass_subpass_descriptor> &subpasses,
				   const lib::vector<vk_render_pass_subpass_dependency> &subpass_dependencies = {}) : device(device) {
		lib::vector<VkAttachmentDescription> attachment_descriptors;
		attachment_descriptors.resize(attachments.size());
		for (std::size_t i = 0; i < attachments.size(); ++i)
			attachment_descriptors[i] = *(attachments.begin() + i);

		lib::vector<VkSubpassDependency> dependency_descriptors;
		dependency_descriptors.resize(subpass_dependencies.size());
		for (std::size_t i = 0; i < subpass_dependencies.size(); ++i)
			dependency_descriptors[i] = *(subpass_dependencies.begin() + i);

		lib::vector<VkSubpassDescription> subpass_descriptors;
		subpass_descriptors.resize(subpasses.size());
		for (std::size_t i = 0; i < subpasses.size(); ++i) {
			const auto &s = *(subpasses.begin() + i);

			VkSubpassDescription subpass = {};
			subpass.flags = 0;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = static_cast<std::uint32_t>(s.color.size());
			subpass.pColorAttachments = s.color.data();
			subpass.inputAttachmentCount = static_cast<std::uint32_t>(s.input.size());
			subpass.pInputAttachments = s.input.data();
			subpass.preserveAttachmentCount = static_cast<std::uint32_t>(s.preserve.size());
			subpass.pPreserveAttachments = s.preserve.data();
			subpass.pDepthStencilAttachment = s.depth ? &s.depth.get() : nullptr;
			subpass.pResolveAttachments = nullptr;

			subpass_descriptors[i] = subpass;
		}

		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.attachmentCount = static_cast<std::uint32_t>(attachment_descriptors.size());
		create_info.pAttachments = attachment_descriptors.data();
		create_info.subpassCount = static_cast<std::uint32_t>(subpass_descriptors.size());
		create_info.pSubpasses = subpass_descriptors.data();
		create_info.dependencyCount = static_cast<std::uint32_t>(dependency_descriptors.size());
		create_info.pDependencies = dependency_descriptors.data();

		VkRenderPass renderpass;
		vk_result res = vkCreateRenderPass(device, &create_info, nullptr, &renderpass);
		if (!res) {
			throw vk_exception(res);
		}

		this->render_pass = renderpass;
	}
	~vk_render_pass() noexcept { destroy_render_pass(); }

	vk_render_pass(vk_render_pass &&) = default;
	vk_render_pass &operator=(vk_render_pass &&) = default;
	vk_render_pass(const vk_render_pass &) = delete;
	vk_render_pass &operator=(const vk_render_pass &) = delete;

	void destroy_render_pass() {
		if (render_pass) {
			vkDestroyRenderPass(device.get(), *this, nullptr);
			render_pass = none;
		}
	}

	auto& get() const { return render_pass.get(); }
};

}

}
}
