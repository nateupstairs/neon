#include <emscripten/bind.h>

using namespace emscripten;

#include "base.h"
#include "scap.cpp"

namespace Neon {

string evaluate(string syntax, string scope) {
	ScapNode s = parse(syntax);
	json scoped = json::parse(scope);

	json result = s.eval(scoped);

	return result.dump();
}

// Binding code
EMSCRIPTEN_BINDINGS(my_module) {
    function("evaluate", &evaluate);
}

} // namespace Neon
