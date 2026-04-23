#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>
#include "ar_system.h"
#include "ar_method.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include "ar_executable.h"
#include "ar_heap.h"
#include "ar_io.h"
#include "ar_agent_store.h"

/* Bootstrap agent configuration */
static const char *BOOTSTRAP_METHOD_NAME = "bootstrap";
static const char *BOOTSTRAP_METHOD_VERSION = "1.0.0";
static const char *BOOTSTRAP_METHOD_IDENTIFIER = "bootstrap-1.0.0";

static bool _parse_runtime_options(int argc,
                                   char **argv,
                                   const char **ref_boot_method_identifier,
                                   bool *ref_persistence_disabled);
static bool _split_method_identifier(const char *ref_method_identifier,
                                     char *mut_method_name,
                                     size_t method_name_size,
                                     char *mut_method_version,
                                     size_t method_version_size);
static void _report_persistence_mode(bool ref_persistence_disabled);
static void _report_boot_method_selection(const char *ref_boot_method_identifier);
static int _report_invalid_boot_method_override(const char *ref_boot_method_identifier);
static int _report_boot_agent_creation_failure(const char *ref_boot_method_identifier,
                                               const char *ref_selected_method_name,
                                               const char *ref_selected_method_version);
static void _report_restored_startup_outcome(const char *ref_boot_method_identifier);

/**
 * Load all method files from the methods directory
 * @param mut_methodology The methodology instance to load methods into
 * @return Number of methods loaded successfully
 */
static bool _parse_runtime_options(int argc,
                                   char **argv,
                                   const char **ref_boot_method_identifier,
                                   bool *ref_persistence_disabled) {
    int ref_index;

    if (!ref_boot_method_identifier || !ref_persistence_disabled) {
        return false;
    }

    *ref_boot_method_identifier = NULL;
    *ref_persistence_disabled = false;

    for (ref_index = 1; ref_index < argc; ref_index++) {
        const char *ref_argument = argv[ref_index];
        if (!ref_argument) {
            continue;
        }

        if (strcmp(ref_argument, "--boot-method") == 0) {
            if (ref_index + 1 >= argc || !argv[ref_index + 1] || argv[ref_index + 1][0] == '\0') {
                printf("Error: Missing value for --boot-method\n");
                return false;
            }

            *ref_boot_method_identifier = argv[ref_index + 1];
            ref_index++;
            continue;
        }

        if (strcmp(ref_argument, "--no-persistence") == 0) {
            *ref_persistence_disabled = true;
            continue;
        }

        printf("Error: Unknown argument '%s'\n", ref_argument);
        return false;
    }

    return true;
}

static bool _split_method_identifier(const char *ref_method_identifier,
                                     char *mut_method_name,
                                     size_t method_name_size,
                                     char *mut_method_version,
                                     size_t method_version_size) {
    const char *ref_last_hyphen;
    size_t ref_method_name_length;
    size_t ref_method_version_length;

    if (!ref_method_identifier || !mut_method_name || !mut_method_version) {
        return false;
    }

    ref_last_hyphen = strrchr(ref_method_identifier, '-');
    if (!ref_last_hyphen || ref_last_hyphen == ref_method_identifier || ref_last_hyphen[1] == '\0') {
        return false;
    }

    ref_method_name_length = (size_t)(ref_last_hyphen - ref_method_identifier);
    ref_method_version_length = strlen(ref_last_hyphen + 1);
    if (ref_method_name_length + 1 > method_name_size || ref_method_version_length + 1 > method_version_size) {
        return false;
    }

    strncpy(mut_method_name, ref_method_identifier, ref_method_name_length);
    mut_method_name[ref_method_name_length] = '\0';
    strncpy(mut_method_version, ref_last_hyphen + 1, method_version_size - 1);
    mut_method_version[method_version_size - 1] = '\0';
    return true;
}

static void _report_persistence_mode(bool ref_persistence_disabled) {
    if (ref_persistence_disabled) {
        printf("Persistence disabled for this run: skipping persisted methodology and agency load/save\n");
    }
}

static void _report_boot_method_selection(const char *ref_boot_method_identifier) {
    if (ref_boot_method_identifier) {
        printf("Boot method override requested: '%s'\n", ref_boot_method_identifier);
    } else {
        printf("No boot override requested; using default boot method '%s'\n",
               BOOTSTRAP_METHOD_IDENTIFIER);
    }
}

