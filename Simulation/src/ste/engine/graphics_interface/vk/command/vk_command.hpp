//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <vk_command_buffers.hpp>

namespace StE {
namespace GL {

class vk_command {
	friend class vk_command_recorder;

private:
	virtual void operator()(const vk_command_buffer &command_buffer) const = 0;

public:
	virtual ~vk_command() noexcept {}
};

}
}
