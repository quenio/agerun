# Debug Program Recompilation Pattern

## Learning
When creating standalone debug programs that link against project libraries, they must be recompiled after any library changes. The compiled binary contains the old library code and won't reflect updates unless explicitly rebuilt.

## Importance
Forgetting to recompile debug programs leads to testing outdated code, causing confusion when fixes don't appear to work. This wastes significant debugging time and can lead to incorrect conclusions about the nature of bugs.

## Example
```c
// test_debug.c - Standalone debug program
#include "modules/ar_yaml.h"
#include "modules/ar_data.h"

int main() {
    ar_data_t *data = ar_data__create_map();
    ar_yaml__write_to_file(data, "debug.yaml");
    
    // Test the parsing
    ar_data_t *loaded = ar_yaml__read_from_file("debug.yaml");
    // ... debug checks ...
    
    ar_data__destroy(data);
    ar_data__destroy(loaded);
    return 0;
}
```

After modifying ar_yaml.c:
```bash
# WRONG: Running old binary
./test_debug  # Still uses old library code!

# RIGHT: Recompile first
gcc -I./modules -L./bin/run-tests -o test_debug test_debug.c -lagerun -lm
./test_debug  # Now uses updated library code
```

## Generalization
Debug program workflow:
1. Create debug program linking to library
2. Make library changes
3. **ALWAYS recompile debug program** before running
4. Run recompiled program to test changes
5. Repeat steps 2-4 for each iteration

## Implementation
```bash
# Create a debug script that always recompiles
#!/bin/bash
# debug_run.sh
gcc -I./modules -L./bin/run-tests -o "$1" "$1.c" -lagerun -lm && ./"$1"

# Usage: ./debug_run.sh test_debug
```

Or use make for consistency:
```makefile
# Add to Makefile
debug_%: %.c
	gcc -I./modules -L./bin/run-tests -o $@ $< -lagerun -lm
	./$@
```

## Related Patterns
- [Make-Only Test Execution](make-only-test-execution.md)
- [Test Build Before Run Pattern](test-build-before-run-pattern.md)
- [Integration Test Binary Execution](integration-test-binary-execution.md)