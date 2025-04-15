/* Agerun Runtime System Implementation */
#include "../include/agerun.h"
#include "../include/interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Constants */
#define MAX_AGENTS 1024
#define MAX_METHODS 256
#define MAX_VERSIONS_PER_METHOD 64
#define MAX_METHOD_NAME_LENGTH 64
#define MAX_MESSAGE_LENGTH 1024
#define MAX_INSTRUCTIONS_LENGTH 16384
#define MEMORY_SIZE 256
#define QUEUE_SIZE 256

/* Value Type Definitions */
typedef enum {
    VALUE_INT,
    VALUE_DOUBLE,
    VALUE_STRING
} value_type_t;

typedef struct value_s {
    value_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
    } data;
} value_t;

/* Memory Dictionary for Agent State */
typedef struct memory_entry_s {
    char *key;
    value_t value;
    bool is_used;
} memory_entry_t;

typedef struct memory_dict_s {
    memory_entry_t entries[MEMORY_SIZE];
    int count;
} memory_dict_t;

/* Message Queue for Agent Communication */
typedef struct message_queue_s {
    char messages[QUEUE_SIZE][MAX_MESSAGE_LENGTH];
    int head;
    int tail;
    int size;
} message_queue_t;

/* Method Definition */
typedef struct method_s {
    char name[MAX_METHOD_NAME_LENGTH];
    version_t version;
    version_t previous_version;
    bool backward_compatible;
    bool persist;
    char instructions[MAX_INSTRUCTIONS_LENGTH];
} method_t;

/* Agent Definition */
typedef struct agent_s {
    agent_id_t id;
    char method_name[MAX_METHOD_NAME_LENGTH];
    version_t method_version;
    bool is_active;
    bool is_persistent;
    message_queue_t queue;
    memory_dict_t memory;
    memory_dict_t *context;
} agent_t;

/* Global State */
static agent_t agents[MAX_AGENTS];
static method_t methods[MAX_METHODS][MAX_VERSIONS_PER_METHOD];
static int method_counts[MAX_METHODS];
static int method_name_count = 0;
static agent_id_t next_agent_id = 1;
static bool is_initialized = false;

/* Forward Declarations */
static bool init_memory_dict(memory_dict_t *dict);
static value_t* memory_get(memory_dict_t *memory, const char *key);
static void free_value(value_t *value);
static bool init_message_queue(message_queue_t *queue);
static bool queue_push(message_queue_t *queue, const char *message);
static bool queue_pop(message_queue_t *queue, char *message);
static int find_method_idx(const char *name);
static method_t* find_latest_method(const char *name);
static method_t* find_method(const char *name, version_t version);
static bool interpret_method(agent_t *agent, const char *message);

/* Implementation */
agent_id_t agerun_init(const char *method_name, version_t version) {
    if (is_initialized) {
        printf("Agerun already initialized\n");
        return 0;
    }
    
    // Initialize all agents as inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        agents[i].is_active = false;
    }
    
    // Initialize method counts
    for (int i = 0; i < MAX_METHODS; i++) {
        method_counts[i] = 0;
    }
    
    is_initialized = true;
    
    // Load methods from file if available
    if (!agerun_load_methods()) {
        printf("Warning: Could not load methods from file\n");
    }
    
    // Load agents from file if available
    if (!agerun_load_agents()) {
        printf("Warning: Could not load agents from file\n");
    }
    
    // Create initial agent if method_name is provided
    if (method_name != NULL) {
        agent_id_t initial_agent = agerun_create(method_name, version, NULL);
        if (initial_agent != 0) {
            // Send wake message to initial agent
            agerun_send(initial_agent, "__wake__");
        }
        return initial_agent;
    }
    
    return 0;
}

void agerun_shutdown(void) {
    if (!is_initialized) {
        return;
    }
    
    // Save methods to file
    agerun_save_methods();
    
    // Save persistent agents to file
    agerun_save_agents();
    
    // Clean up memory for all active agents
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            // Free memory dictionary entries
            for (int j = 0; j < MEMORY_SIZE; j++) {
                if (agents[i].memory.entries[j].is_used && agents[i].memory.entries[j].key) {
                    free(agents[i].memory.entries[j].key);
                    free_value(&agents[i].memory.entries[j].value);
                }
            }
        }
    }
    
    is_initialized = false;
}

