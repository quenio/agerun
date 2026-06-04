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
static bool _parse_method_filename(const char *ref_filename,
                                   char *mut_method_name,
                                   size_t method_name_size,
                                   char *mut_method_version,
                                   size_t method_version_size);
static bool _load_method_file(ar_methodology_t *mut_methodology,
                              const char *ref_methods_dir,
                              const char *ref_filename,
                              const char *ref_methodology_name);
static int _load_methods_from_directory_path(ar_methodology_t *mut_methodology,
                                             const char *ref_methods_dir,
                                             const char *ref_methodology_name);
static int _load_methods_from_methodologies_directory(ar_methodology_t *mut_methodology);

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

static bool _parse_method_filename(const char *ref_filename,
                                   char *mut_method_name,
                                   size_t method_name_size,
                                   char *mut_method_version,
                                   size_t method_version_size) {
    const char *ref_extension;
    char *mut_last_hyphen;
    size_t ref_name_length;

    if (!ref_filename || !mut_method_name || !mut_method_version) {
        return false;
    }

    ref_extension = strrchr(ref_filename, '.');
    if (!ref_extension || strcmp(ref_extension, ".method") != 0) {
        return false;
    }

    ref_name_length = (size_t)(ref_extension - ref_filename);
    if (ref_name_length >= method_name_size) {
        printf("Method filename too long: %s\n", ref_filename);
        return false;
    }

    strncpy(mut_method_name, ref_filename, ref_name_length);
    mut_method_name[ref_name_length] = '\0';

    mut_last_hyphen = strrchr(mut_method_name, '-');
    if (!mut_last_hyphen) {
        printf("Invalid method filename format: %s\n", ref_filename);
        return false;
    }

    *mut_last_hyphen = '\0';
    strncpy(mut_method_version, mut_last_hyphen + 1, method_version_size - 1);
    mut_method_version[method_version_size - 1] = '\0';
    return true;
}

static bool _load_method_file(ar_methodology_t *mut_methodology,
                              const char *ref_methods_dir,
                              const char *ref_filename,
                              const char *ref_methodology_name) {
    char mut_method_name[256];
    char mut_version[32];
    char mut_filepath[2048];
    FILE *mut_fp = NULL;
    ar_file_result_t result;
    long file_size;
    char *own_content;
    size_t bytes_read;

    if (!_parse_method_filename(ref_filename,
                                mut_method_name,
                                sizeof(mut_method_name),
                                mut_version,
                                sizeof(mut_version))) {
        return false;
    }

    snprintf(mut_filepath, sizeof(mut_filepath), "%s/%s", ref_methods_dir, ref_filename);

    result = ar_io__open_file(mut_filepath, "r", &mut_fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        printf("Failed to open method file %s: %d\n", mut_filepath, result);
        return false;
    }

    fseek(mut_fp, 0, SEEK_END);
    file_size = ftell(mut_fp);
    fseek(mut_fp, 0, SEEK_SET);

    own_content = AR__HEAP__MALLOC((size_t)(file_size + 1), "Method file content");
    if (!own_content) {
        ar_io__close_file(mut_fp, mut_filepath);
        return false;
    }

    bytes_read = fread(own_content, 1, (size_t)file_size, mut_fp);
    own_content[bytes_read] = '\0';
    ar_io__close_file(mut_fp, mut_filepath);

    if (ar_methodology__create_method(mut_methodology,
                                      mut_method_name,
                                      own_content,
                                      mut_version)) {
        if (ref_methodology_name) {
            printf("Loaded method '%s' version '%s' from methodology '%s'\n",
                   mut_method_name,
                   mut_version,
                   ref_methodology_name);
        } else {
            printf("Loaded method '%s' version '%s' from directory\n",
                   mut_method_name,
                   mut_version);
        }

        AR__HEAP__FREE(own_content);
        return true;
    }

    printf("Failed to create method '%s' version '%s'\n", mut_method_name, mut_version);
    AR__HEAP__FREE(own_content);
    return false;
}

static int _load_methods_from_directory_path(ar_methodology_t *mut_methodology,
                                             const char *ref_methods_dir,
                                             const char *ref_methodology_name) {
    DIR *mut_dir = opendir(ref_methods_dir);
    int loaded_count = 0;
    struct dirent *ref_entry;

    if (!mut_dir) {
        if (ref_methodology_name) {
            printf("Failed to open methodology directory '%s': %s\n",
                   ref_methodology_name,
                   ref_methods_dir);
        } else {
            printf("Failed to open methods directory: %s\n", ref_methods_dir);
        }
        return 0;
    }

    while ((ref_entry = readdir(mut_dir)) != NULL) {
        if (_load_method_file(mut_methodology,
                              ref_methods_dir,
                              ref_entry->d_name,
                              ref_methodology_name)) {
            loaded_count++;
        }
    }

    closedir(mut_dir);

    if (loaded_count > 0) {
        if (ref_methodology_name) {
            printf("Loaded %d methods from methodology '%s'\n",
                   loaded_count,
                   ref_methodology_name);
        } else {
            printf("Loaded %d methods from directory\n", loaded_count);
        }
    }

    return loaded_count;
}

static int _load_methods_from_directory(ar_methodology_t *mut_methodology) {
    // Allow methods directory to be overridden via environment variable
    const char *ref_methods_dir = getenv("AGERUN_METHODS_DIR");
    if (!ref_methods_dir) {
        ref_methods_dir = "../../methods";
    }

    return _load_methods_from_directory_path(mut_methodology, ref_methods_dir, NULL);
}

static int _load_methods_from_methodologies_directory(ar_methodology_t *mut_methodology) {
    const char *ref_methodologies_dir = getenv("AGERUN_METHODOLOGIES_DIR");
    DIR *mut_dir;
    int loaded_count = 0;
    struct dirent *ref_entry;

    if (!ref_methodologies_dir) {
        ref_methodologies_dir = "../../methodologies";
    }

    mut_dir = opendir(ref_methodologies_dir);
    if (!mut_dir) {
        return 0;
    }

    printf("Loading methodologies from directory...\n");

    while ((ref_entry = readdir(mut_dir)) != NULL) {
        char mut_methodology_path[2048];
        struct stat mut_entry_stat;

        if (strcmp(ref_entry->d_name, ".") == 0 || strcmp(ref_entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(mut_methodology_path,
                 sizeof(mut_methodology_path),
                 "%s/%s",
                 ref_methodologies_dir,
                 ref_entry->d_name);
        if (stat(mut_methodology_path, &mut_entry_stat) != 0 ||
            !S_ISDIR(mut_entry_stat.st_mode)) {
            continue;
        }

        loaded_count += _load_methods_from_directory_path(mut_methodology,
                                                          mut_methodology_path,
                                                          ref_entry->d_name);
    }

    closedir(mut_dir);

    if (loaded_count > 0) {
        printf("Loaded %d methods from methodologies directory\n", loaded_count);
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
    int methodology_methods_loaded = 0;
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
        methodology_methods_loaded = _load_methods_from_methodologies_directory(mut_methodology);
    }
    
    if (!loaded_from_file && methods_loaded == 0 && methodology_methods_loaded == 0) {
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
        if (methods_loaded > 0) {
            printf("Successfully loaded %d methods from directory\n", methods_loaded);
        }
        if (methodology_methods_loaded > 0) {
            printf("Successfully loaded %d methods from "
                   "methodologies directory\n",
                   methodology_methods_loaded);
        }
        printf("\n");
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
