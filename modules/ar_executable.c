#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <dirent.h>
#include "ar_system.h"
#include "ar_method.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include "ar_executable.h"
#include "ar_heap.h"
#include "ar_io.h"

/* Bootstrap agent configuration */
static const char *BOOTSTRAP_METHOD_NAME = "bootstrap";
static const char *BOOTSTRAP_METHOD_VERSION = "1.0.0";

/**
 * Load all method files from the methods directory
 * @param mut_methodology The methodology instance to load methods into
 * @return Number of methods loaded successfully
 */
static int _load_methods_from_directory(ar_methodology_t *mut_methodology) {
    // Allow methods directory to be overridden via environment variable
    const char *methods_dir = getenv("AGERUN_METHODS_DIR");
    if (!methods_dir) {
        methods_dir = "../../methods";
    }
    DIR *dir = opendir(methods_dir);
    if (!dir) {
        printf("Failed to open methods directory: %s\n", methods_dir);
        return 0;
    }
    
    int loaded_count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Check if it's a .method file
        const char *extension = strrchr(entry->d_name, '.');
        if (!extension || strcmp(extension, ".method") != 0) {
            continue;
        }
        
        // Parse the filename to extract name and version
        // Format: name-version.method (e.g., echo-1.0.0.method)
        char method_name[256];
        char version[32];
        
        // Copy filename without extension
        size_t name_len = (size_t)(extension - entry->d_name);
        if (name_len >= sizeof(method_name)) {
            printf("Method filename too long: %s\n", entry->d_name);
            continue;
        }
        strncpy(method_name, entry->d_name, name_len);
        method_name[name_len] = '\0';
        
        // Find the last hyphen to separate name and version
        char *last_hyphen = strrchr(method_name, '-');
        if (!last_hyphen) {
            printf("Invalid method filename format: %s\n", entry->d_name);
            continue;
        }
        
        // Split into name and version
        *last_hyphen = '\0';
        strncpy(version, last_hyphen + 1, sizeof(version) - 1);
        version[sizeof(version) - 1] = '\0';
        
        // Build full path to method file
        char filepath[2048];
        snprintf(filepath, sizeof(filepath), "%s/%s", methods_dir, entry->d_name);
        
        // Read the method file
        FILE *fp = NULL;
        ar_file_result_t result = ar_io__open_file(filepath, "r", &fp);
        if (result != AR_FILE_RESULT__SUCCESS) {
            printf("Failed to open method file %s: %d\n", filepath, result);
            continue;
        }
        
        // Get file size
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        // Allocate buffer for file content
        char *own_content = AR__HEAP__MALLOC((size_t)(file_size + 1), "Method file content");
        if (!own_content) {
            ar_io__close_file(fp, filepath);
            continue;
        }
        
        // Read file content
        size_t bytes_read = fread(own_content, 1, (size_t)file_size, fp);
        own_content[bytes_read] = '\0';
        ar_io__close_file(fp, filepath);
        
        // Create method in methodology
        if (ar_methodology__create_method_with_instance(mut_methodology, method_name, own_content, version)) {
            printf("Loaded method '%s' version '%s' from directory\n", method_name, version);
            loaded_count++;
        } else {
            printf("Failed to create method '%s' version '%s'\n", method_name, version);
        }
        
        AR__HEAP__FREE(own_content);
    }
    
    closedir(dir);
    
    if (loaded_count > 0) {
        printf("Loaded %d methods from directory\n", loaded_count);
    }
    
    return loaded_count;
}

int ar_executable__main(void) {
    printf("Agerun Example Application\n");
    printf("==========================\n\n");
    
    // Create the system instance
    printf("Creating system instance...\n");
    ar_system_t *mut_system = ar_system__create();
    if (!mut_system) {
        printf("Error: Failed to create system instance\n");
        return 1;
    }
    
    // Get references needed for loading methods
    
    // Get agency instance from the system
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        printf("Error: Failed to get agency instance\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get methodology instance from the agency
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology) {
        printf("Error: Failed to get methodology instance\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Always load methods from directory (no persistence files)
    printf("Loading methods from directory...\n");
    int methods_loaded = _load_methods_from_directory(mut_methodology);
    
    if (methods_loaded == 0) {
        // Fall back to creating methods programmatically if directory loading fails
        printf("No methods loaded from directory, creating default methods...\n");
        
        // Create a simple echo method that handles wake messages properly
        printf("Creating echo method...\n");
        const char *ref_echo_instructions = "if(message = \"__wake__\", send(0, \"Echo agent is awake\"), send(0, message))";
        const char *ref_echo_version = "1.0.0";
        
        if (!ar_methodology__create_method_with_instance(mut_methodology, "echo", ref_echo_instructions, ref_echo_version)) {
            printf("Failed to create echo method\n");
            ar_system__shutdown_with_instance(mut_system);
            ar_system__destroy(mut_system);
            return 1;
        }
        
        printf("Echo method created with version %s\n\n", ref_echo_version);
        
        // Create a simplified counter method that just echoes back messages
        printf("Creating counter method...\n");
        const char *ref_counter_code = "send(0, \"Hello from counter!\")";
        const char *ref_counter_version = "1.0.0";
        
        if (!ar_methodology__create_method_with_instance(mut_methodology, "counter", ref_counter_code, ref_counter_version)) {
            printf("Failed to create counter method\n");
            ar_system__shutdown_with_instance(mut_system);
            ar_system__destroy(mut_system);
            return 1;
        }
        
        printf("Counter method created with version %s\n\n", ref_counter_version);
    } else {
        printf("Successfully loaded %d methods from directory\n\n", methods_loaded);
    }
    
    // Initialize the system and create bootstrap agent
    printf("Creating bootstrap agent...\n");
    int64_t initial_agent = ar_system__init_with_instance(mut_system, BOOTSTRAP_METHOD_NAME, BOOTSTRAP_METHOD_VERSION);
    if (initial_agent <= 0) {
        printf("Error: Failed to create bootstrap agent\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    printf("Bootstrap agent created with ID: %" PRId64 "\n", initial_agent);
    
    // Shutdown the runtime
    printf("Shutting down runtime...\n");
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    printf("Runtime shutdown complete\n\n");
    
    return 0;
}

int main(void) {
    return ar_executable__main();
}