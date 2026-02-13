#include "easing.h"

#include <cmath>
#include <numbers>

#include "_bezier.h"
#include "_spring.h"

namespace Neon {

f64 c1 = 1.70158;
f64 c2 = c1 * 1.525;
f64 c3 = c1 + 1;
f64 c4 = (2 * std::numbers::pi) / 3;
f64 c5 = (2 * std::numbers::pi) / 4.5;

f64
bounce_out(f64 x) {
	f64 n1 = 7.5625;
	f64 d1 = 2.75;

	if (x < 1 / d1) {
		return n1 * x * x;
	} else if (x < 2 / d1) {
		f64 p = x - (1.5 / d1);
		return n1 * p * p + 0.75;
	} else if (x < 2.5 / d1) {
		f64 p = x - (2.25 / d1);
		return n1 * p * p + 0.9375;
	} else {
		f64 p = x - (2.625 / d1);
		return n1 * p * p + 0.984375;
	}
}

f64
Easing::solve(f64 x) {
	switch (this->curve) {
	case Linear:
		return x;
		break;
	case HoldIn:
		return 0;
		break;
	case HoldOut:
		return 1;
		break;
	case PlateauIn:
		return 1;
		if (x > 0.0) {
			return 1.0;
		}
		return 0.0;
		break;
	case PlateauOut:
		return 1;
		if (x < 1.0) {
			return 0.0;
		}
		return 1.0;
		break;
	case Round:
		return std::round(x);
		break;
	case EaseInQuad:
		return x * x;
		break;
	case EaseOutQuad:
		return 1 - (1 - x) * (1 - x);
		break;
	case EaseInOutQuad:
		return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
		break;
	case EaseInCubic:
		return x * x * x;
		break;
	case EaseOutCubic:
		return 1 - pow(1 - x, 3);
		break;
	case EaseInOutCubic:
		return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
		break;
	case EaseInQuart:
		return x * x * x * x;
		break;
	case EaseOutQuart:
		return 1 - pow(1 - x, 4);
		break;
	case EaseInOutQuart:
		return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
		break;
	case EaseInQuint:
		return x * x * x * x * x;
		break;
	case EaseOutQuint:
		return 1 - pow(1 - x, 5);
		break;
	case EaseInOutQuint:
		return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
		break;
	case EaseInSine:
		return 1 - cos((x * std::numbers::pi) / 2);
		break;
	case EaseOutSine:
		return sin((x * std::numbers::pi) / 2);
		break;
	case EaseInOutSine:
		return -(cos(std::numbers::pi * x) - 1) / 2;
		break;
	case EaseInExpo:
		return x == 0 ? 0 : pow(2, 10 * x - 10);
		break;
	case EaseOutExpo:
		return x == 1 ? 1 : 1 - pow(2, -10 * x);
		break;
	case EaseInOutExpo:
		return x == 0	 ? 0
			   : x == 1	 ? 1
			   : x < 0.5 ? pow(2, 20 * x - 10) / 2
						 : (2 - pow(2, -20 * x + 10)) / 2;
		break;
	case EaseInCirc:
		return 1 - sqrt(1 - pow(x, 2));
		break;
	case EaseOutCirc:
		return sqrt(1 - pow(x - 1, 2));
		break;
	case EaseInOutCirc:
		return x < 0.5 ? (1 - sqrt(1 - pow(2 * x, 2))) / 2
					   : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
		break;
	case EaseInBack:
		return c3 * x * x * x - c1 * x * x;
		break;
	case EaseOutBack:
		return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
		break;
	case EaseInOutBack:
		return x < 0.5
				   ? (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
				   : (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) /
						 2;
		break;
	case EaseInElastic:
		return x == 0	? 0
			   : x == 1 ? 1
						: -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * c4);
		break;
	case EaseOutElastic:
		return x == 0	? 0
			   : x == 1 ? 1
						: pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
		break;
	case EaseInOutElastic:
		return x == 0	? 0
			   : x == 1 ? 1
			   : x < 0.5
				   ? -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2
				   : (pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 +
						 1;
		break;
	case EaseInBounce:
		return 1 - bounce_out(1 - x);
		break;
	case EaseOutBounce:
		return bounce_out(x);
		break;
	case EaseInOutBounce:
		return x < 0.5 ? (1 - bounce_out(1 - 2 * x)) / 2
					   : (1 + bounce_out(2 * x - 1)) / 2;
		break;
	default:
		if (this->curve == Bezier) {
			Neon::UnitBezier b_solver = Neon::UnitBezier(
				this->points[0],
				this->points[1],
				this->points[2],
				this->points[3]
			);

			return b_solver.solve(x, std::numeric_limits<double>::epsilon());
		} else if (this->curve == Spring) {
			Neon::SpringSolver s_solver = Neon::SpringSolver(
				this->points[0],
				this->points[1],
				this->points[2],
				this->points[3]
			);

			return s_solver.solve(x);
		} else {
			return 0;
		}
		break;
	}
}

} // namespace Neon