version_t agerun_method(const char *name, const char *instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist) {
    if (!is_initialized || !name || !instructions) {
        return 0;
    }
    
    // Find or create method entry
    int method_idx = find_method_idx(name);
    if (method_idx < 0) {
        if (method_name_count >= MAX_METHODS) {
            printf("Error: Maximum number of method types reached\n");
            return 0;
        }
        
        method_idx = method_name_count++;
        strncpy(methods[method_idx][0].name, name, MAX_METHOD_NAME_LENGTH - 1);
        methods[method_idx][0].name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    }
    
    // Check if we've reached max versions for this method
    if (method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
        printf("Error: Maximum number of versions reached for method %s\n", name);
        return 0;
    }
    
    // Create new version
    int version_idx = method_counts[method_idx]++;
    version_t new_version = previous_version + 1;
    
    // Make sure the version is unique
    for (int i = 0; i < version_idx; i++) {
        if (methods[method_idx][i].version == new_version) {
            new_version = methods[method_idx][i].version + 1;
        }
    }
    
    // Initialize the new method version
    strncpy(methods[method_idx][version_idx].name, name, MAX_METHOD_NAME_LENGTH - 1);
    methods[method_idx][version_idx].name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    methods[method_idx][version_idx].version = new_version;
    methods[method_idx][version_idx].previous_version = previous_version;
    methods[method_idx][version_idx].backward_compatible = backward_compatible;
    methods[method_idx][version_idx].persist = persist;
    strncpy(methods[method_idx][version_idx].instructions, instructions, MAX_INSTRUCTIONS_LENGTH - 1);
    methods[method_idx][version_idx].instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
    
    printf("Created method %s version %d\n", name, new_version);
    
    return new_version;
}

agent_id_t agerun_create(const char *method_name, version_t version, void *context) {
    if (!is_initialized || !method_name) {
        return 0;
    }
    
    // Find free slot for new agent
    int agent_idx = -1;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (!agents[i].is_active) {
            agent_idx = i;
            break;
        }
    }
    
    if (agent_idx < 0) {
        printf("Error: Maximum number of agents reached\n");
        return 0;
    }
    
    // Find method definition
    method_t *method = NULL;
    if (version == 0) {
        // Use latest version
        method = find_latest_method(method_name);
    } else {
        // Use specific version
        method = find_method(method_name, version);
    }
    
    if (!method) {
        printf("Error: Method %s%s%d not found\n", 
               method_name, version ? " version " : "", version);
        return 0;
    }
    
    // Initialize agent structure
    agents[agent_idx].id = next_agent_id++;
    strncpy(agents[agent_idx].method_name, method_name, MAX_METHOD_NAME_LENGTH - 1);
    agents[agent_idx].method_name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    agents[agent_idx].method_version = method->version;
    agents[agent_idx].is_active = true;
    agents[agent_idx].is_persistent = method->persist;
    agents[agent_idx].context = (memory_dict_t *)context;
    
    init_memory_dict(&agents[agent_idx].memory);
    init_message_queue(&agents[agent_idx].queue);
    
    printf("Created agent %lld using method %s version %d\n", 
           agents[agent_idx].id, method_name, method->version);
    
    return agents[agent_idx].id;
}

bool agerun_destroy(agent_id_t agent_id) {
    if (!is_initialized || agent_id == 0) {
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            // Send sleep message before destroying
            agerun_send(agent_id, "__sleep__");
            
            // Process the sleep message
            char message[MAX_MESSAGE_LENGTH];
            if (queue_pop(&agents[i].queue, message)) {
                interpret_method(&agents[i], message);
            }
            
            // Free memory dictionary entries
            for (int j = 0; j < MEMORY_SIZE; j++) {
                if (agents[i].memory.entries[j].is_used && agents[i].memory.entries[j].key) {
                    free(agents[i].memory.entries[j].key);
                    free_value(&agents[i].memory.entries[j].value);
                }
            }
            
            agents[i].is_active = false;
            printf("Destroyed agent %lld\n", agent_id);
            return true;
        }
    }
    
    return false;
}

