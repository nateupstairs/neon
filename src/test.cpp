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

	json scope_json = json::parse(R"(
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
	Scope scope = Scope(scope_json);
	json result = n.eval(&scope);

	if (result.is_number()) {
		printf("eval: %f\n", result.get<f64>());
	}

	Node n2 = parse(R"(
		["var", "test"]
	)");
	json result2 = n2.eval(&scope);

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
	json result3 = n3.eval(&scope);

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
	json result4 = n4.eval(&scope);

	if (result4.is_boolean()) {
		printf("eval: %d\n", result4.get<bool>());
	}

	Node n5 = parse(R"(
		[">=",
			10,
			["floor", 10.00001]
		]
	)");
	json result5 = n5.eval(&scope);

	if (result5.is_boolean()) {
		printf("eval: %d\n", result5.get<bool>());
	}

	json component_scope_json = json::parse(R"({
		"component": ["ds-webflow-sidepanel", {
	        "start": 54300,
	        "duration": 5000,
	        "build": 56300,
	        "z-index": 2,
	        "handle": "8c5608ab-cf91-43db-9251-74bf1f5dcda6",
			"media": {
			    "file": "cloud://uploads/57c6a125131e4776b8fe2179ace98e58.mp4",
			    "duration": 50300,
			    "originalDuration": 50300,
			    "slip": 0,
			    "justify": "fill",
			    "meta": {
			        "hasAudio": true,
			        "fileType": "video/mp4",
			        "assetType": "video"
			    }
			},
	        "meta": {
	            "display": "List with Media",
	            "isClipComponent": true,
	            "lastUpdated": 1770848134575,
	            "tag": {
	                "id": "64d3f08a40686237d0944c66",
	                "name": "List",
	                "createdAt": "2023-08-09T20:01:14.274Z",
	                "updatedAt": "2023-08-09T20:01:14.274Z"
	            },
	            "transcriptNodeKeys": [
	                "698cff7039a8de700895cbd6-space-0"
	            ]
	        },
	        "swap-sides": true,
	        "transcript": []
		}]
	})");

	Scope component_scope = Scope(component_scope_json);

	Node n6 = parse(R"(
		["let",
			[
				["media-dur",
					["get",
						"component",
						1,
						"media",
						"originalDuration"
					]
				]
			],
			["if",
				["=",
					["type", ["get", "media-dur"]],
					"number"
				],
				["floor", ["get", "media-dur"]],
				null
			]
		]
	)");
	json result6 = n6.eval(&component_scope);

	printf("media-dur: %s\n", result6.dump().c_str());

	Node n7 = parse(R"(
		["let",
			[
				["one", 1],
				["two", 2]
			],
			["+", ["var", "one"], ["var", "two"]]
		]
	)");
	json result7 = n7.eval(&component_scope);
	printf("debuging: %s\n", result7.dump().c_str());

	return 0;
}
