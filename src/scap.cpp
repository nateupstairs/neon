#pragma once

#include "scap.h"
#include "scap_functions.cpp"

namespace Neon {

void
ScapNode::set(json value) {
	this->type = ScapType::Value;
	this->value = value;
}

bool
ScapNode::is_null() {
	if (this->type == ScapType::Value) {
		return this->value.is_null();
	}

	return false;
}

bool
ScapNode::is_boolean() {
	if (this->type == ScapType::Value) {
		return this->value.is_boolean();
	}

	return false;
}

bool
ScapNode::is_number() {
	if (this->type == ScapType::Value) {
		return this->value.is_number();
	}

	return false;
}

bool
ScapNode::is_string() {
	if (this->type == ScapType::Value) {
		return this->value.is_string();
	}

	return false;
}

bool
ScapNode::is_command() {
	if (this->type == ScapType::Command) {
		return true;
	}

	return false;
}

bool
ScapNode::is_value() {
	if (this->type != ScapType::Command) {
		return true;
	}

	return false;
}

json
ScapNode::eval(json scope) {
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

ScapNode
parse_nodes(json blob) {
	ScapNode node = (ScapNode){};

	if (blob.is_array()) {
		node.type = ScapType::Command;

		json cmd_data = blob.at(0);
		string f_name = cmd_data.get<string>();
		ScapFunction exists = get_scap_function(f_name);
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
			node.type = ScapType::Value;
			node.value = nullptr;
		}
	} else {
		node.set(blob);
	}

	return node;
}

ScapNode
parse(string s) {
	json blob = json::parse(s);

	if (blob.is_array()) {
		return parse_nodes(blob);
	}

	ScapNode x = (ScapNode){};
	return x;
}

} // namespace Neon
