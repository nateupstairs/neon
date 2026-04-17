#include "base.h"
#include "scrap.cpp"

using namespace Neon::Scrap;
using json = nlohmann::json;
using std::string;

string evaluateImpl(string syntax, string scope) {
	Node s = parse(syntax);
	json scoped = json::parse(scope);
	Scope eval_scope = Scope(scoped);

	json result = s.eval(&eval_scope);

	return result.dump();
}

#ifdef NEON_STANDALONE
#include "wasm_standalone_utils.h"

extern "C" {

EMSCRIPTEN_KEEPALIVE
char* evaluate (const char* syntax, const char* scope) {
    return wasm_create_string (evaluateImpl (syntax, scope));
}

}

#else
#include <emscripten/bind.h>

using namespace emscripten;

string evaluate (std::string syntax, std::string scope) {
    return evaluateImpl (syntax, scope);
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("evaluate", &evaluate);
}

#endif
