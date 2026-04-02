#pragma once

#include "scrap.h"
#include "scrap_functions.h"

namespace Neon {
namespace Scrap {

json
S_get(const vector<Node>& params, Scope* scope) {
	json result = json();

	if (params.empty()) {
		return result;
	}

	Node v = params[0];
	if (v.is_string()) {
		string key = v.value.get<string>();
		bool exists = scope->exists(key);
		if (exists) {
			result = scope->get(key);
		} else {
			return json();
		}
	} else if (v.is_command()) {
		result = v.eval(scope);
	}

	for (i32 i = 1; i < params.size(); i++) {
		Node param = params[i];

		if (param.is_string() && result.is_object()) {
			string key = param.value.get<string>();

			if (result.contains(key)) {
				result = result.at(key);
			} else {
				return json();
			}
		} else if (param.is_number() && result.is_array()) {
			i32 index = param.value.get<i32>();

			if (index < result.size()) {
				result = result.at(index);
			} else {
				return json();
			}
		} else {
			return json();
		}
	}

	return result;
}

json
S_set(const vector<Node>& params, Scope* scope) {
	if (params.size() != 2) {
		return json();
	}

	Node key = params[0];
	Node val = params[1];
	if (
		key.is_string()
	) {
		string key_string = key.value.get<string>();
		json solved = val.eval(scope);
		scope->set(key_string, solved);
	}

	return json();
}

json
S_if(const vector<Node>& params, Scope* scope) {
	json result;

	Node comparator = params[0];
	json evaluated = comparator.eval(scope);

	if (params.size() > 1) {
		if (evaluated.is_boolean()) {
			if (evaluated.get<bool>() == true) {
				Node true_path = params[1];
				return true_path.eval(scope);
			} else {
				if (params.size() > 2) {
					Node false_path = params[2];
					return false_path.eval(scope);
				}
			}
		}
	}

	return result;
}

json
S_let(const vector<Node>& params, Scope* scope) {
	if (params.size() < 2) {
		return json();
	}

	scope->push_frame();
	scope->set("test-dude", "true");

	// run all but last item
	// these are all just setting scope
	for (i32 i = 0; i < params.size() - 1; i++) {
		Node v = params[i];
		json evaluated = v.eval(scope);
	}

	Node body = params[params.size() - 1];
	json result = body.eval(scope);

	scope->pop_frame();
	return result;
}

json
S_array(const vector<Node>& params, Scope* scope) {
	json result = json::parse("[]");

	for (i32 i = 0; i < params.size(); i++) {
		Node param = params[i];
		json eval = param.eval(scope);
		result.push_back(eval);
	}

	return result;
}

json
S_equal(const vector<Node>& params, Scope* scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		Node v = params[i];
		json evaluated = v.eval(scope);

		if (i == 0) {
			result = evaluated;
		} else {
			if (
				result.type() != evaluated.type()
				&&
				!(
					// we can be a little soft on numbers
					result.is_number()
					&&
					evaluated.is_number()
				)
			) {
				return false;
			}

			switch (result.type()) {
				case json::value_t::null:
					break;
				case json::value_t::object:
					return false;
					break;
				case json::value_t::array:
					return false;
					break;
				case json::value_t::binary:
					return false;
					break;
				case json::value_t::discarded:
					return false;
					break;
				case json::value_t::number_integer:
					if (result.get<i64>() != evaluated.get<i64>()) {
						return false;
					}
					break;
				case json::value_t::number_unsigned:
					if (result.get<u64>() != evaluated.get<u64>()) {
						return false;
					}
					break;
				case json::value_t::number_float:
					if (result.get<f64>() != evaluated.get<f64>()) {
						return false;
					}
					break;
				case json::value_t::boolean:
					if (result.get<bool>() != evaluated.get<bool>()) {
						return false;
					}
					break;
				case json::value_t::string:
					if (result.get<string>() != evaluated.get<string>()) {
						return false;
					}
					break;
			}
		}
	}

