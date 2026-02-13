#pragma once

#include "base.h"

namespace Neon {

struct ScapNode;
using ScapFunction = json (*)(const vector<ScapNode>&, const json&);

ScapFunction get_scap_function(const string&);

} // namespace Neon
