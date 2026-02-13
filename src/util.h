#pragma once

#include "base.h"

namespace Neon {

f64
map_to_range(
	f64 value,
	f64 in_min,
	f64 in_max,
	f64 out_min,
	f64 out_max,
	bool clamped //
) {
	f64 v =
		((value - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;

	if (clamped) {
		f64 lower = std::min(out_min, out_max);
		f64 upper = std::max(out_min, out_max);

		return std::max(lower, std::min(v, upper));
	}

	return v;
}

f64
lerp(f64 factor, f64 a, f64 b) {
	return a * (1.0 - factor) + (b * factor);
}

} // namespace Neon
