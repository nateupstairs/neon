#pragma once

#include <cmath>

#include "animation.h"
#include "context.h"
#include "uvalue.h"
#include "util.h"

namespace Neon {

uptr<Animation>
Animation::make() {
	uptr<Animation> x = std::make_unique<Animation>();
	return x;
}

void
Animation::add_point(DopePoint p) {
	this->dopesheet.push_back(p);
}

f64
Animation::solve(uptr<Context>& ctx, i64 frame_request) {
	i64 frame = frame_request;

	if (this->loop && this->dopesheet.size() > 1) {
		i64 loop_start = this->dopesheet.front().time;
		i64 loop_end = this->dopesheet.back().time;
		i64 loop_duration = loop_end - loop_start;
		i64 offset = frame - loop_start;
		i64 remainder = offset % loop_duration;

		if (remainder < 0) {
			remainder = remainder + loop_duration;
		}

		if (this->boomerang) {
			i64 direction = static_cast<i64>(
				std::floor((offset / loop_duration) % 2)
			);

			if (direction == 0) {
				frame = remainder;
			} else {
				frame = loop_duration - remainder;
			}
		} else {
			frame = remainder;
		}

		frame = loop_start + frame;
	}

	for (i16 i = 0; i < this->dopesheet.size(); i++) {
		DopePoint cursor = this->dopesheet.at(i);

		if (i == this->dopesheet.size() - 1) {
			return ctx->solve(cursor.value);
		}

		DopePoint next = this->dopesheet.at(i + 1);

		if (frame >= cursor.time && frame < next.time) {
			f64 prog = map_to_range(frame, cursor.time, next.time, 0, 1, true);

			prog = cursor.ease.solve(prog);

			f64 value = map_to_range(
				prog,
				0,
				1,
				ctx->solve(cursor.value),
				ctx->solve(next.value),
				false //
			);

			return value;
		}
	}

	return 0;
}

Animation::~Animation() { printf("Destroyed Animation\n"); }

} // namespace Neon