bool agerun_send(agent_id_t agent_id, const char *message) {
    if (!is_initialized || !message) {
        return false;
    }
    
    // Special case: agent_id 0 is a no-op
    if (agent_id == 0) {
        return true;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            // Add message to queue
            return queue_push(&agents[i].queue, message);
        }
    }
    
    return false;
}

bool agerun_process_next_message(void) {
    // Find an agent with a non-empty message queue
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].queue.size > 0) {
            // Process one message
            char message[MAX_MESSAGE_LENGTH];
            if (queue_pop(&agents[i].queue, message)) {
                interpret_method(&agents[i], message);
                return true;
            }
        }
    }
    
    return false; // No messages to process
}

int agerun_process_all_messages(void) {
    int count = 0;
    
    while (agerun_process_next_message()) {
        count++;
    }
    
    return count;
}

bool agerun_agent_exists(agent_id_t agent_id) {
    if (!is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            return true;
        }
    }
    
    return false;
}

int agerun_count_agents(void) {
    if (!is_initialized) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            count++;
        }
    }
    
    return count;
}

bool agerun_save_agents(void) {
    if (!is_initialized) {
        return false;
    }
    
    // Simple placeholder implementation for now
    FILE *fp = fopen("agrun.agents", "w");
    if (!fp) {
        printf("Error: Could not open agrun.agents for writing\n");
        return false;
    }
    
    // Count how many persistent agents we have
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].is_persistent) {
            count++;
        }
    }
    
    fprintf(fp, "%d\n", count);
    
    // Save basic agent info
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].is_persistent) {
            fprintf(fp, "%lld %s %d\n", agents[i].id, agents[i].method_name, agents[i].method_version);
            
            // Save memory dictionary - for simplicity just save int and string values
            fprintf(fp, "%d\n", agents[i].memory.count);
            for (int j = 0; j < MEMORY_SIZE; j++) {
                if (agents[i].memory.entries[j].is_used && agents[i].memory.entries[j].key) {
                    fprintf(fp, "%s ", agents[i].memory.entries[j].key);
                    
                    value_t *val = &agents[i].memory.entries[j].value;
                    if (val->type == VALUE_INT) {
                        fprintf(fp, "int %lld\n", val->data.int_value);
                    } else if (val->type == VALUE_DOUBLE) {
                        fprintf(fp, "double %f\n", val->data.double_value);
                    } else if (val->type == VALUE_STRING && val->data.string_value) {
                        fprintf(fp, "string %s\n", val->data.string_value);
                    } else {
                        fprintf(fp, "unknown\n");
                    }
                }
            }
        }
    }
    
    fclose(fp);
    return true;
}

bool agerun_load_agents(void) {
    if (!is_initialized) {
        return false;
    }
    
    FILE *fp = fopen("agrun.agents", "r");
    if (!fp) {
        // Not an error, might be first run
        return true;
    }
    
    int count = 0;
    fscanf(fp, "%d", &count);
    
    for (int i = 0; i < count; i++) {
        agent_id_t id;
        char method_name[MAX_METHOD_NAME_LENGTH];
        version_t version;
        
        if (fscanf(fp, "%lld %s %d", &id, method_name, &version) != 3) {
            printf("Error: Malformed agent entry in agrun.agents\n");
            fclose(fp);
            return false;
        }
        
        // Create the agent
        agent_id_t new_id = agerun_create(method_name, version, NULL);
        if (new_id == 0) {
            printf("Error: Could not recreate agent %lld\n", id);
            continue;
        }
        
        // Update the assigned ID to match the stored one
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (agents[j].is_active && agents[j].id == new_id) {
                agents[j].id = id;
                
                // Read memory dictionary
                int mem_count = 0;
                fscanf(fp, "%d", &mem_count);
                
                for (int k = 0; k < mem_count; k++) {
                    char key[256];
                    char type[32];
                    
                    if (fscanf(fp, "%255s %31s", key, type) != 2) {
                        printf("Error: Malformed memory entry in agrun.agents\n");
                        break;
                    }
                    
                    value_t value;
                    if (strcmp(type, "int") == 0) {
                        value.type = VALUE_INT;
                        fscanf(fp, "%lld", &value.data.int_value);
                    } else if (strcmp(type, "double") == 0) {
                        value.type = VALUE_DOUBLE;
                        fscanf(fp, "%lf", &value.data.double_value);
                    } else if (strcmp(type, "string") == 0) {
                        value.type = VALUE_STRING;
                        char str[1024];
                        fscanf(fp, "%1023s", str);
                        value.data.string_value = strdup(str);
                    } else {
                        // Skip unknown type
                        char line[1024];
                        fgets(line, sizeof(line), fp);
                        continue;
                    }
                    
                    memory_set(&agents[j].memory, key, &value);
                    
                    if (value.type == VALUE_STRING && value.data.string_value) {
                        free(value.data.string_value);
                    }
                }
                
                break;
            }
        }
        
        // Update next_agent_id if needed
        if (id >= next_agent_id) {
            next_agent_id = id + 1;
        }
    }
    
    fclose(fp);
    return true;
}

