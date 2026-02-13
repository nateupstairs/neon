#pragma once

#include "scap.h"
#include "scap_functions.h"

namespace Neon {

json
S_var(const vector<ScapNode>& params, const json& scope) {
	json result;

	ScapNode v = params[0];
	if (
		v.is_string()
		&&
		scope.is_object()
	) {
		string key = v.value.get<string>();
		bool exists = scope.contains(key);
		if (exists) {
			return scope.at(key);
		}
	}

	return result;
}

json
S_add(const vector<ScapNode>& params, const json& scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		ScapNode v = params[i];
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
S_sub(const vector<ScapNode>& params, const json& scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		ScapNode v = params[i];
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
S_mult(const vector<ScapNode>& params, const json& scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		ScapNode v = params[i];
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
S_div(const vector<ScapNode>& params, const json& scope) {
	json result = 0;

	for (i32 i = 0; i < params.size(); i++) {
		ScapNode v = params[i];
		json evaluated = v.eval(scope);

		if (i == 0) {
			result = evaluated;
		} else if (evaluated.is_number()) {
			result = result.get<f64>() / evaluated.get<f64>();
		}
	}

	return result;
}

std::unordered_map<string, ScapFunction> operations = {
	{"var", &S_var},
	{"+", &S_add},
	{"-", &S_sub},
	{"*", &S_mult},
	{"/", &S_div},
};

ScapFunction get_scap_function(const string& key) {
	bool exists = operations.contains(key);
	if (exists) {
		return operations[key];
	}
	return nullptr;
}

} // namespace Neon
