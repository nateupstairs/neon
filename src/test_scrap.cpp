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
		{"mod: basic",            R"(["mod", 10, 3])",         1.0,     nullptr},
		{"mod: float",            R"(["mod", 10.5, 3])",       json(std::fmod(10.5, 3.0)), nullptr},
		{"mod: divide by zero",   R"(["mod", 10, 0])",         nullptr, nullptr},
		{"mod: non-number",       R"(["mod", "a", 3])",        nullptr, nullptr},
		{"mod: no args",          R"(["mod"])",                nullptr, nullptr},
		{"abs: positive",         R"(["abs", 5])",             5.0,     nullptr},
		{"abs: negative",         R"(["abs", -5])",            5.0,     nullptr},
		{"abs: zero",             R"(["abs", 0])",             0.0,     nullptr},
		{"abs: non-number",       R"(["abs", "hi"])",          nullptr, nullptr},
		{"abs: no args",          R"(["abs"])",                nullptr, nullptr},
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

	// ── Logic: and / or / not ───────────────────────────────────
	printf("\n── Logic ──\n");
	vector<Test> logic_tests = {
		{"and: all true",           R"(["and", true, true])",          true,  nullptr},
		{"and: one false",          R"(["and", true, false])",         false, nullptr},
		{"and: all false",          R"(["and", false, false])",        false, nullptr},
		{"and: with null",          R"(["and", true, null])",          false, nullptr},
		{"and: no args",            R"(["and"])",                      true,  nullptr},
		{"and: single true",        R"(["and", true])",                true,  nullptr},
		{"and: three args",         R"(["and", true, true, true])",    true,  nullptr},
		{"and: truthy non-bool",    R"(["and", 1, "hi"])",             true,  nullptr},
		{"or: all true",            R"(["or", true, true])",           true,  nullptr},
		{"or: one true",            R"(["or", false, true])",          true,  nullptr},
		{"or: all false",           R"(["or", false, false])",         false, nullptr},
		{"or: with null",           R"(["or", null, false])",          false, nullptr},
		{"or: no args",             R"(["or"])",                       false, nullptr},
		{"or: truthy non-bool",     R"(["or", false, 1])",             true,  nullptr},
		{"or: truthy string",       R"(["or", false, "hi"])",          true,  nullptr},
		{"not: true",               R"(["not", true])",                false, nullptr},
		{"not: false",              R"(["not", false])",               true,  nullptr},
		{"not: null",               R"(["not", null])",                true,  nullptr},
		{"not: truthy non-bool",    R"(["not", 1])",                   false, nullptr},
		{"not: no args",            R"(["not"])",                      true,  nullptr},
	};
	for (const auto& t : logic_tests) run_test(t);

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

	// ── Set ─────────────────────────────────────────────────────
	printf("\n── Set ──\n");
	{
		json scope_data = json::parse(R"({})");
		Scope scope = Scope(scope_data);

		Node set_node = parse(R"(["set", "x", 42])");
		set_node.eval(&scope);

		Node get_node = parse(R"(["var", "x"])");
		json result = get_node.eval(&scope);

		bool ok = (result == json(42));
		if (ok) {
			passed++;
			printf("  PASS  set: basic set and retrieve\n");
		} else {
			failed++;
			printf("  FAIL  set: basic set and retrieve\n");
			printf("        expected: 42, got: %s\n", result.dump().c_str());
		}
	}

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
		{"list: alias",          R"(["list", 1, 2, 3])",                    json::parse("[1,2,3]"),       nullptr},
		{"nth: index 0",         R"(["nth", ["array", "a", "b", "c"], 0])", "a",     nullptr},
		{"nth: index 1",         R"(["nth", ["array", "a", "b", "c"], 1])", "b",     nullptr},
		{"nth: non-array",       R"(["nth", 5, 0])",                         nullptr, nullptr},
		{"nth: no args",         R"(["nth"])",                               nullptr, nullptr},
		{"length: array",        R"(["length", ["array", 1, 2, 3]])",       3,       nullptr},
		{"length: empty array",  R"(["length", ["array"]])",                0,       nullptr},
		{"length: string",       R"(["length", "hello"])",                  5,       nullptr},
		{"length: empty string", R"(["length", ""])",                       0,       nullptr},
		{"length: non-coll",     R"(["length", 42])",                       nullptr, nullptr},
		{"length: no args",      R"(["length"])",                           nullptr, nullptr},
		{"append: two arrays",   R"(["append", ["array", 1, 2], ["array", 3, 4]])", json::parse("[1,2,3,4]"), nullptr},
		{"append: three arrays", R"(["append", ["array", 1], ["array", 2], ["array", 3]])", json::parse("[1,2,3]"), nullptr},
		{"append: empty arrays", R"(["append", ["array"], ["array"]])",     json::parse("[]"),            nullptr},
		{"append: skip non-arr", R"(["append", ["array", 1], 5, ["array", 2]])", json::parse("[1,2]"), nullptr},
		{"reverse: array",       R"(["reverse", ["array", 1, 2, 3]])",     json::parse("[3,2,1]"),       nullptr},
		{"reverse: string",      R"(["reverse", "abc"])",                   "cba",   nullptr},
		{"reverse: empty array", R"(["reverse", ["array"]])",               json::parse("[]"),            nullptr},
		{"reverse: non-coll",    R"(["reverse", 42])",                      nullptr, nullptr},
		{"reverse: no args",     R"(["reverse"])",                          nullptr, nullptr},
		{"last: basic",          R"(["last", ["array", 1, 2, 3]])",        3,       nullptr},
		{"last: single elem",    R"(["last", ["array", 99]])",             99,      nullptr},
		{"last: empty array",    R"(["last", ["array"]])",                 nullptr, nullptr},
		{"last: non-array",      R"(["last", 42])",                        nullptr, nullptr},
		{"last: no args",        R"(["last"])",                            nullptr, nullptr},
		{"compact: removes nulls", R"(["compact", ["array", 1, null, 2, null, 3]])", json::parse("[1,2,3]"), nullptr},
		{"compact: no nulls",    R"(["compact", ["array", 1, 2, 3]])",    json::parse("[1,2,3]"), nullptr},
		{"compact: all nulls",   R"(["compact", ["array", null, null]])",  json::parse("[]"),     nullptr},
		{"compact: empty array", R"(["compact", ["array"]])",              json::parse("[]"),     nullptr},
		{"compact: non-array",   R"(["compact", 42])",                     nullptr, nullptr},
		{"compact: no args",     R"(["compact"])",                         nullptr, nullptr},
	};
	for (const auto& t : arr_tests) run_test(t);

	// ── Positional Access: first–tenth ──────────────────────────
	printf("\n── Positional Access ──\n");
	vector<Test> pos_tests = {
		{"first: basic",    R"(["first", ["array", "a", "b", "c"]])",   "a",     nullptr},
		{"second: basic",   R"(["second", ["array", "a", "b", "c"]])",  "b",     nullptr},
		{"third: basic",    R"(["third", ["array", "a", "b", "c"]])",   "c",     nullptr},
		{"fourth: basic",   R"(["fourth", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])", 4, nullptr},
		{"fifth: basic",    R"(["fifth", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])",  5, nullptr},
		{"sixth: basic",    R"(["sixth", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])",  6, nullptr},
		{"seventh: basic",  R"(["seventh", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])", 7, nullptr},
		{"eighth: basic",   R"(["eighth", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])", 8, nullptr},
		{"ninth: basic",    R"(["ninth", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])",  9, nullptr},
		{"tenth: basic",    R"(["tenth", ["array", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])",  10, nullptr},
		{"first: empty",    R"(["first", ["array"]])",                   nullptr, nullptr},
		{"first: non-arr",  R"(["first", 42])",                          nullptr, nullptr},
		{"first: no args",  R"(["first"])",                              nullptr, nullptr},
		{"third: too short", R"(["third", ["array", "a"]])",             nullptr, nullptr},
	};
	for (const auto& t : pos_tests) run_test(t);

	// ── Lisp: cons / car / cdr ──────────────────────────────────
	printf("\n── Lisp: cons / car / cdr ──\n");
	vector<Test> lisp_tests = {
		{"cons: prepend",        R"(["cons", 1, ["array", 2, 3]])",      json::parse("[1,2,3]"), nullptr},
		{"cons: to empty",       R"(["cons", 1, ["array"]])",            json::parse("[1]"),     nullptr},
		{"cons: non-array tail", R"(["cons", 1, 2])",                    nullptr,                nullptr},
		{"cons: no args",        R"(["cons"])",                          nullptr,                nullptr},
		{"car: basic",           R"(["car", ["array", 1, 2, 3]])",      1,       nullptr},
		{"car: single elem",     R"(["car", ["array", 99]])",           99,      nullptr},
		{"car: empty array",     R"(["car", ["array"]])",               nullptr, nullptr},
		{"car: non-array",       R"(["car", 5])",                       nullptr, nullptr},
		{"car: no args",         R"(["car"])",                          nullptr, nullptr},
		{"cdr: basic",           R"(["cdr", ["array", 1, 2, 3]])",     json::parse("[2,3]"),   nullptr},
		{"cdr: single elem",     R"(["cdr", ["array", 99]])",          json::parse("[]"),      nullptr},
		{"cdr: empty array",     R"(["cdr", ["array"]])",              json::parse("[]"),      nullptr},
		{"cdr: non-array",       R"(["cdr", 5])",                      json::parse("[]"),      nullptr},
		{"cdr: no args",         R"(["cdr"])",                         nullptr,                nullptr},
		{"car of cons",          R"(["car", ["cons", "x", ["array", "y"]]])", "x", nullptr},
		{"cdr of cons",          R"(["cdr", ["cons", "x", ["array", "y"]]])", json::parse(R"(["y"])"), nullptr},
	};
	for (const auto& t : lisp_tests) run_test(t);

	// ── String Functions ────────────────────────────────────────
	printf("\n── String Functions ──\n");
	vector<Test> str_tests = {
		{"lowercase: basic",       R"(["lowercase", "HELLO"])",       "hello",   nullptr},
		{"lowercase: mixed",       R"(["lowercase", "HeLLo"])",       "hello",   nullptr},
		{"lowercase: already low", R"(["lowercase", "hello"])",       "hello",   nullptr},
		{"lowercase: non-string",  R"(["lowercase", 42])",            nullptr,   nullptr},
		{"lowercase: no args",     R"(["lowercase"])",                nullptr,   nullptr},
		{"uppercase: basic",       R"(["uppercase", "hello"])",       "HELLO",   nullptr},
		{"uppercase: mixed",       R"(["uppercase", "HeLLo"])",       "HELLO",   nullptr},
		{"uppercase: already up",  R"(["uppercase", "HELLO"])",       "HELLO",   nullptr},
		{"uppercase: non-string",  R"(["uppercase", 42])",            nullptr,   nullptr},
		{"uppercase: no args",     R"(["uppercase"])",                nullptr,   nullptr},
		{"trim: leading",          R"(["trim", "  hello"])",          "hello",   nullptr},
		{"trim: trailing",         R"(["trim", "hello  "])",          "hello",   nullptr},
		{"trim: both",             R"(["trim", "  hello  "])",        "hello",   nullptr},
		{"trim: tabs/newlines",    R"(["trim", "\t hello \n"])",      "hello",   nullptr},
		{"trim: no whitespace",    R"(["trim", "hello"])",            "hello",   nullptr},
		{"trim: all whitespace",   R"(["trim", "   "])",              "",        nullptr},
		{"trim: non-string",       R"(["trim", 42])",                 nullptr,   nullptr},
		{"trim: no args",          R"(["trim"])",                     nullptr,   nullptr},
		{"concat: two strings",    R"(["concat", "hello", " world"])", "hello world", nullptr},
		{"concat: multi",          R"(["concat", "a", "b", "c"])",    "abc",     nullptr},
		{"concat: empty",          R"(["concat"])",                    "",        nullptr},
		{"concat: skip non-str",   R"(["concat", "a", 1, "b"])",      "ab",      nullptr},
		{"join: no separator",     R"(["join", ["array", "a", "b", "c"]])", "abc", nullptr},
		{"join: with separator",   R"(["join", ["array", "a", "b", "c"], ", "])", "a, b, c", nullptr},
		{"join: single element",   R"(["join", ["array", "hello"]])", "hello",   nullptr},
		{"join: empty array",      R"(["join", ["array"]])",          "",        nullptr},
		{"join: non-array",        R"(["join", "hello"])",            nullptr,   nullptr},
		{"join: no args",          R"(["join"])",                     nullptr,   nullptr},
		{"split: no delimiter",    R"(["split", "abc"])",             json::parse(R"(["a","b","c"])"), nullptr},
		{"split: with delimiter",  R"(["split", "a,b,c", ","])",     json::parse(R"(["a","b","c"])"), nullptr},
		{"split: multi-char delim", R"(["split", "a--b--c", "--"])", json::parse(R"(["a","b","c"])"), nullptr},
		{"split: no match",        R"(["split", "abc", ","])",       json::parse(R"(["abc"])"),       nullptr},
		{"split: empty string",    R"(["split", "", ","])",          json::parse(R"([""])"),          nullptr},
		{"split: non-string",      R"(["split", 42])",               nullptr,   nullptr},
		{"split: no args",         R"(["split"])",                   nullptr,   nullptr},
	};
	for (const auto& t : str_tests) run_test(t);

	// ── Contains ────────────────────────────────────────────────
	printf("\n── Contains ──\n");
	vector<Test> contains_tests = {
		{"contains: array has elem",     R"(["contains", ["array", 1, 2, 3], 2])",    true,  nullptr},
		{"contains: array missing",      R"(["contains", ["array", 1, 2, 3], 5])",    false, nullptr},
		{"contains: array string elem",  R"(["contains", ["array", "a", "b"], "b"])", true,  nullptr},
		{"contains: array empty",        R"(["contains", ["array"], 1])",              false, nullptr},
		{"contains: string has substr",  R"(["contains", "hello world", "world"])",    true,  nullptr},
		{"contains: string missing",     R"(["contains", "hello world", "xyz"])",      false, nullptr},
		{"contains: string empty needle", R"(["contains", "hello", ""])",              true,  nullptr},
		{"contains: non-coll",           R"(["contains", 42, 4])",                     false, nullptr},
		{"contains: no args",            R"(["contains"])",                             false, nullptr},
	};
	for (const auto& t : contains_tests) run_test(t);

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
		{"cons + car roundtrip",
			R"(["car", ["cons", "head", ["array", "tail"]]])",
			"head", nullptr},
		{"split then join roundtrip",
			R"(["join", ["split", "a,b,c", ","], ","])",
			"a,b,c", nullptr},
		{"and + not combo",
			R"(["and", ["not", false], ["not", null]])",
			true, nullptr},
		{"compact + length",
			R"(["length", ["compact", ["array", 1, null, 2, null]]])",
			2, nullptr},
		{"reverse + first = last",
			R"(["first", ["reverse", ["array", 1, 2, 3]]])",
			3, nullptr},
		{"uppercase + trim",
			R"(["uppercase", ["trim", "  hello  "]])",
			"HELLO", nullptr},
		{"contains + if pattern",
			R"(["if", ["contains", ["array", "a", "b", "c"], "b"], "found", "missing"])",
			"found", nullptr},
	};
	for (const auto& t : integration_tests) run_test(t);

	// ── Trace ───────────────────────────────────────────────────
	printf("\n── Trace ──\n");

	// Helper to run trace tests
	auto run_trace_test = [&](string name, string expr, json expected_trace, json scope_data = nullptr) {
		Scope scope = Scope(scope_data.is_null() ? json::object() : scope_data);
		Trace trace;
		trace.start();
		scope.trace = &trace;

		Node node = parse(expr);
		node.eval(&scope);

		scope.trace = nullptr;
		json result = trace.stop();

		bool ok = (result == expected_trace);
		if (ok) {
			passed++;
			printf("  PASS  %s\n", name.c_str());
		} else {
			failed++;
			printf("  FAIL  %s\n", name.c_str());
			printf("        expected: %s\n", expected_trace.dump().c_str());
			printf("        got:      %s\n", result.dump().c_str());
		}
	};

	run_trace_test("trace: bare value",
		"5", json(5));

	run_trace_test("trace: simple add",
		R"(["+", 1, 2])",
		json::parse("[3.0, 1, 2]"));

	run_trace_test("trace: nested arithmetic",
		R"(["+", 1, ["-", 5, 2]])",
		json::parse("[4.0, 1, [3.0, 5, 2]]"));

	run_trace_test("trace: deep nesting",
		R"(["*", ["+", 1, 2], ["-", 10, 4]])",
		json::parse("[18.0, [3.0, 1, 2], [6.0, 10, 4]]"));

	run_trace_test("trace: if true path",
		R"(["if", true, "yes", "no"])",
		json::parse(R"(["yes", true, "yes"])"));

	run_trace_test("trace: if false path",
		R"(["if", false, "yes", "no"])",
		json::parse(R"(["no", false, null, "no"])"));

	run_trace_test("trace: if nested condition",
		R"(["if", ["=", 1, 1], "yes", "no"])",
		json::parse(R"(["yes", [true, 1, 1], "yes"])"));

	run_trace_test("trace: var lookup",
		R"(["var", "x"])",
		json::parse(R"([5, "x"])"),
		json::parse(R"({"x": 5})"));

	run_trace_test("trace: equality",
		R"(["=", 5, 5])",
		json::parse("[true, 5, 5]"));

	run_trace_test("trace: let basic",
		R"(["let", [["x", 5]], ["var", "x"]])",
		json::parse(R"([5, "x", 5, [5, "x"]])"));

	run_trace_test("trace: let with expression",
		R"(["let", [["x", ["+", 1, 2]]], ["var", "x"]])",
		json::parse(R"([3.0, "x", [3.0, 1, 2], [3.0, "x"]])"));

	run_trace_test("trace: and short-circuits",
		R"(["and", true, false])",
		json::parse("[false, true, false]"));

	// ── Trace: range ────────────────────────────────────────────
	printf("\n── Trace: range ──\n");

	run_trace_test("trace: range basic",
		R"(["range", 1, 3])",
		json::parse("[[1,2,3], 1, 3]"));

	run_trace_test("trace: range single element",
		R"(["range", 5, 5])",
		json::parse("[[5], 5, 5]"));

	run_trace_test("trace: range with expressions",
		R"(["range", ["+", 0, 1], ["-", 5, 2]])",
		json::parse("[[1,2,3], [1.0, 0, 1], [3.0, 5, 2]]"));

	// ── Trace: for-each ─────────────────────────────────────────
	printf("\n── Trace: for-each ──\n");

	run_trace_test("trace: for-each simple body",
		R"(["for-each", "i", ["array", 10, 20], ["get", "i"]])",
		json::parse(R"([20, [[10,20], 10, 20], [10, "i"], [20, "i"]])"),
		json::parse("{}"));

	run_trace_test("trace: for-each with arithmetic body",
		R"(["for-each", "i", ["array", 1, 2, 3], ["+", ["get", "i"], 10]])",
		json::parse(R"([13.0, [[1,2,3], 1, 2, 3], [11.0, [1, "i"], 10], [12.0, [2, "i"], 10], [13.0, [3, "i"], 10]])"),
		json::parse("{}"));

	// ── Trace: for-each + range ─────────────────────────────────
	printf("\n── Trace: for-each + range ──\n");

	run_trace_test("trace: for-each over range",
		R"(["for-each", "i", ["range", 1, 3], ["+", ["get", "i"], 10]])",
		json::parse(R"([13.0, [[1,2,3], 1, 3], [11.0, [1, "i"], 10], [12.0, [2, "i"], 10], [13.0, [3, "i"], 10]])"),
		json::parse("{}"));

	// ── Trace: for-each + set accumulator ───────────────────────
	printf("\n── Trace: for-each + set (accumulator) ──\n");

	run_trace_test("trace: for-each set accumulator",
		R"(["for-each", "i", ["array", 1, 2, 3],
			["set", "total", ["+", ["get", "total"], ["get", "i"]]]])",
		json::parse(R"([6.0,
			[[1,2,3], 1, 2, 3],
			[1.0, "total", [1.0, [0, "total"], [1, "i"]]],
			[3.0, "total", [3.0, [1.0, "total"], [2, "i"]]],
			[6.0, "total", [6.0, [3.0, "total"], [3, "i"]]]
		])"),
		json::parse(R"({"total": 0})"));

	// ── Trace: let + for-each + range (full pattern) ────────────
	printf("\n── Trace: let + for-each + range ──\n");

	run_trace_test("trace: let + for-each + range accumulator",
		R"(["let",
			[["total", 0]],
			["for-each", "i", ["range", 1, 2],
				["set", "total", ["+", ["get", "total"], ["get", "i"]]]]])",
		json::parse(R"([3.0,
			"total", 0,
			[3.0, [[1,2], 1, 2],
				[1.0, "total", [1.0, [0, "total"], [1, "i"]]],
				[3.0, "total", [3.0, [1.0, "total"], [2, "i"]]]
			]
		])"),
		json::parse("{}"));

	// ── Summary ─────────────────────────────────────────────────
	printf("\n────────────────────────────\n");
	printf("  %d passed, %d failed, %d total\n\n", passed, failed, passed + failed);

	return failed > 0 ? 1 : 0;
}
