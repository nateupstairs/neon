#pragma once

#include "context.h"
#include "base.h"
#include "uvalue.h"

namespace Neon {

uptr<Context>
Context::make() {
	uptr<Context> x = std::make_unique<Context>();
	return x;
}

uptr<Context>
Context::clone() {
	uptr<Context> x = std::make_unique<Context>();

	x->screen_x = this->screen_x;
	x->screen_y = this->screen_y;
	x->rect_x = this->rect_x;
	x->rect_y = this->rect_y;
	x->pixel_size = this->pixel_size;

	return x;
}

void
Context::set_screen(i64 x, i64 y) {
	this->screen_x = x;
	this->screen_y = y;
}

void
Context::set_rect(f64 x, f64 y) {
	this->rect_x = x;
	this->rect_y = y;
}

void
Context::set_pixel(f64 x) {
	this->pixel_size = x;
}

f64
Context::solve(UValue v) {
	switch (v.unit) {
	case UUnit::None:
		return v.value;
		break;
	case UUnit::Pixel:
		return v.value * this->pixel_size;
		break;
	case UUnit::SU:
		return v.value * this->pixel_size;
		break;
	case UUnit::NSU:
		return v.value * this->pixel_size;
		break;
	case UUnit::RectWidth:
		return v.value/100.0 * this->rect_x;
		break;
	case UUnit::RectHeight:
		return v.value/100.0 * this->rect_y;
		break;
	case UUnit::CanvasWidth:
		return v.value/100.0 * this->screen_x;
		break;
	case UUnit::CanvasHeight:
		return v.value/100.0 * this->screen_y;
		break;
	default:
		return 0;
		break;
	}

	return 0;
}

} // namespace Neon
