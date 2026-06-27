[![License:MIT](https://img.shields.io/badge/License-MIT-blue?style=plastic)](LICENSE)
[![C++ CI build](../../actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml)

### caps_cpp

A C++ library for reading and applying Linux capabilities. It wraps a bundled
copy of libcap-ng - so it has no external dependencies - and makes capability
handling simpler than the raw API.

### <sub>Usage</sub>

```cpp
#include "capabilities.hpp"

// read the current process's capabilities
CAPS caps = CAPS::get();
std::cout << caps << "\n";

// build a capability set and apply it to this process
CAPS desired = {
    { CAP::SET::BOUNDING,  { CAP::KILL, CAP::NET_BIND_SERVICE }},
    { CAP::SET::PERMITTED, { CAP::KILL, CAP::NET_BIND_SERVICE }},
    { CAP::SET::EFFECTIVE, { CAP::KILL }},
};
desired.set();

// or drop privileges to a uid/gid while keeping the requested capabilities
desired.set_user(1000, 1000, { /* additional gids */ });
```

Capabilities are named by the `CAP` type (`CAP::KILL`, `CAP::NET_ADMIN`, ...) and
grouped into the five sets (`BOUNDING`, `PERMITTED`, `INHERITABLE`, `EFFECTIVE`,
`AMBIENT`). `CAPS::get(pid)` reads them, `set()` applies them, `set_user()` drops
to a uid/gid while keeping the requested set, and `CAPS::lock()` locks the
securebits. Streaming a `CAPS` prints a readable summary.

### <sub>Dependencies</sub>

None - a copy of libcap-ng is bundled. Requires Linux and a C++17 (or newer)
compiler.

### <sub>Examples</sub>

Sample programs are provided: `show_caps`, `list_caps`, `set_caps` and
`set_user` (run as root).
