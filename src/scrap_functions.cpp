#pragma once

#include "scrap.h"
#include <cmath>
#include "scrap_functions.h"

namespace Neon {
namespace Scrap {

json
S_var(const vector<Node>& params, const json& scope) {
	json result = json();

	if (params.empty()) {
		return result;
	}

	Node v = params[0];
	if (
		v.is_string()
		&&
		scope.is_object()
	) {
		string key = v.value.get<string>();
		bool exists = scope.contains(key);
		if (exists) {
			result = scope.at(key);
		} else {
			return json();
		}
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
S_if(const vector<Node>& params, const json& scope) {
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
S_equal(const vector<Node>& params, const json& scope) {
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
S_gt(const vector<Node>& params, const json& scope) {
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
S_lt(const vector<Node>& params, const json& scope) {
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
S_gte(const vector<Node>& params, const json& scope) {
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
S_lte(const vector<Node>& params, const json& scope) {
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
S_add(const vector<Node>& params, const json& scope) {
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
S_sub(const vector<Node>& params, const json& scope) {
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
S_mult(const vector<Node>& params, const json& scope) {
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
S_div(const vector<Node>& params, const json& scope) {
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
S_round(const vector<Node>& params, const json& scope) {
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
S_ceil(const vector<Node>& params, const json& scope) {
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
S_floor(const vector<Node>& params, const json& scope) {
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

std::unordered_map<string, ScrapFunction> operations = {
	{"var", &S_var},
	{"if", &S_if},
	{"=", &S_equal},
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
