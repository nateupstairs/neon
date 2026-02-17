#include <emscripten/bind.h>

using namespace emscripten;

#include "base.h"
#include "scrap.cpp"

using namespace Neon::Scrap;
using json = nlohmann::json;
using std::string;

string evaluate(string syntax, string scope) {
	Node s = parse(syntax);
	json scoped = json::parse(scope);
	Scope eval_scope = Scope(scoped);

	json result = s.eval(&eval_scope);

	return result.dump();
}

// Binding code
EMSCRIPTEN_BINDINGS(my_module) {
    function("evaluate", &evaluate);
}
