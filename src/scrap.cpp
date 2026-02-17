#pragma once

#include "scrap.h"
#include "scrap_functions.cpp"

namespace Neon {
namespace Scrap {

Scope::Scope(json frame) {
	this->stack.push_back(frame);
}

bool
Scope::exists(string x) {
	json last = this->stack.back();
	bool exists = last.contains(x);
	return exists;
}

json
Scope::get(string x) {
	json last = this->stack.back();
	bool exists = last.contains(x);
	if (exists) {
		return last.at(x);
	}
	return json();
}

void
Scope::set(string key, json value) {
	i32 last = this->stack.size() - 1;
	this->stack[last][key] = value;
	return;
}

void
Scope::push_frame() {
	json new_frame = this->stack.back();
	this->stack.push_back(new_frame);
	return;
}

void
Scope::pop_frame() {
	this->stack.pop_back();
	return;
}

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
Node::eval(Scope* scope) {
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

		// special cases
		if (f_name == "let") {
			i32 children = blob.size();
			if (children == 3) {
				// let block
				if (blob[1].is_array()) {
					json let_cmd = json::parse("[\"array\"]");
					i32 let_items = blob[1].size();
					for (i32 i = 0; i < let_items; i++) {
						json set_cmd = json::parse("[\"set\"]");
						i32 set_items = blob[1][i].size();
						for (i32 n = 0; n < set_items; n++) {
							set_cmd.push_back(blob[1][i][n]);
						}
						let_cmd.push_back(set_cmd);
					}
					node.params.push_back(parse_nodes(let_cmd));
				} else {
					node.params.push_back(parse_nodes(json()));
				}

				// do block
				json child_data = blob.at(2);
				node.params.push_back(parse_nodes(child_data));
			}
		// regular case
		} else if (node.command != nullptr) {
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