static int _report_invalid_boot_method_override(const char *ref_boot_method_identifier) {
    printf("Error: Invalid boot method override '%s'\n", ref_boot_method_identifier);
    return 1;
}

static int _report_boot_agent_creation_failure(const char *ref_boot_method_identifier,
                                               const char *ref_selected_method_name,
                                               const char *ref_selected_method_version) {
    if (ref_boot_method_identifier) {
        printf("Error: Failed to create boot agent from method '%s' version '%s'\n",
               ref_selected_method_name,
               ref_selected_method_version);
    } else {
        printf("Error: Failed to create bootstrap agent\n");
    }

    return 1;
}

static void _report_restored_startup_outcome(const char *ref_boot_method_identifier) {
    if (ref_boot_method_identifier) {
        printf("Boot method override '%s' skipped because agents were restored from disk\n",
               ref_boot_method_identifier);
    }

    printf("Agents loaded from disk, skipping bootstrap creation\n");
}

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
        if (ar_methodology__create_method(mut_methodology, method_name, own_content, version)) {
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

int ar_executable__main_with_args(int argc, char **argv) {
    const char *ref_boot_method_identifier = NULL;
    const char *ref_selected_method_name = BOOTSTRAP_METHOD_NAME;
    const char *ref_selected_method_version = BOOTSTRAP_METHOD_VERSION;
    bool ref_persistence_disabled = false;
    char mut_override_method_name[256];
    char mut_override_method_version[32];

    printf("Agerun Example Application\n");
    printf("==========================\n\n");

    if (!_parse_runtime_options(argc,
                                argv,
                                &ref_boot_method_identifier,
                                &ref_persistence_disabled)) {
        return 1;
    }

    _report_persistence_mode(ref_persistence_disabled);
    _report_boot_method_selection(ref_boot_method_identifier);
    if (ref_boot_method_identifier) {
        if (!_split_method_identifier(ref_boot_method_identifier,
                                      mut_override_method_name,
                                      sizeof(mut_override_method_name),
                                      mut_override_method_version,
                                      sizeof(mut_override_method_version))) {
            return _report_invalid_boot_method_override(ref_boot_method_identifier);
        }

        ref_selected_method_name = mut_override_method_name;
        ref_selected_method_version = mut_override_method_version;
    }
    
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
        ar_system__shutdown(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get methodology instance from the agency
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology) {
        printf("Error: Failed to get methodology instance\n");
        ar_system__shutdown(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Check if persisted methodology file exists
    int methods_loaded = 0;
    bool loaded_from_file = false;
    struct stat st;
    if (!ref_persistence_disabled && stat(METHODOLOGY_FILE_NAME, &st) == 0) {
        // Load from persisted file
        printf("Loading methods from persisted methodology...\n");
        if (ar_methodology__load_methods(mut_methodology, METHODOLOGY_FILE_NAME)) {
            printf("Successfully loaded methods from %s\n", METHODOLOGY_FILE_NAME);
            loaded_from_file = true;
            // We don't know the exact count when loading from file, but that's OK
        } else {
            printf("Failed to load from %s, will try directory\n", METHODOLOGY_FILE_NAME);
        }
    }
    
    // If no persisted file or loading failed, load from directory
    if (!loaded_from_file) {
        printf("Loading methods from directory...\n");
        methods_loaded = _load_methods_from_directory(mut_methodology);
    }
    
    if (!loaded_from_file && methods_loaded == 0) {
        // Fall back to creating methods programmatically if both file and directory loading fail
        printf("No methods loaded from file or directory, creating default methods...\n");
        
        // Create a simple echo method
        printf("Creating echo method...\n");
        const char *ref_echo_instructions = "send(0, message)";
        const char *ref_echo_version = "1.0.0";
        
        if (!ar_methodology__create_method(mut_methodology, "echo", ref_echo_instructions, ref_echo_version)) {
            printf("Failed to create echo method\n");
            ar_system__shutdown(mut_system);
            ar_system__destroy(mut_system);
            return 1;
        }
        
        printf("Echo method created with version %s\n\n", ref_echo_version);
        
        // Create a simplified counter method that just echoes back messages
        printf("Creating counter method...\n");
        const char *ref_counter_code = "send(0, \"Hello from counter!\")";
        const char *ref_counter_version = "1.0.0";
        
        if (!ar_methodology__create_method(mut_methodology, "counter", ref_counter_code, ref_counter_version)) {
            printf("Failed to create counter method\n");
            ar_system__shutdown(mut_system);
            ar_system__destroy(mut_system);
            return 1;
        }
        
        printf("Counter method created with version %s\n\n", ref_counter_version);
    } else if (!loaded_from_file) {
        printf("Successfully loaded %d methods from directory\n\n", methods_loaded);
    }
    // If loaded_from_file is true, we already printed the success message above

    // Check if persisted agency file exists and load agents
    struct stat agency_stat;
    if (!ref_persistence_disabled && stat(AGENT_STORE_FILE_NAME, &agency_stat) == 0) {
        printf("Loading agents from persisted agency...\n");
        if (ar_agency__load_agents(mut_agency, AGENT_STORE_FILE_NAME)) {
            printf("Successfully loaded agents from %s\n", AGENT_STORE_FILE_NAME);
        } else {
            printf("Warning: Failed to load agents from %s\n", AGENT_STORE_FILE_NAME);
        }
    }

    // Only create bootstrap if no agents were loaded
    if (ar_agency__count_agents(mut_agency) == 0) {
        ar_data_t *own_boot_message;
        bool boot_queued;
        int64_t initial_agent;

        if (ref_boot_method_identifier) {
            printf("Creating boot agent from method '%s' version '%s'...\n",
                   ref_selected_method_name,
                   ref_selected_method_version);
        } else {
            printf("Creating bootstrap agent...\n");
        }
        initial_agent = ar_system__init(mut_system, ref_selected_method_name, ref_selected_method_version);
        if (initial_agent <= 0) {
            int exit_code = _report_boot_agent_creation_failure(ref_boot_method_identifier,
                                                                ref_selected_method_name,
                                                                ref_selected_method_version);
            ar_system__shutdown(mut_system);
            ar_system__destroy(mut_system);
            return exit_code;
        }
        if (ref_boot_method_identifier) {
            printf("Boot agent created with ID: %" PRId64 "\n", initial_agent);
        } else {
            printf("Bootstrap agent created with ID: %" PRId64 "\n", initial_agent);
        }

        own_boot_message = ar_data__create_string("__boot__");
        if (!own_boot_message) {
            printf("Error: Failed to create bootstrap boot message\n");
            ar_system__shutdown(mut_system);
            ar_system__destroy(mut_system);
            return 1;
        }

        boot_queued = ar_agency__send_to_agent(mut_agency, initial_agent, own_boot_message);
        if (!boot_queued) {
            ar_data__destroy(own_boot_message);
            printf("Error: Failed to queue bootstrap boot message\n");
            ar_system__shutdown(mut_system);
            ar_system__destroy(mut_system);
            return 1;
        }
    } else {
        _report_restored_startup_outcome(ref_boot_method_identifier);
    }
    
    // Process all messages until none remain
    printf("Processing messages...\n");
    int messages_processed = ar_system__process_all_messages(mut_system);
    if (messages_processed > 0) {
        printf("Processed %d message%s\n", messages_processed, 
               messages_processed == 1 ? "" : "s");
    } else {
        printf("No messages to process\n");
    }
    
    // Save methodology to file after processing
    if (ref_persistence_disabled) {
        printf("Skipping methodology save because persistence is disabled\n");
    } else {
        printf("Saving methodology to file...\n");
        if (ar_methodology__save_methods(mut_methodology, METHODOLOGY_FILE_NAME)) {
            printf("Methodology saved to %s\n", METHODOLOGY_FILE_NAME);
        } else {
            printf("Warning: Failed to save methodology to %s\n", METHODOLOGY_FILE_NAME);
        }
    }

    // Save agents to file before shutdown
    if (ref_persistence_disabled) {
        printf("Skipping agents save because persistence is disabled\n");
    } else {
        printf("Saving agents to file...\n");
        if (ar_agency__save_agents(mut_agency, AGENT_STORE_FILE_NAME)) {
            printf("Agents saved to %s\n", AGENT_STORE_FILE_NAME);
        } else {
            printf("Warning: Failed to save agents to %s\n", AGENT_STORE_FILE_NAME);
        }
    }

    // Shutdown the runtime
    printf("Shutting down runtime...\n");
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    printf("Runtime shutdown complete\n\n");
    
    return 0;
}

int ar_executable__main(void) {
    char mut_default_program_name[] = "agerun";
    char *mut_default_argv[] = { mut_default_program_name, NULL };
    return ar_executable__main_with_args(1, mut_default_argv);
}

int main(int argc, char **argv) {
    return ar_executable__main_with_args(argc, argv);
}