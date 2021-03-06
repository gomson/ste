// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <cstddef>

namespace ste {

template<typename T = std::size_t>
struct range {
	T start, length;

	range() = default;
	range(const T &start, const T &length) : start(start), length(length) {}

	template<typename S>
	bool overlaps(const range<S> &r) const {
		return  (start < r.start + r.length) &&
			(r.start < start + length);
	}

	bool operator==(const range &r) const {
		return start == r.start && length == r.length;
	}
	bool operator!=(const range &r) const {
		return !(*this == r);
	}
	bool operator<(const range &r) const {
		return start < r.start;
	}
	bool operator>(const range &r) const {
		return r < *this;
	}
};

template <typename T, typename S>
bool inline operator==(const range<T> &lhs, const range<S> &rhs) {
	return lhs.start == rhs.start && lhs.length == rhs.length;
}
template <typename T, typename S>
bool inline operator!=(const range<T> &lhs, const range<S> &rhs) {
	return !(lhs == rhs);
}

}
