
#include <stdafx.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

using namespace ste::gl;

ste::lib::string vk::vk_query::read_results(std::size_t data_size, VkQueryResultFlags flags) const {
	return pool.read_results(data_size, index, 1, data_size, flags);
}