bool agerun_save_methods(void) {
    if (!is_initialized) {
        return false;
    }
    
    // Simple placeholder implementation for now
    FILE *fp = fopen("agrun.methods", "w");
    if (!fp) {
        printf("Error: Could not open agrun.methods for writing\n");
        return false;
    }
    
    fprintf(fp, "%d\n", method_name_count);
    
    for (int i = 0; i < method_name_count; i++) {
        fprintf(fp, "%d\n", method_counts[i]);
        
        for (int j = 0; j < method_counts[i]; j++) {
            method_t *method = &methods[i][j];
            fprintf(fp, "%s %d %d %d %d\n", 
                   method->name, method->version, method->previous_version,
                   method->backward_compatible ? 1 : 0, method->persist ? 1 : 0);
            
            // Save instructions with special encoding for newlines
            for (size_t k = 0; k < strlen(method->instructions); k++) {
                if (method->instructions[k] == '\n') {
                    fprintf(fp, "\\n");
                } else if (method->instructions[k] == '\\') {
                    fprintf(fp, "\\\\");
                } else {
                    fputc(method->instructions[k], fp);
                }
            }
            fprintf(fp, "\n");
        }
    }
    
    fclose(fp);
    return true;
}

bool agerun_load_methods(void) {
    if (!is_initialized) {
        return false;
    }
    
    FILE *fp = fopen("agrun.methods", "r");
    if (!fp) {
        // Not an error, might be first run
        return true;
    }
    
    fscanf(fp, "%d", &method_name_count);
    
    for (int i = 0; i < method_name_count; i++) {
        fscanf(fp, "%d", &method_counts[i]);
        
        for (int j = 0; j < method_counts[i]; j++) {
            method_t *method = &methods[i][j];
            int backward_compatible_int, persist_int;
            
            if (fscanf(fp, "%s %d %d %d %d", 
                      method->name, &method->version, &method->previous_version,
                      &backward_compatible_int, &persist_int) != 5) {
                printf("Error: Malformed method entry in agrun.methods\n");
                fclose(fp);
                return false;
            }
            
            method->backward_compatible = backward_compatible_int != 0;
            method->persist = persist_int != 0;
            
            // Read instructions with special handling for newlines
            int c;  // Using int for fgetc return value
            int idx = 0;
            // Skip the rest of the line
            while ((c = fgetc(fp)) != '\n' && c != EOF);
            
            while ((c = fgetc(fp)) != '\n' && c != EOF && idx < MAX_INSTRUCTIONS_LENGTH - 1) {
                if (c == '\\') {
                    c = fgetc(fp);
                    if (c == 'n') {
                        method->instructions[idx++] = '\n';
                    } else if (c == '\\') {
                        method->instructions[idx++] = '\\';
                    } else {
                        method->instructions[idx++] = '\\';
                        method->instructions[idx++] = (char)c;  // Explicit cast
                    }
                } else {
                    method->instructions[idx++] = (char)c;  // Explicit cast
                }
            }
            method->instructions[idx] = '\0';
        }
    }
    
    fclose(fp);
    return true;
}

static bool init_memory_dict(memory_dict_t *dict) {
    if (!dict) {
        return false;
    }
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        dict->entries[i].is_used = false;
        dict->entries[i].key = NULL;
    }
    
    dict->count = 0;
    return true;
}

