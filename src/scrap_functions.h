#pragma once

#include "base.h"

namespace Neon {
namespace Scrap {

struct Node;
using ScrapFunction = json (*)(const vector<Node>&, const json&);

ScrapFunction get_scap_function(const string&);

} // namespace Scrap
} // namespace Neon
