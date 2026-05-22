#pragma once

#include "fmt.h"

#include "base.h"

namespace Neon {

// Custom parameter order for JSON objects
vector<string> get_parameter_order() {
  return {
      "key",
      "display",
      "type",
      "default",
      "global",
      "required",
      "private",
      "//",
      "enabled",
      "start",
      "duration",
      "stagger",
      "mixins",
      "layout",
      "rect",
      "min-width",
      "min-height",
      "max-width",
      "max-height",
      "align",
      "padding",
      "handle",
      "mute",
      "file",
      "hold",
      "font",
      "string",
      "case",
      "gap",
      "height",
      "leading",
      "kerning",
      "measure",
      "justify",
      "boundary",
      "color",
      "substring",
      "background-color",
      "background-padding",
      "background-radius",
      "stroke",
      "stroke-color",
      "stroke-align",
      "svg-recolor",
      "opacity",
      "mask",
      "transform",
      "animations",
      "items",
      "breakpoints",
      "script",
  };
}

// Format a JSON string value with proper escaping
string format_string(const json &j) { return j.dump(); }

// Format a float, trimming trailing zeros
string format_float(f64 val) {
  string s = std::to_string(val);
  // Trim trailing zeros after decimal point
  size_t dot = s.find('.');
  if (dot != string::npos) {
    size_t last = s.find_last_not_of('0');
    if (last == dot) {
      s = s.substr(0, dot + 2); // keep at least one decimal
    } else {
      s = s.substr(0, last + 1);
    }
  }
  return s;
}

// Check if all elements in an array are primitives (no nested arrays/objects)
bool all_primitives(const json &j) {
  for (const auto &el : j) {
    if (el.is_object() || el.is_array())
      return false;
  }
  return true;
}

// Format a primitive value inline
string format_primitive(const json &j) {
  if (j.is_null())
    return "null";
  if (j.is_boolean())
    return j.get<bool>() ? "true" : "false";
  if (j.is_number_integer())
    return std::to_string(j.get<i64>());
  if (j.is_number_float())
    return format_float(j.get<f64>());
  if (j.is_string())
    return format_string(j);
  return j.dump();
}

// Format a small primitive-only array inline: [1, 2, 3]
string format_inline_array(const json &j) {
  string result = "[";
  for (size_t i = 0; i < j.size(); i++) {
    result += format_primitive(j[i]);
    if (i < j.size() - 1)
      result += ", ";
  }
  result += "]";
  return result;
}

// Format an array with one element per line (for rect, padding, etc.)
string format_expanded_array(const json& j, i32 indent) {
  string indent_str(indent, '\t');
  string result = "[\n";
  for (size_t i = 0; i < j.size(); i++) {
    result += indent_str + "\t" + format_json(j[i], indent + 1);
    if (i < j.size() - 1) result += ",";
    result += "\n";
  }
  result += indent_str + "]";
  return result;
}

// Custom JSON formatter for display
string format_json(const json &j, i32 indent = 0) {
  string result;
  string indent_str(indent, '\t');
  static const vector<string> param_order = get_parameter_order();

  if (j.is_null()) {
    result = "null";
  } else if (j.is_boolean()) {
    result = j.get<bool>() ? "true" : "false";
  } else if (j.is_number_integer()) {
    result = std::to_string(j.get<i64>());
  } else if (j.is_number_float()) {
    result = format_float(j.get<f64>());
  } else if (j.is_string()) {
    result = format_string(j);
  } else if (j.is_array()) {
    // Check if this is a solver array: ["$", ...] or ["$:", ...]
    bool is_solver = j.size() >= 2 && j[0].is_string() &&
                    (j[0].get<string>() == "$" || j[0].get<string>() == "$:");

    // Check if this is a script solver: ["$$", ...]
    bool is_script = !is_solver && j.size() >= 2 && j[0].is_string() &&
                    j[0].get<string>() == "$$";

    // Check if this is a node tuple: ["string", {}, optional_array]
    bool is_node = !is_solver && !is_script && j.size() >= 2 && j.size() <= 3 &&
                  j[0].is_string() && j[1].is_object();

    // Check if this is a top-level component: ["name", [mixins], [children]]
    // Only at the root level (indent == 0)
    bool is_component = !is_solver && !is_script && !is_node && indent == 0 &&
                       j.size() == 3 && j[0].is_string() && j[1].is_array() &&
                       j[2].is_array();

    // Check if this is a function-call array: ["name", args...]
    bool is_func_call = !is_solver && !is_script && !is_node && !is_component &&
                      j.size() >= 2 && j[0].is_string();

    if (is_solver) {
      // Solver expressions are always inline: ["$", ["var", "color-2"]]
      string solver_name = j[0].get<string>();
      result = "[\"" + solver_name + "\"";
      for (size_t i = 1; i < j.size(); i++) {
        result += ", " + format_json(j[i], indent);
      }
      result += "]";
    } else if (is_script) {
      // Script solver: ["$$", ...] always multi-line
      result = "[\"$$\",\n";
      for (size_t i = 1; i < j.size(); i++) {
        result += indent_str + "\t" + format_json(j[i], indent + 1);
        if (i < j.size() - 1)
          result += ",";
        result += "\n";
      }
      result += indent_str + "]";
    } else if (is_node) {
      // Node tuple: ["name", {params}, [children]]
      string node_name = j[0].get<string>();

      result = "[\"" + node_name + "\", {";

      const json &params = j[1];
      if (!params.empty()) {
        result += "\n";

        // Collect and order parameter keys
        vector<string> ordered_keys;
        std::set<string> remaining_keys;
        for (auto &[key, value] : params.items()) {
          remaining_keys.insert(key);
        }

        for (const string &key : param_order) {
          if (remaining_keys.count(key)) {
            ordered_keys.push_back(key);
            remaining_keys.erase(key);
          }
        }

        for (const string &key : remaining_keys) {
          ordered_keys.push_back(key);
        }

        size_t count = 0;
        for (const string &key : ordered_keys) {
          const json &value = params[key];
          result += indent_str + "\t\"" + key + "\": ";
          if ((key == "rect" || key == "padding") && value.is_array()) {
            result += format_expanded_array(value, indent + 1);
          } else {
            result += format_json(value, indent + 1);
          }
          if (++count < params.size())
            result += ",";
          result += "\n";
        }
        result += indent_str;
      }

      if (j.size() == 3 && j[2].is_array() && !j[2].empty()) {
        result += "}, [\n";

        const json &children = j[2];
        for (size_t i = 0; i < children.size(); i++) {
          result += indent_str + "\t" + format_json(children[i], indent + 1);
          if (i < children.size() - 1)
            result += ",";
          result += "\n";
        }

        result += indent_str + "]]";
      } else {
        result += "}]";
      }
    } else if (is_component) {
      // Top-level component: ["name", [header], [children]]
      string comp_name = j[0].get<string>();
      const json &header = j[1];
      const json &children = j[2];

      result = "[\n";
      result += indent_str + "\t" + format_string(json(comp_name)) + ",\n";

      // Format header array
      result += indent_str + "\t[\n";
      for (size_t i = 0; i < header.size(); i++) {
        result += indent_str + "\t\t" + format_json(header[i], indent + 2);
        if (i < header.size() - 1)
          result += ",";
        result += "\n";
      }
      result += indent_str + "\t],\n";

      // Format children array
      result += indent_str + "\t[\n";
      for (size_t i = 0; i < children.size(); i++) {
        result += indent_str + "\t\t" + format_json(children[i], indent + 2);
        if (i < children.size() - 1)
          result += ",";
        result += "\n";
      }
      result += indent_str + "\t]\n";

      result += indent_str + "]";
    } else if (is_func_call) {
      // Function-call array: ["name", args...]
      string func_name = j[0].get<string>();

      if (j.size() == 2) {
        // 2-item: keep inline ["name", arg]
        result = "[\"" + func_name + "\", " + format_json(j[1], indent) + "]";
      } else {
        // 3+ items: break after command
        result = "[\"" + func_name + "\",\n";
        for (size_t i = 1; i < j.size(); i++) {
          result += indent_str + "\t" + format_json(j[i], indent + 1);
          if (i < j.size() - 1)
            result += ",";
          result += "\n";
        }
        result += indent_str + "]";
      }
    } else if (j.empty()) {
      result = "[]";
    } else if (all_primitives(j)) {
      // Inline primitive-only arrays: [0, 0], [1, 2, 3]
      result = format_inline_array(j);
    } else {
      // Standard array formatting (contains complex elements)
      result = "[\n";
      for (size_t i = 0; i < j.size(); i++) {
        result += indent_str + "\t" + format_json(j[i], indent + 1);
        if (i < j.size() - 1)
          result += ",";
        result += "\n";
      }
      result += indent_str + "]";
    }
  } else if (j.is_object()) {
    if (j.empty()) {
      result = "{}";
    } else {
      result = "{\n";

      vector<string> ordered_keys;
      std::set<string> remaining_keys;
      for (auto &[key, value] : j.items()) {
        remaining_keys.insert(key);
      }

      for (const string &key : param_order) {
        if (remaining_keys.count(key)) {
          ordered_keys.push_back(key);
          remaining_keys.erase(key);
        }
      }

      for (const string &key : remaining_keys) {
        ordered_keys.push_back(key);
      }

      size_t count = 0;
      for (const string &key : ordered_keys) {
        const json &value = j[key];
        result += indent_str + "\t\"" + key + "\": ";
        if ((key == "rect" || key == "padding") && value.is_array()) {
          result += format_expanded_array(value, indent + 1);
        } else {
          result += format_json(value, indent + 1);
        }
        if (++count < j.size())
          result += ",";
        result += "\n";
      }
      result += indent_str + "}";
    }
  }

  return result;
}

} // namespace Neon
