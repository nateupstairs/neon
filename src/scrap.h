#pragma once

#include "base.h"
#include "scrap_functions.h"

namespace Neon {
namespace Scrap {

enum ScrapType {
	Command,
	Value
};

struct Node;

struct Node {
	ScrapFunction command;
	vector<Node> params;

	ScrapType type;
	json value;

	json eval(json);
	bool is_command();
	bool is_value();
	bool is_null();
	bool is_boolean();
	bool is_number();
	bool is_string();

	void set(json);
};

Node parse(string);

} // namespace Scrap
} // namespace Neon
