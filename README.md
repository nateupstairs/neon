# Scrap

A JSON-based expression language. Expressions are written as JSON arrays in prefix notation. Values (strings, numbers, booleans, null) are literals. Arrays are interpreted as function calls where the first element is the function name.

```json
["+", 1, 2]
["if", [">", ["var", "x"], 10], "big", "small"]
```

## Expressions

| Form | Description |
|------|-------------|
| `5` | Literal number |
| `"hello"` | Literal string |
| `true` / `false` | Literal boolean |
| `null` | Null value |
| `["fn", ...args]` | Function call |

Unknown function names evaluate to `null`.

## Scope

Scope is a stack of JSON object frames. Variable lookup reads from the top frame. `let` and `for-each` push/pop frames for isolation.

```json
{"x": 10, "nested": {"a": 1}}
```

## Functions

### Variables

| Function | Example | Description |
|----------|---------|-------------|
| `get` / `var` | `["var", "x"]` | Read variable from scope |
| `get` (deep) | `["get", "obj", "key", 0, "nested"]` | Deep access into objects/arrays |
| `set` | `["set", "x", 5]` | Set variable in current frame |

`get` accepts a chain of string keys (object access) and integer keys (array index).

### Arithmetic

| Function | Example | Result |
|----------|---------|--------|
| `+` | `["+", 1, 2, 3]` | `6` |
| `-` | `["-", 10, 3]` | `7` |
| `*` | `["*", 4, 5]` | `20` |
| `/` | `["/", 10, 2]` | `5` |
| `mod` | `["mod", 10, 3]` | `1` |
| `abs` | `["abs", -5]` | `5` |
| `round` | `["round", 1.6]` | `2` |
| `ceil` | `["ceil", 1.1]` | `2` |
| `floor` | `["floor", 1.9]` | `1` |
| `min` | `["min", 3, 1, 2]` | `1` |
| `max` | `["max", 3, 1, 2]` | `3` |

Arithmetic operators accept variadic arguments. Non-number arguments are skipped.

### Comparison

| Function | Example | Result |
|----------|---------|--------|
| `=` | `["=", 1, 1]` | `true` |
| `!=` | `["!=", 1, 2]` | `true` |
| `>` | `[">", 5, 3]` | `true` |
| `<` | `["<", 3, 5]` | `true` |
| `>=` | `[">=", 5, 5]` | `true` |
| `<=` | `["<=", 3, 5]` | `true` |

`=` accepts variadic arguments (all must be equal). Compares by value for primitives. Mixed number types (int/float) are compared numerically.

### Logic

| Function | Example | Result |
|----------|---------|--------|
| `and` | `["and", true, true]` | `true` |
| `or` | `["or", false, true]` | `true` |
| `not` | `["not", false]` | `true` |

`null` and `false` are falsy. Numbers, non-empty strings, objects, and `true` are truthy.

### Control Flow

#### if

```json
["if", condition, true_branch, false_branch]
```

Condition must evaluate to a boolean. The false branch is optional.

#### let

```json
["let",
  [["x", 5], ["y", ["+", "x", 1]]],
  body
]
```

Bindings are evaluated sequentially (later bindings can reference earlier ones). The scope frame is popped after the body evaluates. Returns the body result.

#### for-each

```json
["for-each", "i", list_expression, body]
```

Iterates over an array, binding each element to the named variable. Returns the result of the last iteration. Pushes/pops a scope frame.

#### range

```json
["range", start, end]
```

Returns an array of integers from `start` to `end` (inclusive).

### Arrays

| Function | Example | Result |
|----------|---------|--------|
| `array` / `list` | `["array", 1, 2, 3]` | `[1, 2, 3]` |
| `nth` | `["nth", arr, 1]` | element at index |
| `first` .. `tenth` | `["first", arr]` | positional access |
| `last` | `["last", arr]` | last element |
| `length` | `["length", arr]` | size (works on strings too) |
| `append` | `["append", arr1, arr2]` | concatenate arrays |
| `reverse` | `["reverse", arr]` | reversed copy |
| `compact` | `["compact", arr]` | remove nulls |
| `contains` | `["contains", arr, val]` | membership test |
| `cons` | `["cons", 1, [2, 3]]` | prepend element |
| `car` | `["car", arr]` | first element |
| `cdr` | `["cdr", arr]` | all but first |

### Strings

| Function | Example | Result |
|----------|---------|--------|
| `concat` | `["concat", "a", "b"]` | `"ab"` |
| `join` | `["join", arr, ", "]` | `"a, b, c"` |
| `split` | `["split", "a,b", ","]` | `["a", "b"]` |
| `uppercase` | `["uppercase", "hi"]` | `"HI"` |
| `lowercase` | `["lowercase", "HI"]` | `"hi"` |
| `trim` | `["trim", " hi "]` | `"hi"` |
| `contains` | `["contains", "hello", "ell"]` | `true` |

