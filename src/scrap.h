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

	json eval(Scope*) const;
	bool is_command() const;
	bool is_value() const;
	bool is_null() const;
	bool is_boolean() const;
	bool is_number() const;
	bool is_string() const;

	void set(json);
};

Node parse(string);

} // namespace Scrap
} // namespace Neon