	return true;
}

json
S_nequal(const vector<Node>& params, Scope* scope) {
	return !S_equal(params, scope);
}

json
S_gt(const vector<Node>& params, Scope* scope) {
	if (params.size() < 2) {
		return false;
	}

	Node left_node = params[0];
	json left = left_node.eval(scope);

	Node right_node = params[1];
	json right = right_node.eval(scope);

	if (!left.is_number() || !right.is_number()) {
		return false;
	}

	if (left.get<f64>() > right.get<f64>()) {
		return true;
	} else {
		return false;
	}
}

json
S_lt(const vector<Node>& params, Scope* scope) {
	if (params.size() < 2) {
		return false;
	}

	Node left_node = params[0];
	json left = left_node.eval(scope);

	Node right_node = params[1];
	json right = right_node.eval(scope);

	if (!left.is_number() || !right.is_number()) {
		return false;
	}

	if (left.get<f64>() < right.get<f64>()) {
		return true;
	} else {
		return false;
	}
}

json
S_gte(const vector<Node>& params, Scope* scope) {
	if (params.size() < 2) {
		return false;
	}

	Node left_node = params[0];
	json left = left_node.eval(scope);

	Node right_node = params[1];
	json right = right_node.eval(scope);

	if (!left.is_number() || !right.is_number()) {
		return false;
	}

	if (left.get<f64>() >= right.get<f64>()) {
		return true;
	} else {
		return false;
	}
}

json
S_lte(const vector<Node>& params, Scope* scope) {
	if (params.size() < 2) {
		return false;
	}

	Node left_node = params[0];
	json left = left_node.eval(scope);

	Node right_node = params[1];
	json right = right_node.eval(scope);

	if (!left.is_number() || !right.is_number()) {
		return false;
	}

	if (left.get<f64>() <= right.get<f64>()) {
		return true;
	} else {
		return false;
	}
}

json
S_add(const vector<Node>& params, Scope* scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		Node v = params[i];
		json evaluated = v.eval(scope);

		if (i == 0) {
			result = evaluated;
		} else if (evaluated.is_number()) {
			result = result.get<f64>() + evaluated.get<f64>();
		}
	}

	return result;
}

json
S_sub(const vector<Node>& params, Scope* scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		Node v = params[i];
		json evaluated = v.eval(scope);

		if (i == 0) {
			result = evaluated;
		} else if (evaluated.is_number()) {
			result = result.get<f64>() - evaluated.get<f64>();
		}
	}

	return result;
}

json
S_mult(const vector<Node>& params, Scope* scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		Node v = params[i];
		json evaluated = v.eval(scope);

		if (i == 0) {
			result = evaluated;
		} else if (evaluated.is_number()) {
			result = result.get<f64>() * evaluated.get<f64>();
		}
	}

	return result;
}

json
S_div(const vector<Node>& params, Scope* scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		Node v = params[i];
		json evaluated = v.eval(scope);

		if (i == 0) {
			result = evaluated;
		} else if (evaluated.is_number()) {
			result = result.get<f64>() / evaluated.get<f64>();
		}
	}

	return result;
}

json
S_round(const vector<Node>& params, Scope* scope) {
	json result = 0;

	if (params.size() < 1) {
		return result;
	}

	Node v = params[0];
	json evaluated = v.eval(scope);

	if (evaluated.is_number()) {
		result = std::round(evaluated.get<f64>());
	}

	return result;
}

json
S_ceil(const vector<Node>& params, Scope* scope) {
	json result = 0;

	if (params.size() < 1) {
		return result;
	}

	Node v = params[0];
	json evaluated = v.eval(scope);

	if (evaluated.is_number()) {
		result = std::ceil(evaluated.get<f64>());
	}

	return result;
}

json
S_floor(const vector<Node>& params, Scope* scope) {
	json result = 0;

	if (params.size() < 1) {
		return result;
	}

	Node v = params[0];
	json evaluated = v.eval(scope);

	if (evaluated.is_number()) {
		result = std::floor(evaluated.get<f64>());
	}

	return result;
}

json
S_type(const vector<Node>& params, Scope* scope) {
	Node v = params[0];
	json evaluated = v.eval(scope);

	switch (evaluated.type()) {
		case json::value_t::null:
			return "null";
			break;
		case json::value_t::object:
			return "object";
			break;
		case json::value_t::array:
			return "array";
			break;
		case json::value_t::binary:
			return "binary";
			break;
		case json::value_t::discarded:
			return "discarded";
			break;
		case json::value_t::number_integer:
			return "number";
			break;
		case json::value_t::number_unsigned:
			return "number";
			break;
		case json::value_t::number_float:
			return "number";
			break;
		case json::value_t::boolean:
			return "boolean";
			break;
		case json::value_t::string:
			return "string";
			break;
		default:
			return "unknown";
			break;
	}
}

json
S_default(const vector<Node>& params, Scope* scope) {
	if (params.empty()) {
		return json();
	}

	Node value_node = params[0];
	json value = value_node.eval(scope);

	if (value.is_null()) {
		if (params.size() >= 2) {
			Node default_node = params[1];
			return default_node.eval(scope);
		}
	}

	return value;
}