bool memory_set(void *memory_ptr, const char *key, void *value_ptr) {
    memory_dict_t *memory = (memory_dict_t *)memory_ptr;
    value_t value = *(value_t *)value_ptr;
    if (!memory || !key) {
        return false;
    }
    
    // First, check if key already exists using memory_get
    value_t *existing = memory_get(memory, key);
    if (existing) {
        // Free old value if it's a string
        free_value(existing);
        
        // Set new value
        *existing = value;
        if (value.type == VALUE_STRING && value.data.string_value) {
            existing->data.string_value = strdup(value.data.string_value);
        }
        
        return true;
    }
    
    // Find empty slot
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (!memory->entries[i].is_used) {
            memory->entries[i].is_used = true;
            memory->entries[i].key = strdup(key);
            memory->entries[i].value = value;
            
            if (value.type == VALUE_STRING && value.data.string_value) {
                memory->entries[i].value.data.string_value = strdup(value.data.string_value);
            }
            
            memory->count++;
            return true;
        }
    }
    
    return false; // No space left
}

static void free_value(value_t *value) {
    if (!value) return;
    
    if (value->type == VALUE_STRING && value->data.string_value) {
        free(value->data.string_value);
        value->data.string_value = NULL;
    }
}

static bool init_message_queue(message_queue_t *queue) {
    if (!queue) {
        return false;
    }
    
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    
    return true;
}

static bool queue_push(message_queue_t *queue, const char *message) {
    if (!queue || !message || queue->size >= QUEUE_SIZE) {
        return false;
    }
    
    strncpy(queue->messages[queue->tail], message, MAX_MESSAGE_LENGTH - 1);
    queue->messages[queue->tail][MAX_MESSAGE_LENGTH - 1] = '\0';
    
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
    
    return true;
}

static bool queue_pop(message_queue_t *queue, char *message) {
    if (!queue || !message || queue->size == 0) {
        return false;
    }
    
    strncpy(message, queue->messages[queue->head], MAX_MESSAGE_LENGTH);
    
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    
    return true;
}

static int find_method_idx(const char *name) {
    for (int i = 0; i < method_name_count; i++) {
        if (strcmp(methods[i][0].name, name) == 0) {
            return i;
        }
    }
    
    return -1;
}

static method_t* find_latest_method(const char *name) {
    int method_idx = find_method_idx(name);
    if (method_idx < 0 || method_counts[method_idx] == 0) {
        return NULL;
    }
    
    // Find the most recent version
    version_t latest_version = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].version > latest_version) {
            latest_version = methods[method_idx][i].version;
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0) {
        return &methods[method_idx][latest_idx];
    }
    
    return NULL;
}

static method_t* find_method(const char *name, version_t version) {
    int method_idx = find_method_idx(name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Case 1: Exact version match
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].version == version) {
            return &methods[method_idx][i];
        }
    }
    
    // Case 2: Find compatible version
    version_t latest_compatible = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].backward_compatible && 
            methods[method_idx][i].version > version && 
            methods[method_idx][i].version > latest_compatible) {
            latest_compatible = methods[method_idx][i].version;
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0) {
        return &methods[method_idx][latest_idx];
    }
    
    return NULL; // No compatible version found
}

static bool interpret_method(agent_t *agent, const char *message);

static bool interpret_method(agent_t *agent, const char *message) {
    // Find the method
    method_t *method = find_method(agent->method_name, agent->method_version);
    if (!method) {
        printf("Error: Method %s version %d not found for agent %lld\n", 
               agent->method_name, agent->method_version, agent->id);
        return false;
    }
    
    printf("Agent %lld received message: %s\n", agent->id, message);
    
    // Call the interpreter function with the method instructions
    return interpret_agent_method(agent, message, method->instructions);
}


static value_t* memory_get(memory_dict_t *memory, const char *key) {
    if (!memory || !key) {
        return NULL;
    }
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->entries[i].is_used && memory->entries[i].key && 
            strcmp(memory->entries[i].key, key) == 0) {
            return &memory->entries[i].value;
        }
    }
    return NULL;
}


