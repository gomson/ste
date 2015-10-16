// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <gli/gli.hpp>

namespace StE {
namespace Text {

class glyph {
private:
	friend class glyph_factory;
	friend class glyph_manager;

public:
	struct glyph_metrics {
		std::int16_t width;
		std::int16_t height;
		std::int16_t start_y;
		std::int16_t start_x;

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
			ar & width;
			ar & height;
			ar & start_y;
			ar & start_x;
		}
	};

	static constexpr int padding = 16;
	static constexpr int ttf_pixel_size = 64;

private:
	gli::texture2D glyph_distance_field;
	glyph_metrics metrics;
	int advance_x;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {
		ar << metrics;
		ar << advance_x;

		ar << static_cast<std::size_t>(glyph_distance_field.dimensions().x);
		ar << static_cast<std::size_t>(glyph_distance_field.dimensions().y);
		ar << static_cast<int>(glyph_distance_field.levels());
		ar << static_cast<int>(glyph_distance_field.format());

		std::size_t size = glyph_distance_field[0].dimensions().x * glyph_distance_field[0].dimensions().y * 4;
		std::string data;
		data.resize(size);
		memcpy(&data[0], glyph_distance_field[0].data(), size);
		ar << data;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version) {
		ar >> metrics;
		ar >> advance_x;

		std::size_t w, h, l;
		gli::format format;
		ar >> w;
		ar >> h;
		ar >> l;
		ar >> format;
		glyph_distance_field = gli::texture2D(l, format, { w,h });

		std::string data;
		ar >> data;
		memcpy(glyph_distance_field[0].data(), &data[0], data.size());
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER();

public:
	glyph() {}

	glyph(glyph &&) = default;
	glyph &operator=(glyph &&) = default;
	glyph(const glyph &) = default;
	glyph &operator=(const glyph &) = default;

	bool empty() const { return glyph_distance_field.empty(); }
};

}
}
