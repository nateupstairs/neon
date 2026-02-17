#pragma once

#include "base.h"

namespace Neon {
namespace Scrap {

struct Node;
struct Scope;
using ScrapFunction = json (*)(const vector<Node>&, Scope*);

ScrapFunction get_scap_function(const string&);

} // namespace Scrap
} // namespace Neon