### Type

| Function | Example | Result |
|----------|---------|--------|
| `type` | `["type", 5]` | `"number"` |
| `to-string` | `["to-string", 42]` | `"42"` |
| `to-number` | `["to-number", "3.14"]` | `3.14` |
| `default` | `["default", null, "fb"]` | `"fb"` |
| `exists` | `["exists", ["var", "x"]]` | `true` / `false` |

`type` returns: `"number"`, `"string"`, `"boolean"`, `"null"`, `"array"`, `"object"`.

`exists` returns `false` for null, empty strings, empty arrays, and `false`. Returns `true` otherwise.

### Debug

| Function | Example | Description |
|----------|---------|-------------|
| `print` | `["print", expr]` | Prints type and value to stdout |
| `print` | `["print", expr, "label"]` | Prints with label prefix |

`print` returns its evaluated first argument (passthrough).

## Trace (Debugger)

The trace system records a tree of every evaluation. Attach a `Trace` object to the scope, call `start()` before evaluation and `stop()` after to retrieve the trace tree.

### Trace format

Every evaluated node produces a trace entry:

- **Value nodes** produce their literal value: `5`, `"hello"`, `true`, `null`
- **Command nodes** produce an array: `[result, child_1, child_2, ...]`

The first element is always the command's return value. Subsequent elements are the traces of each sub-expression that was evaluated during execution.

### Examples

```
Expression: ["+", 1, 2]
Trace:      [3.0, 1, 2]
             ^    ^  ^
             |    |  └─ trace of arg 2 (value)
             |    └──── trace of arg 1 (value)
             └───────── result of +
```

```
Expression: ["+", 1, ["-", 5, 2]]
Trace:      [4.0, 1, [3.0, 5, 2]]
                      ^
                      └─ nested command trace
```

```
Expression: ["var", "x"]       (scope: {"x": 5})
Trace:      [5, "x"]
             ^   ^
             |   └─ key that was accessed
             └───── resolved value
```

```
Expression: ["var", "obj", "nested", "key"]
Trace:      [value, "obj", "nested", "key"]
                    ^
                    └─ full access path is visible
```

```
Expression: ["set", "total", ["+", 1, 2]]
Trace:      [3.0, "total", [3.0, 1, 2]]
                   ^        ^
                   |        └─ trace of value expression
                   └─ key that was written
```

```
Expression: ["if", true, "yes", "no"]
Trace:      ["yes", true, "yes"]
                          ^
                          └─ only the taken branch appears
```

```
Expression: ["if", false, "yes", "no"]
Trace:      ["no", false, null, "no"]
                          ^
                          └─ null placeholder for skipped branch
```

```
Expression: ["range", 1, 3]
Trace:      [[1,2,3], 1, 3]
```

```
Expression: ["for-each", "i", ["range", 1, 3], ["+", ["get", "i"], 10]]
Trace:      [13.0, [[1,2,3], 1, 3], [11.0, [1,"i"], 10], [12.0, [2,"i"], 10], [13.0, [3,"i"], 10]]
                    ^                ^                     ^                     ^
                    |                |                     |                     └─ iteration 3
                    |                |                     └─ iteration 2
                    |                └─ iteration 1 (get "i" → 1, + 10 → 11)
                    └─ list expression trace
```

### Trace structure rules

1. The trace tree mirrors the evaluation tree, not the syntax tree.
2. `get`/`var` include the access path as children: `[resolved_value, key1, key2, ...]`
3. `set` includes the key and value trace: `[result, key, value_trace]`
4. `for-each` produces one child for the list evaluation, then one child per iteration of the body. The binding variable name does not appear directly (it's read without eval), but is visible in `get` traces within the body.
5. `let` produces children for each binding (key eval, value eval in pairs), then the body trace.
6. `if` inserts a `null` placeholder for skipped branches so the position of the taken branch is stable.
7. The top-level trace is the single root entry (value or command array).

### Accumulator pattern

A common pattern combining `let`, `for-each`, `range`, and `set`:

```json
["let",
  [["total", 0]],
  ["for-each", "i", ["range", 1, 3],
    ["set", "total",
      ["+", ["get", "total"], ["get", "i"]]]]]
```

Result: `6` (sum of 1 + 2 + 3)

The trace for this shows the accumulation across iterations:

```json
[6.0,
  "total", 0,
  [6.0,
    [[1,2,3], 1, 3],
    [1.0, "total", [1.0, [0, "total"], [1, "i"]]],
    [3.0, "total", [3.0, [1.0, "total"], [2, "i"]]],
    [6.0, "total", [6.0, [3.0, "total"], [3, "i"]]]
  ]
]
```

Each iteration's `set` trace shows `[new_value, "key", [add_trace, [prev_total, "total"], [i_value, "i"]]]`. Variable names are preserved at every access point, making it possible to reconstruct the full execution history and see exactly which variables contributed to each result.
