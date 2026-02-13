#pragma once

#include "uvalue.h"

namespace Neon {

void
UValue::set(string x) {
	if (x.empty()) {
		return;
	}

	i32 break_at = 0;

	for (i32 i = 0; i < x.size(); i++) {
		unsigned char current = static_cast<unsigned char>(x.at(i));

		if (std::isalpha(current)) {
			break_at = i;
			break;
		}
	}

	if (break_at == 0) {
		return;
	}

	std::string num = x.substr(0, break_at);
	std::string unit = x.substr(break_at);

	this->value = std::stod(num.c_str());

	if (unit.empty()) {
		this->unit = UUnit::None;
	}
	else if (unit == "px") {
		this->unit = UUnit::Pixel;
	}
	else if (unit == "rw") {
		this->unit = UUnit::RectWidth;
	}
	else if (unit == "rh") {
		this->unit = UUnit::RectHeight;
	}
	else if (unit == "cw") {
		this->unit = UUnit::CanvasWidth;
	}
	else if (unit == "ch") {
		this->unit = UUnit::CanvasHeight;
	}
	else if (unit == "su") {
		this->unit = UUnit::SU;
	}
	else if (unit == "nsu") {
		this->unit = UUnit::NSU;
	}

}

} // namespace Neon
