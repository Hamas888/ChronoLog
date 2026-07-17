#ifndef CHRONOLOG_FACADE_H
#define CHRONOLOG_FACADE_H

// Facade header — forwards to the real implementation in src/.
// Uses a unique guard so the real header's own guard (in src/ChronoLog.h)
// is not blocked when both are in the include path.
#include "../src/ChronoLog.h"

#endif // CHRONOLOG_FACADE_H
