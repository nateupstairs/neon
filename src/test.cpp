#include "base.h"
#include "easing.h"

#include "animation.cpp"
#include "context.cpp"
#include "uvalue.cpp"
#include "easing.cpp"
#include "scrap.cpp"

using namespace Neon;
using namespace Neon::Scrap;

int
main() {
	uptr<Context> ctx = std::make_unique<Context>();

	ctx->set_screen(1920, 1080);
	ctx->set_rect(500, 500);
	ctx->set_pixel(1.0);

	uptr<Context> ctx2 = ctx->clone();

	// ctx2->set_pixel(1.25);

	Animation a = (Animation){
		.start = 0,
		.end = 0,
		.loop = true,
		.boomerang = false,
	};

	uptr<Animation> aa = Animation::make();

	aa->start = 0;
	aa->end = 5000;
	aa->loop = false;
	aa->boomerang = false;

	DopePoint p1 = {
		.time = 0,
		.value = {
			.value = 0,
			.unit = UUnit::None,
		},
		.ease = {
			.curve = EaseCurve::Linear,
			.points = {0, 0, 0, 0},
		}
	};

	// p1.value.set("100rw");

	DopePoint p2 = {
		.time = 1000,
		.value = {
			.value = 100,
			.unit = UUnit::Pixel,
		},
		.ease = {
			.curve = EaseCurve::Linear,
			.points = {0, 0, 0, 0},
		}
	};

	// p2.value.set("201.223rw");
	//

	p1.ease.curve = Neon::Bezier;
	p1.ease.points[0] = 0.17;
	p1.ease.points[1] = 0.67;
	p1.ease.points[2] = 0.83;
	p1.ease.points[3] = 0.67;
	// p1.ease.points[0] = 0.25;
	// p1.ease.points[1] = 1.0;
	// p1.ease.points[2] = 0.25;
	// p1.ease.points[3] = 1.0;
	// p1.ease.points[0] = 0.42;
	// p1.ease.points[1] = 0;
	// p1.ease.points[2] = 0.58;
	// p1.ease.points[3] = 1;
	//

	// p1.ease.curve = Neon::EaseCurve::Spring;
	// p1.ease.points[0] = 1;
	// p1.ease.points[1] = 2000;
	// p1.ease.points[2] = 20;
	// p1.ease.points[3] = 0;

	aa->add_point(p1);
	aa->add_point(p2);

	f64 v1 = aa->solve(ctx, 0);
	f64 v2 = aa->solve(ctx, 100);
	f64 v3 = aa->solve(ctx, 500);
	f64 v4 = aa->solve(ctx, 700);
	f64 v5 = aa->solve(ctx, 1000);

	printf("%f\n", v1);
	printf("%f\n", v2);
	printf("%f\n", v3);
	printf("%f\n", v4);
	printf("%f\n", v5);

	json scope = json::parse(R"(
		{
			"test": "yes",
			"deep": {
				"var": {
					"test": false
				}
			}
		}
	)");
	Node n = parse(R"(
		["*",
			["+",
				1,
				["-", 10, 5]
			],
			1.5
		]
	)");
	json result = n.eval(scope);

	if (result.is_number()) {
		printf("eval: %f\n", result.get<f64>());
	}

	Node n2 = parse(R"(
		["var", "test"]
	)");
	json result2 = n2.eval(scope);

	if (result2.is_string()) {
		printf("eval: %s\n", result2.get<string>().c_str());
	}

	Node n3 = parse(R"(
		["if",
			["=",
				"yes",
				"no"
			],
			"true path",
			"false path"
		]
	)");
	json result3 = n3.eval(scope);

	if (result3.is_string()) {
		printf("eval: %s\n", result3.get<string>().c_str());
	}

	Node n4 = parse(R"(
		["var",
			"deep",
			"var",
			"test"
		]
	)");
	json result4 = n4.eval(scope);

	if (result4.is_boolean()) {
		printf("eval: %d\n", result4.get<bool>());
	}

	Node n5 = parse(R"(
		[">=",
			10,
			["floor", 10.00001]
		]
	)");
	json result5 = n5.eval(scope);

	if (result5.is_boolean()) {
		printf("eval: %d\n", result5.get<bool>());
	}

	return 0;
}
