#pragma once

#include "base.h"
#include "uvalue.h"
#include "context.h"
#include "easing.h"

namespace Neon {

struct DopePoint {
	i64 time;
	UValue value;
	Easing ease;
};

struct Animation {
	i32 start;
	i32 end;
	bool loop;
	bool boomerang;
	vector<DopePoint> dopesheet;

	static uptr<Animation> make();

	void add_point(DopePoint);
	f64 solve(uptr<Context>&, i64);

	~Animation();
};

} // namespace Neon
