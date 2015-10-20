// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "task.h"
#include "Scene.h"

#include <memory>
#include <unordered_map>
#include <string>

#include <gli/gli.hpp>

#include <tinyobjloader/tiny_obj_loader.h>

namespace StE {
namespace Resource {

class ModelLoader {
private:
	using texture_map_type = std::unordered_map<std::string, std::unique_ptr<gli::texture2D>>;

private:
	~ModelLoader() {}

	static StE::task<void> load_texture(const std::string &name, bool srgb, texture_map_type *texmap, const std::string &dir);
	static void process_model_mesh(const tinyobj::shape_t &shape, 
								   Graphics::Scene *scene,
								   std::vector<tinyobj::material_t> &materials,
								   texture_map_type &textures);

public:
	static task<bool> load_model_task(const std::string &file_path, Graphics::Scene *scene);
};

}
}
