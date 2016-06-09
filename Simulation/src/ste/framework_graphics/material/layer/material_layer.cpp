
#include "stdafx.hpp"
#include "material_layer.hpp"

using namespace StE::Graphics;

material_layer::material_layer() {
	material_sampler.set_wrap_s(Core::TextureWrapMode::Wrap);
	material_sampler.set_wrap_t(Core::TextureWrapMode::Wrap);
	material_sampler.set_wrap_r(Core::TextureWrapMode::Wrap);
	material_sampler.set_min_filter(Core::TextureFiltering::Linear);
	material_sampler.set_mag_filter(Core::TextureFiltering::Linear);
	material_sampler.set_mipmap_filter(Core::TextureFiltering::Linear);
	material_sampler.set_anisotropic_filter(8);
}

StE::Core::texture_handle material_layer::handle_for_texture(const Core::Texture2D *t) const {
	Core::texture_handle h;
	if (t != nullptr) {
		h = t->get_texture_handle(material_sampler);
		h.make_resident();
	}
	return h;
}
