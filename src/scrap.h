#pragma once

#include "base.h"
#include "scrap_functions.h"

namespace Neon {
namespace Scrap {

enum ScrapType {
	Command,
	Value
};

struct Trace {
	struct Frame {
		vector<json> children;
	};

	bool active = false;
	vector<Frame> stack;
	json tree;

	void start() {
		active = true;
		stack.clear();
		tree = nullptr;
	}

	json stop() {
		active = false;
		json out = tree;
		tree = nullptr;
		stack.clear();
		return out;
	}

	void enter() {
		if (!active) return;
		stack.push_back(Frame{});
	}

	void exit(json result, bool is_command) {
		if (!active) return;

		Frame frame = stack.back();
		stack.pop_back();

		json trace;
		if (is_command) {
			trace = json::array();
			trace.push_back(result);
			for (auto& c : frame.children) {
				trace.push_back(c);
			}
		} else {
			trace = result;
		}

		if (stack.empty()) {
			tree = trace;
		} else {
			stack.back().children.push_back(trace);
		}
	}
};

struct Scope {
	Scope(json);

	vector<json> stack;
	Trace* trace = nullptr;
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
