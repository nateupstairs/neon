#pragma once

#include "scrap.h"
#include "scrap_functions.cpp"

namespace Neon {
namespace Scrap {

void
Node::set(json value) {
	this->type = ScrapType::Value;
	this->value = value;
}

bool
Node::is_null() {
	if (this->type == ScrapType::Value) {
		return this->value.is_null();
	}

	return false;
}

bool
Node::is_boolean() {
	if (this->type == ScrapType::Value) {
		return this->value.is_boolean();
	}

	return false;
}

bool
Node::is_number() {
	if (this->type == ScrapType::Value) {
		return this->value.is_number();
	}

	return false;
}

bool
Node::is_string() {
	if (this->type == ScrapType::Value) {
		return this->value.is_string();
	}

	return false;
}

bool
Node::is_command() {
	if (this->type == ScrapType::Command) {
		return true;
	}

	return false;
}

bool
Node::is_value() {
	if (this->type != ScrapType::Command) {
		return true;
	}

	return false;
}

json
Node::eval(json scope) {
	if (this->is_command()) {
		if (this->command != nullptr) {
			json evaluated = this->command(this->params, scope);
			return evaluated;
		} else {
			return this->value;
		}
	} else {
		return this->value;
	}
}

Node
parse_nodes(json blob) {
	Node node = (Node){};

	if (blob.is_array()) {
		node.type = ScrapType::Command;

		json cmd_data = blob.at(0);
		string f_name = cmd_data.get<string>();
		ScrapFunction exists = get_scrap_function(f_name);
		if (exists != nullptr) {
			node.command = exists;
		}

		if (node.command != nullptr) {
			i32 children = blob.size();
			for (i32 i = 1; i < children; i++) {
				json child_data = blob.at(i);
				node.params.push_back(parse_nodes(child_data));
			}
		} else {
			node.type = ScrapType::Value;
			node.value = nullptr;
		}
	} else {
		node.set(blob);
	}

	return node;
}

Node
parse(string s) {
	json blob = json::parse(s);

	if (blob.is_array()) {
		return parse_nodes(blob);
	}

	Node x = (Node){};
	return x;
}

} // namespace Scrap
} // namespace Neon
