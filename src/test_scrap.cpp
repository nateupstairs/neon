#include "base.h"

#include "scrap.cpp"

using namespace Neon;
using namespace Neon::Scrap;

struct Test {
	string name;
	string expr;
	json expected;
	json scope_data;
};

i32 passed = 0;
i32 failed = 0;

void
run_test(const Test& t) {
	Scope scope = Scope(t.scope_data.is_null() ? json::object() : t.scope_data);
	Node node = parse(t.expr);
	json result = node.eval(&scope);

	bool ok = (result == t.expected);

	if (ok) {
		passed++;
		printf("  PASS  %s\n", t.name.c_str());
	} else {
		failed++;
		printf("  FAIL  %s\n", t.name.c_str());
		printf("        expected: %s\n", t.expected.dump().c_str());
		printf("        got:      %s\n", result.dump().c_str());
	}
}

void
run_let_isolation_test() {
	// Verify let doesn't leak into outer scope
	json scope_data = json::parse(R"({"outer": "yes"})");
	Scope scope = Scope(scope_data);

	Node let_node = parse(R"(["let", [["x", 5]], ["var", "x"]])");
	json let_result = let_node.eval(&scope);

	Node after = parse(R"(["var", "x"])");
	json after_result = after.eval(&scope);

	bool ok = (let_result == json(5) && after_result.is_null());

	if (ok) {
		passed++;
		printf("  PASS  let: scope isolation\n");
	} else {
		failed++;
		printf("  FAIL  let: scope isolation\n");
		printf("        let result: %s (expected 5)\n", let_result.dump().c_str());
		printf("        outer 'x' after let: %s (expected null)\n", after_result.dump().c_str());
	}
}

