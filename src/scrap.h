#pragma once

#include "base.h"
#include "scrap_functions.h"

namespace Neon {
namespace Scrap {

enum ScrapType {
	Command,
	Value
};

struct Scope {
	Scope(json);

	vector<json> stack;
	bool exists(string);
	json get(string);
	void set(string, json);
	void push_frame();
	void pop_frame();
};

struct Node;
struct Node {
	ScrapFunction command;
	vector<Node> params;

	ScrapType type;
	json value;

	json eval(Scope*);
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
