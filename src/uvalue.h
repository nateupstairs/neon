#pragma once

#include "base.h"

namespace Neon {

enum UUnit {
	None,
	Pixel,
	SU,
	NSU,
	CanvasWidth,
	CanvasHeight,
	RectWidth,
	RectHeight,
};

struct UValue {
	f64 value;
	UUnit unit;

	void set(string);
	void set(f64);
};

} // namespace Neon
