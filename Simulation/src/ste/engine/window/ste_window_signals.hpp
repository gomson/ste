//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <signal.hpp>

#include <pointer.hpp>

namespace ste {

class ste_window;

class ste_window_signals {
public:
	using window_resize_signal_type = signal<glm::i32vec2>;

	using hid_pointer_button_signal_type = signal<HID::pointer::B, HID::Status, HID::ModifierBits>;
	using hid_pointer_movement_signal_type = signal<glm::dvec2>;
	using hid_scroll_signal_type = signal<glm::dvec2>;
	using hid_keyboard_signal_type = signal<HID::keyboard::K, int, HID::Status, HID::ModifierBits>;

private:
	window_resize_signal_type framebuffer_resize_signal;

	hid_pointer_movement_signal_type hid_pointer_movement_signal;
	hid_pointer_button_signal_type hid_pointer_button_signal;
	hid_scroll_signal_type hid_scroll_signal;
	hid_keyboard_signal_type hid_keyboard_signal;

public:
	ste_window_signals(const ste_window *win);

	auto &signal_window_resize() { return framebuffer_resize_signal; }

	auto &signal_pointer_movement() { return hid_pointer_movement_signal; }
	auto &signal_pointer_button() { return hid_pointer_button_signal; }
	auto &signal_scroll() { return hid_scroll_signal; }
	auto &signal_keyboard() { return hid_keyboard_signal; }
};

}
