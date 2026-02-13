#pragma once

#include "base.h"
#include "scap_functions.h"

namespace Neon {

enum ScapType {
	Command,
	Value
};

struct ScapNode;

struct ScapNode {
	ScapFunction command;
	vector<ScapNode> params;

	ScapType type;
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

ScapNode parse(string);

} // namespace Neon