json
S_to_string(const vector<Node>& params, Scope* scope) {
	if (params.empty()) {
		return json();
	}

	Node value_node = params[0];
	json value = value_node.eval(scope);

	if (value.is_number()) {
		// Convert number to string
		if (value.is_number_integer()) {
			return std::to_string(value.get<i64>());
		} else if (value.is_number_unsigned()) {
			return std::to_string(value.get<u64>());
		} else if (value.is_number_float()) {
			// Remove trailing zeros for float representation
			string str = std::to_string(value.get<f64>());
			str.erase(str.find_last_not_of('0') + 1, std::string::npos);
			if (str.back() == '.') {
				str.pop_back();
			}
			return str;
		}
	} else if (value.is_boolean()) {
		return value.get<bool>() ? "true" : "false";
	} else if (value.is_string()) {
		return value;
	} else if (value.is_null()) {
		return json();
	}

	return json();
}

json
S_to_number(const vector<Node>& params, Scope* scope) {
	if (params.size() < 1) {
		return json();
	}

	Node value_node = params[0];
	json value = value_node.eval(scope);

	if (value.is_string()) {
		string str = value.get<string>();
		try {
			// Try to parse as a floating point number
			f64 number = std::stod(str);
			return number;
		} catch (const std::exception& e) {
			// If parsing fails, return null
			return json();
		}
	} else if (value.is_number()) {
		// Already a number, return as is
		return value;
	} else if (value.is_boolean()) {
		// Convert boolean to number (true = 1, false = 0)
		return value.get<bool>() ? 1.0 : 0.0;
	}

	return json();
}

json
S_min(const vector<Node>& params, Scope* scope) {
	if (params.empty()) {
		return json();
	}

	f64 min_value = 0;
	bool found_number = false;

	// for (Node param : params) {
	for (const Node& param : params) {
		json evaluated = param.eval(scope);
		if (evaluated.is_number()) {
			f64 value = evaluated.get<f64>();
			if (
				!found_number
				||
				min_value > value
			) {
				min_value = value;
			}
			found_number = true;
		}
	}

	if (!found_number) {
		return json();
	}

	return min_value;
}

json
S_max(const vector<Node>& params, Scope* scope) {
	if (params.empty()) {
		return json();
	}

	f64 max_value = 0;
	bool found_number = false;

	for (Node param : params) {
		json evaluated = param.eval(scope);
		if (evaluated.is_number()) {
			f64 value = evaluated.get<f64>();

			if (
				!found_number
				||
				max_value < value
			) {
				max_value = value;
			}
			found_number = true;

		}
	}

	if (!found_number) {
		return json();
	}

	return max_value;
}

json
S_nth(const vector<Node>& params, Scope* scope) {
	if (params.size() < 2) {
		return json();
	}

	Node haystack = params[0];
	json evaluated = haystack.eval(scope);

	if (!evaluated.is_array()) {
		return json();
	}

	Node needle = params[1];
	json needle_eval = needle.eval(scope);

	if (!needle_eval.is_number_integer()) {
		return json();
	}
	i32 index = needle_eval.get<i32>();

	if (evaluated.size() > index) {
		return evaluated.at(index);
	}

	return json();
}

json
S_print(const vector<Node>& params, Scope* scope) {
	if (params.empty()) {
		return json();
	}

	Node haystack = params[0];
	json evaluated = haystack.eval(scope);

	return evaluated;
}

std::unordered_map<string, ScrapFunction> operations = {
	{"get", &S_get},
	{"set", &S_set},
	{"var", &S_get},
	{"if", &S_if},
	{"let", &S_let},
	{"array", &S_array},
	{"=", &S_equal},
	{"!=", &S_nequal},
	{">", &S_gt},
	{"<", &S_lt},
	{">=", &S_gte},
	{"<=", &S_lte},
	{"+", &S_add},
	{"-", &S_sub},
	{"*", &S_mult},
	{"/", &S_div},
	{"round", &S_round},
	{"ceil", &S_ceil},
	{"floor", &S_floor},
	{"type", &S_type},
	{"default", &S_default},
	{"to-string", &S_to_string},
	{"to-number", &S_to_number},
	{"min", &S_min},
	{"max", &S_max},
	{"nth", &S_nth},
	{"print", &S_print},
};

ScrapFunction get_scrap_function(const string& key) {
	bool exists = operations.contains(key);
	if (exists) {
		return operations[key];
	}
	return nullptr;
}

} // namespace Scrap
} // namespace Neon