int
main() {
	json basic_scope = json::parse(R"({
		"name": "hello",
		"count": 42,
		"flag": true,
		"nested": {"a": {"b": 99}},
		"arr": [10, 20, 30]
	})");

	// ── Parser / Static Values ──────────────────────────────────
	printf("\n── Parser / Static Values ──\n");
	vector<Test> parser_tests = {
		{"parse: bare number",         "51",      51,      nullptr},
		{"parse: bare string",         R"("hello")", "hello", nullptr},
		{"parse: bare bool true",      "true",    true,    nullptr},
		{"parse: bare bool false",     "false",   false,   nullptr},
		{"parse: bare null",           "null",    nullptr, nullptr},
		{"parse: unknown function",    R"(["fake", 1])", nullptr, nullptr},
	};
	for (const auto& t : parser_tests) run_test(t);

	// ── Arithmetic ──────────────────────────────────────────────
	printf("\n── Arithmetic ──\n");
	vector<Test> arith_tests = {
		{"add: basic",            R"(["+", 1, 2])",           3.0,     nullptr},
		{"add: multi-arg",        R"(["+", 1, 2, 3])",        6.0,     nullptr},
		{"add: nested",           R"(["*", ["+", 1, 2], 3])", 9.0,     nullptr},
		{"sub: basic",            R"(["-", 10, 3])",           7.0,     nullptr},
		{"mult: basic",           R"(["*", 4, 5])",            20.0,    nullptr},
		{"div: basic",            R"(["/", 10, 2])",           5.0,     nullptr},
		{"div: float result",     R"(["/", 10, 3])",           json(10.0 / 3.0), nullptr},
		{"add: single arg",       R"(["+", 5])",               5,       nullptr},
		{"sub: single arg",       R"(["-", 5])",               5,       nullptr},
		{"add: no args",          R"(["+"])",                  0,       nullptr},
		{"add: skip non-number",  R"(["+", 1, "a", 2])",      3.0,     nullptr},
	};
	for (const auto& t : arith_tests) run_test(t);

	// ── Rounding ────────────────────────────────────────────────
	printf("\n── Rounding ──\n");
	vector<Test> round_tests = {
		{"round: 1.5",          R"(["round", 1.5])",    2.0,    nullptr},
		{"round: 1.4",          R"(["round", 1.4])",    1.0,    nullptr},
		{"ceil: 1.1",           R"(["ceil", 1.1])",     2.0,    nullptr},
		{"ceil: negative",      R"(["ceil", -1.1])",    -1.0,   nullptr},
		{"floor: 1.9",          R"(["floor", 1.9])",    1.0,    nullptr},
		{"floor: negative",     R"(["floor", -1.1])",   -2.0,   nullptr},
		{"floor: non-number",   R"(["floor", "abc"])",  0,      nullptr},
		{"floor: no args",      R"(["floor"])",         0,      nullptr},
	};
	for (const auto& t : round_tests) run_test(t);

	// ── Comparisons ─────────────────────────────────────────────
	printf("\n── Comparisons ──\n");
	vector<Test> cmp_tests = {
		{"eq: same strings",        R"(["=", "a", "a"])",         true,  nullptr},
		{"eq: diff strings",        R"(["=", "a", "b"])",         false, nullptr},
		{"eq: same numbers",        R"(["=", 5, 5])",             true,  nullptr},
		{"eq: int vs float",        R"(["=", 5, 5.0])",           true,  nullptr},
		{"eq: type mismatch",       R"(["=", 5, "5"])",           false, nullptr},
		{"eq: multi-arg all same",  R"(["=", 1, 1, 1])",          true,  nullptr},
		{"eq: multi-arg one diff",  R"(["=", 1, 1, 2])",          false, nullptr},
		{"eq: booleans",            R"(["=", true, true])",        true,  nullptr},
		{"eq: nulls",               R"(["=", null, null])",        true,  nullptr},
		{"neq: different",          R"(["!=", 1, 2])",             true,  nullptr},
		{"neq: same",               R"(["!=", 1, 1])",             false, nullptr},
		{"gt: true",                R"([">", 5, 3])",              true,  nullptr},
		{"gt: false",               R"([">", 3, 5])",              false, nullptr},
		{"gt: equal",               R"([">", 5, 5])",              false, nullptr},
		{"lt: true",                R"(["<", 3, 5])",              true,  nullptr},
		{"lt: false",               R"(["<", 5, 3])",              false, nullptr},
		{"gte: greater",            R"([">=", 5, 3])",             true,  nullptr},
		{"gte: equal",              R"([">=", 5, 5])",             true,  nullptr},
		{"gte: less",               R"([">=", 3, 5])",             false, nullptr},
		{"lte: less",               R"(["<=", 3, 5])",             true,  nullptr},
		{"lte: equal",              R"(["<=", 5, 5])",             true,  nullptr},
		{"lte: greater",            R"(["<=", 5, 3])",             false, nullptr},
		{"gt: non-number",          R"([">", "a", "b"])",          false, nullptr},
		{"gt: no args",             R"([">"])",                    false, nullptr},
	};
	for (const auto& t : cmp_tests) run_test(t);

	// ── Conditionals ────────────────────────────────────────────
	printf("\n── Conditionals ──\n");
	vector<Test> if_tests = {
		{"if: true path",           R"(["if", true, "yes", "no"])",           "yes",    nullptr},
		{"if: false path",          R"(["if", false, "yes", "no"])",          "no",     nullptr},
		{"if: false no else",       R"(["if", false, "yes"])",                nullptr,  nullptr},
		{"if: non-bool condition",  R"(["if", 5, "yes", "no"])",              nullptr,  nullptr},
		{"if: nested condition",    R"(["if", ["=", 1, 1], "match", "no"])",  "match",  nullptr},
		{"if: nested false",        R"(["if", ["=", 1, 2], "match", "no"])",  "no",     nullptr},
	};
	for (const auto& t : if_tests) run_test(t);

	// ── Scope: get / var ────────────────────────────────────────
	printf("\n── Scope: get / var ──\n");
	vector<Test> scope_tests = {
		{"var: simple",            R"(["var", "name"])",              "hello",  basic_scope},
		{"var: number",            R"(["var", "count"])",             42,       basic_scope},
		{"var: bool",              R"(["var", "flag"])",              true,     basic_scope},
		{"var: missing key",       R"(["var", "missing"])",           nullptr,  basic_scope},
		{"var: deep access",       R"(["var", "nested", "a", "b"])", 99,       basic_scope},
		{"var: deep missing",      R"(["var", "nested", "a", "x"])", nullptr,  basic_scope},
		{"get: array index",       R"(["get", "arr", 0])",           10,       basic_scope},
		{"get: array index 2",     R"(["get", "arr", 2])",           30,       basic_scope},
		{"get: out of bounds",     R"(["get", "arr", 5])",           nullptr,  basic_scope},
		{"get: command first arg", R"(["get", ["var", "nested"], "a", "b"])", 99, basic_scope},
	};
	for (const auto& t : scope_tests) run_test(t);

	// ── Let ─────────────────────────────────────────────────────
	printf("\n── Let ──\n");
	vector<Test> let_tests = {
		{"let: basic static",
			R"(["let", [["x", 5]], ["var", "x"]])",
			5, nullptr},
		{"let: expression value",
			R"(["let", [["x", ["+", 1, 2]]], ["var", "x"]])",
			3.0, nullptr},
		{"let: chained bindings",
			R"(["let", [["a", 1], ["b", ["+", ["var", "a"], 1]]], ["var", "b"]])",
			2.0, nullptr},
		{"let: preserves outer scope",
			R"(["let", [["x", 1]], ["var", "name"]])",
			"hello", basic_scope},
		{"let: shadow outer var",
			R"(["let", [["name", "world"]], ["var", "name"]])",
			"world", basic_scope},
		{"let: nested lets",
			R"(["let", [["x", 1]], ["let", [["y", 2]], ["+", ["var", "x"], ["var", "y"]]]])",
			3.0, nullptr},
		{"let: body uses binding in expression",
			R"(["let", [["n", 10]], ["*", ["var", "n"], 2]])",
			20.0, nullptr},
	};
	for (const auto& t : let_tests) run_test(t);

	run_let_isolation_test();

	// ── Type Functions ──────────────────────────────────────────
	printf("\n── Type Functions ──\n");
	vector<Test> type_tests = {
		{"type: number",     R"(["type", 5])",              "number",  nullptr},
		{"type: string",     R"(["type", "hi"])",            "string",  nullptr},
		{"type: boolean",    R"(["type", true])",            "boolean", nullptr},
		{"type: null",       R"(["type", null])",            "null",    nullptr},
		{"type: array",      R"(["type", ["array", 1]])",    "array",   nullptr},
	};
	for (const auto& t : type_tests) run_test(t);

	// ── Type Conversion ─────────────────────────────────────────
	printf("\n── Type Conversion ──\n");
	vector<Test> conv_tests = {
		{"to-string: integer",     R"(["to-string", 42])",     "42",     nullptr},
		{"to-string: bool true",   R"(["to-string", true])",   "true",   nullptr},
		{"to-string: bool false",  R"(["to-string", false])",  "false",  nullptr},
		{"to-string: string",      R"(["to-string", "hi"])",   "hi",     nullptr},
		{"to-string: null",        R"(["to-string", null])",   nullptr,  nullptr},
		{"to-number: valid str",   R"(["to-number", "3.14"])", 3.14,     nullptr},
		{"to-number: invalid str", R"(["to-number", "abc"])",  nullptr,  nullptr},
		{"to-number: bool true",   R"(["to-number", true])",   1.0,      nullptr},
		{"to-number: bool false",  R"(["to-number", false])",  0.0,      nullptr},
		{"to-number: number",      R"(["to-number", 7])",      7,        nullptr},
	};
	for (const auto& t : conv_tests) run_test(t);

	// ── Array / Collection ──────────────────────────────────────
	printf("\n── Array / Collection ──\n");
	vector<Test> arr_tests = {
		{"array: basic",         R"(["array", 1, 2, 3])",                   json::parse("[1,2,3]"),       nullptr},
		{"array: empty",         R"(["array"])",                             json::parse("[]"),            nullptr},
		{"array: mixed types",   R"(["array", 1, "two", true, null])",      json::parse(R"([1,"two",true,null])"), nullptr},
		{"nth: index 0",         R"(["nth", ["array", "a", "b", "c"], 0])", "a",     nullptr},
		{"nth: index 1",         R"(["nth", ["array", "a", "b", "c"], 1])", "b",     nullptr},
		{"nth: non-array",       R"(["nth", 5, 0])",                         nullptr, nullptr},
		{"nth: no args",         R"(["nth"])",                               nullptr, nullptr},
	};
	for (const auto& t : arr_tests) run_test(t);

	// ── Min / Max ───────────────────────────────────────────────
	printf("\n── Min / Max ──\n");
	vector<Test> minmax_tests = {
		{"min: basic",          R"(["min", 3, 1, 2])",        1.0,     nullptr},
		{"min: single",         R"(["min", 7])",               7.0,     nullptr},
		{"min: negative",       R"(["min", -5, 0, 5])",        -5.0,    nullptr},
		{"min: skip non-num",   R"(["min", 3, "a", 1])",       1.0,     nullptr},
		{"min: no args",        R"(["min"])",                   nullptr, nullptr},
		{"max: basic",          R"(["max", 3, 1, 2])",         3.0,     nullptr},
		{"max: single",         R"(["max", 7])",                7.0,     nullptr},
		{"max: negative",       R"(["max", -5, 0, 5])",        5.0,     nullptr},
		{"max: no args",        R"(["max"])",                   nullptr, nullptr},
	};
	for (const auto& t : minmax_tests) run_test(t);

	// ── Default ─────────────────────────────────────────────────
	printf("\n── Default ──\n");
	vector<Test> default_tests = {
		{"default: null fallback",   R"(["default", null, "fb"])",    "fb",    nullptr},
		{"default: has value",       R"(["default", "val", "fb"])",   "val",   nullptr},
		{"default: zero not null",   R"(["default", 0, "fb"])",       0,       nullptr},
		{"default: false not null",  R"(["default", false, "fb"])",   false,   nullptr},
		{"default: no args",         R"(["default"])",                nullptr, nullptr},
	};
	for (const auto& t : default_tests) run_test(t);

	// ── Deep / Integration ──────────────────────────────────────
	printf("\n── Integration ──\n");
	vector<Test> integration_tests = {
		{"deep nesting",
			R"(["+", ["+", ["+", ["+", 1, 1], 1], 1], 1])",
			5.0, nullptr},
		{"let + if + arithmetic",
			R"(["let", [["x", 10], ["y", 3]], ["if", [">", ["var", "x"], ["var", "y"]], ["-", ["var", "x"], ["var", "y"]], 0]])",
			7.0, nullptr},
		{"default with expression",
			R"(["default", ["var", "missing"], ["+", 1, 1]])",
			2.0, basic_scope},
		{"type check pattern",
			R"(["if", ["=", ["type", 42], "number"], "is number", "not number"])",
			"is number", nullptr},
	};
	for (const auto& t : integration_tests) run_test(t);

	// ── Summary ─────────────────────────────────────────────────
	printf("\n────────────────────────────\n");
	printf("  %d passed, %d failed, %d total\n\n", passed, failed, passed + failed);

	return failed > 0 ? 1 : 0;
}
