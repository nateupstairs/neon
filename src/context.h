#pragma once

#include "base.h"
#include "uvalue.h"

namespace Neon {

struct Context {
	i64 screen_x;
	i64 screen_y;
	f64 rect_x;
	f64 rect_y;
	f64 pixel_size;

	static uptr<Context> make();
	uptr<Context> clone();

	void set_screen(i64, i64);
	void set_pixel(f64);
	void set_rect(f64, f64);

	f64 solve(UValue);
};

}
