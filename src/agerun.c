#include "../include/agerun.h"
#include "../include/agerun_interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_AGENTS 1000
#define MAX_METHODS 100
#define MAX_VERSIONS 50
#define MAX_NAME_LENGTH 64
#define MAX_MESSAGE_LENGTH 1024
#define MAX_QUEUE_SIZE 100
#define MAX_MEMORY_ENTRIES 100
#define AGENT_PERSISTENCE_FILE "agrun.agents"
#define METHOD_PERSISTENCE_FILE "agrun.methods"

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

typedef struct {
    char key[MAX_NAME_LENGTH];
    value_t value;
} memory_entry_t;

typedef struct {
    char messages[MAX_QUEUE_SIZE][MAX_MESSAGE_LENGTH];
    int front;
    int rear;
    int size;
} message_queue_t;

typedef struct memory_dict_s {
    memory_entry_t entries[MAX_MEMORY_ENTRIES];
    int count;
} memory_dict_t;

typedef struct agent_s {
    agent_id_t id;
    char method_name[MAX_NAME_LENGTH];
    version_t method_version;
    bool is_active;
    bool is_persistent;
    message_queue_t queue;
    memory_dict_t memory;
    memory_dict_t *context; /* Pointer to parent's memory */
} agent_t;

typedef struct {
    char name[MAX_NAME_LENGTH];
    version_t version;
    char *instructions;
    version_t previous_version;
    bool backward_compatible;
    bool persist;
} method_t;

static agent_t agents[MAX_AGENTS];
static method_t methods[MAX_METHODS][MAX_VERSIONS];
static int method_count = 0;
static int version_counts[MAX_METHODS] = {0};
static agent_id_t next_agent_id = 1;
static bool is_initialized = false;

// Forward declarations for internal functions
static agent_t* find_agent(agent_id_t id);
static method_t* find_method(const char *name, version_t version);
static value_t parse_value(const char *str);
static void free_value(value_t *value);
static bool queue_push(message_queue_t *queue, const char *message);
static bool queue_pop(message_queue_t *queue, char *message);
static bool memory_set(memory_dict_t *memory, const char *key, value_t value);
static value_t* memory_get(memory_dict_t *memory, const char *key);
static value_t* context_get(memory_dict_t *context, const char *key);


agent_id_t agerun_init(const char *method_name, version_t version) {
    if (is_initialized) {
        return 0; // Already initialized
    }
    
    // Initialize agent and method arrays
    memset(agents, 0, sizeof(agents));
    memset(methods, 0, sizeof(methods));
    
    // Load methods from file
    agerun_load_methods();
    
    // Create the initial agent
    agent_id_t initial_agent = agerun_create(method_name, version, NULL);
    if (initial_agent == 0) {
        return 0; // Failed to create initial agent
    }
    
    // Load persistent agents
    agerun_load_agents();
    
    // Send __wake__ message to the initial agent
    agerun_send(initial_agent, "__wake__");
    
    is_initialized = true;
    return initial_agent;
}

void agerun_shutdown(void) {
    if (!is_initialized) {
        return;
    }
    
    // Process all remaining messages
    agerun_process_all_messages();
    
    // Send __sleep__ to all active agents
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            agerun_send(agents[i].id, "__sleep__");
        }
    }
    
    // Process sleep messages
    agerun_process_all_messages();
    
    // Save persistent agents and methods
    agerun_save_agents();
    agerun_save_methods();
    
    // Free allocated memory
    for (int i = 0; i < method_count; i++) {
        for (int j = 0; j < version_counts[i]; j++) {
            free(methods[i][j].instructions);
        }
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            for (int j = 0; j < agents[i].memory.count; j++) {
                if (agents[i].memory.entries[j].value.type == VALUE_STRING) {
                    free(agents[i].memory.entries[j].value.data.string_value);
                }
            }
        }
    }
    
    is_initialized = false;
}

version_t agerun_method(const char *name, const char *instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist) {
    if (!name || !instructions) {
        return 0;
    }
    
    // Find or create method
    int method_idx = -1;
    for (int i = 0; i < method_count; i++) {
        if (strcmp(methods[i][0].name, name) == 0) {
            method_idx = i;
            break;
        }
    }
    
    if (method_idx == -1) {
        // New method
        if (method_count >= MAX_METHODS) {
            return 0; // Too many methods
        }
        method_idx = method_count++;
    }
    
    // Check if previous version exists if specified
    if (previous_version > 0) {
        bool found = false;
        for (int i = 0; i < version_counts[method_idx]; i++) {
            if (methods[method_idx][i].version == previous_version) {
                found = true;
                break;
            }
        }
        if (!found) {
            return 0; // Previous version not found
        }
    }
    
    // Create new version
    int ver_idx = version_counts[method_idx]++;
    if (ver_idx >= MAX_VERSIONS) {
        version_counts[method_idx]--;
        return 0; // Too many versions
    }
    
    version_t new_version = (previous_version > 0) ? previous_version + 1 : 1;
    
    // Ensure unique version number
    for (int i = 0; i < ver_idx; i++) {
        if (methods[method_idx][i].version >= new_version) {
            new_version = methods[method_idx][i].version + 1;
        }
    }
    
    // Initialize the new method version
    strncpy(methods[method_idx][ver_idx].name, name, MAX_NAME_LENGTH - 1);
    methods[method_idx][ver_idx].name[MAX_NAME_LENGTH - 1] = '\0';
    methods[method_idx][ver_idx].version = new_version;
    methods[method_idx][ver_idx].instructions = strdup(instructions);
    methods[method_idx][ver_idx].previous_version = previous_version;
    methods[method_idx][ver_idx].backward_compatible = backward_compatible;
    methods[method_idx][ver_idx].persist = persist;
    
    return new_version;
}

agent_id_t agerun_create(const char *method_name, version_t version, void *context) {
    if (!method_name) {
        return 0;
    }
    
    // Find the method
    method_t *method = find_method(method_name, version);
    if (!method) {
        return 0; // Method not found
    }
    
    // Find an empty slot for the new agent
    int slot = -1;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (!agents[i].is_active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        return 0; // No space for new agent
    }
    
    // Initialize the agent
    agent_id_t new_id = next_agent_id++;
    agents[slot].id = new_id;
    strncpy(agents[slot].method_name, method_name, MAX_NAME_LENGTH - 1);
    agents[slot].method_name[MAX_NAME_LENGTH - 1] = '\0';
    agents[slot].method_version = method->version;
    agents[slot].is_active = true;
    agents[slot].is_persistent = method->persist;
    agents[slot].queue.front = 0;
    agents[slot].queue.rear = 0;
    agents[slot].queue.size = 0;
    agents[slot].memory.count = 0;
    agents[slot].context = (memory_dict_t*)context;
    
    return new_id;
}

bool agerun_destroy(agent_id_t agent_id) {
    agent_t *agent = find_agent(agent_id);
    if (!agent) {
        return false;
    }
    
    // Send __sleep__ message before destroying
    agerun_send(agent_id, "__sleep__");
    agerun_process_all_messages();
    
    // Free string values in memory
    for (int i = 0; i < agent->memory.count; i++) {
        if (agent->memory.entries[i].value.type == VALUE_STRING) {
            free(agent->memory.entries[i].value.data.string_value);
        }
    }
    
    // Mark as inactive
    agent->is_active = false;
    return true;
}

bool agerun_send(agent_id_t agent_id, const char *message) {
    if (agent_id == 0 || !message) {
        return true; // No-op for agent_id 0
    }
    
    agent_t *agent = find_agent(agent_id);
    if (!agent) {
        return false; // Agent not found
    }
    
    return queue_push(&agent->queue, message);
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
    return find_agent(agent_id) != NULL;
}

int agerun_count_agents(void) {
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            count++;
        }
    }
    return count;
}

bool agerun_save_agents(void) {
    FILE *fp = fopen(AGENT_PERSISTENCE_FILE, "wb");
    if (!fp) {
        return false;
    }
    
    int persistent_count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].is_persistent) {
            persistent_count++;
        }
    }
    
    // Write count of persistent agents
    fwrite(&persistent_count, sizeof(int), 1, fp);
    
    // Write each persistent agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].is_persistent) {
            // Write agent basic info
            fwrite(&agents[i].id, sizeof(agent_id_t), 1, fp);
            fwrite(agents[i].method_name, sizeof(char), MAX_NAME_LENGTH, fp);
            fwrite(&agents[i].method_version, sizeof(version_t), 1, fp);
            
            // Write message queue
            fwrite(&agents[i].queue.size, sizeof(int), 1, fp);
            int idx = agents[i].queue.front;
            for (int j = 0; j < agents[i].queue.size; j++) {
                fwrite(agents[i].queue.messages[idx], sizeof(char), MAX_MESSAGE_LENGTH, fp);
                idx = (idx + 1) % MAX_QUEUE_SIZE;
            }
            
            // Write memory dictionary
            fwrite(&agents[i].memory.count, sizeof(int), 1, fp);
            for (int j = 0; j < agents[i].memory.count; j++) {
                memory_entry_t *entry = &agents[i].memory.entries[j];
                fwrite(entry->key, sizeof(char), MAX_NAME_LENGTH, fp);
                fwrite(&entry->value.type, sizeof(value_type_t), 1, fp);
                
                switch (entry->value.type) {
                    case VALUE_INT:
                        fwrite(&entry->value.data.int_value, sizeof(int64_t), 1, fp);
                        break;
                    case VALUE_DOUBLE:
                        fwrite(&entry->value.data.double_value, sizeof(double), 1, fp);
                        break;
                    case VALUE_STRING: {
                        size_t len = strlen(entry->value.data.string_value) + 1;
                        fwrite(&len, sizeof(size_t), 1, fp);
                        fwrite(entry->value.data.string_value, sizeof(char), len, fp);
                        break;
                    }
                }
            }
        }
    }
    
    fclose(fp);
    return true;
}

bool agerun_load_agents(void) {
    FILE *fp = fopen(AGENT_PERSISTENCE_FILE, "rb");
    if (!fp) {
        return false; // File doesn't exist or can't be opened
    }
    
    int persistent_count;
    if (fread(&persistent_count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return false;
    }
    
    for (int i = 0; i < persistent_count; i++) {
        agent_id_t id;
        char method_name[MAX_NAME_LENGTH];
        version_t method_version;
        
        // Read agent basic info
        if (fread(&id, sizeof(agent_id_t), 1, fp) != 1 ||
            fread(method_name, sizeof(char), MAX_NAME_LENGTH, fp) != MAX_NAME_LENGTH ||
            fread(&method_version, sizeof(version_t), 1, fp) != 1) {
            
            fclose(fp);
            return false;
        }
        
        // Find an empty slot
        int slot = -1;
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (!agents[j].is_active) {
                slot = j;
                break;
            }
        }
        
        if (slot == -1) {
            fclose(fp);
            return false; // No space for agent
        }
        
        // Ensure we have an appropriate id for future agents
        if (id >= next_agent_id) {
            next_agent_id = id + 1;
        }
        
        // Setup basic agent properties
        agents[slot].id = id;
        strncpy(agents[slot].method_name, method_name, MAX_NAME_LENGTH);
        agents[slot].method_version = method_version;
        agents[slot].is_active = true;
        agents[slot].is_persistent = true;
        agents[slot].context = NULL;
        
        // Read message queue
        int queue_size;
        if (fread(&queue_size, sizeof(int), 1, fp) != 1) {
            fclose(fp);
            return false;
        }
        
        agents[slot].queue.front = 0;
        agents[slot].queue.rear = queue_size % MAX_QUEUE_SIZE;
        agents[slot].queue.size = queue_size;
        
        for (int j = 0; j < queue_size; j++) {
            if (fread(agents[slot].queue.messages[j], sizeof(char), MAX_MESSAGE_LENGTH, fp) != MAX_MESSAGE_LENGTH) {
                fclose(fp);
                return false;
            }
        }
        
        // Read memory dictionary
        int memory_count;
        if (fread(&memory_count, sizeof(int), 1, fp) != 1) {
            fclose(fp);
            return false;
        }
        
        agents[slot].memory.count = memory_count;
        
        for (int j = 0; j < memory_count; j++) {
            memory_entry_t *entry = &agents[slot].memory.entries[j];
            
            if (fread(entry->key, sizeof(char), MAX_NAME_LENGTH, fp) != MAX_NAME_LENGTH ||
                fread(&entry->value.type, sizeof(value_type_t), 1, fp) != 1) {
                
                fclose(fp);
                return false;
            }
            
            switch (entry->value.type) {
                case VALUE_INT:
                    if (fread(&entry->value.data.int_value, sizeof(int64_t), 1, fp) != 1) {
                        fclose(fp);
                        return false;
                    }
                    break;
                case VALUE_DOUBLE:
                    if (fread(&entry->value.data.double_value, sizeof(double), 1, fp) != 1) {
                        fclose(fp);
                        return false;
                    }
                    break;
                case VALUE_STRING: {
                    size_t len;
                    if (fread(&len, sizeof(size_t), 1, fp) != 1) {
                        fclose(fp);
                        return false;
                    }
                    
                    entry->value.data.string_value = (char*)malloc(len);
                    if (!entry->value.data.string_value) {
                        fclose(fp);
                        return false;
                    }
                    
                    if (fread(entry->value.data.string_value, sizeof(char), len, fp) != len) {
                        free(entry->value.data.string_value);
                        fclose(fp);
                        return false;
                    }
                    break;
                }
            }
        }
        
        // Send __wake__ message
        agerun_send(id, "__wake__");
    }
    
    fclose(fp);
    return true;
}

bool agerun_save_methods(void) {
    FILE *fp = fopen(METHOD_PERSISTENCE_FILE, "wb");
    if (!fp) {
        return false;
    }
    
    // Write method count
    fwrite(&method_count, sizeof(int), 1, fp);
    
    // Write version counts for each method
    fwrite(version_counts, sizeof(int), MAX_METHODS, fp);
    
    // Write each method and version
    for (int i = 0; i < method_count; i++) {
        for (int j = 0; j < version_counts[i]; j++) {
            method_t *method = &methods[i][j];
            
            // Write fixed-size data
            fwrite(method->name, sizeof(char), MAX_NAME_LENGTH, fp);
            fwrite(&method->version, sizeof(version_t), 1, fp);
            fwrite(&method->previous_version, sizeof(version_t), 1, fp);
            fwrite(&method->backward_compatible, sizeof(bool), 1, fp);
            fwrite(&method->persist, sizeof(bool), 1, fp);
            
            // Write instructions (variable length)
            size_t len = strlen(method->instructions) + 1;
            fwrite(&len, sizeof(size_t), 1, fp);
            fwrite(method->instructions, sizeof(char), len, fp);
        }
    }
    
    fclose(fp);
    return true;
}

bool agerun_load_methods(void) {
    FILE *fp = fopen(METHOD_PERSISTENCE_FILE, "rb");
    if (!fp) {
        return false; // File doesn't exist or can't be opened
    }
    
    // Read method count
    if (fread(&method_count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return false;
    }
    
    // Read version counts
    if (fread(version_counts, sizeof(int), MAX_METHODS, fp) != MAX_METHODS) {
        fclose(fp);
        return false;
    }
    
    // Read each method and version
    for (int i = 0; i < method_count; i++) {
        for (int j = 0; j < version_counts[i]; j++) {
            method_t *method = &methods[i][j];
            
            // Read fixed-size data
            if (fread(method->name, sizeof(char), MAX_NAME_LENGTH, fp) != MAX_NAME_LENGTH ||
                fread(&method->version, sizeof(version_t), 1, fp) != 1 ||
                fread(&method->previous_version, sizeof(version_t), 1, fp) != 1 ||
                fread(&method->backward_compatible, sizeof(bool), 1, fp) != 1 ||
                fread(&method->persist, sizeof(bool), 1, fp) != 1) {
                
                fclose(fp);
                return false;
            }
            
            // Read instructions
            size_t len;
            if (fread(&len, sizeof(size_t), 1, fp) != 1) {
                fclose(fp);
                return false;
            }
            
            method->instructions = (char*)malloc(len);
            if (!method->instructions) {
                fclose(fp);
                return false;
            }
            
            if (fread(method->instructions, sizeof(char), len, fp) != len) {
                free(method->instructions);
                fclose(fp);
                return false;
            }
        }
    }
    
    fclose(fp);
    return true;
}

// Internal helper functions implementation

static agent_t* find_agent(agent_id_t id) {
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == id) {
            return &agents[i];
        }
    }
    return NULL;
}

static method_t* find_method(const char *name, version_t version) {
    int method_idx = -1;
    
    // Find the method by name
    for (int i = 0; i < method_count; i++) {
        if (strcmp(methods[i][0].name, name) == 0) {
            method_idx = i;
            break;
        }
    }
    
    if (method_idx == -1) {
        return NULL; // Method not found
    }
    
    // If version is 0, find the latest compatible version
    if (version == 0) {
        version_t latest_version = 0;
        int latest_idx = -1;
        
        for (int i = 0; i < version_counts[method_idx]; i++) {
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
    
    // Find the specified version
    for (int i = 0; i < version_counts[method_idx]; i++) {
        if (methods[method_idx][i].version == version) {
            return &methods[method_idx][i];
        }
    }
    
    // If not found, find the latest compatible version
    version_t latest_compatible = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < version_counts[method_idx]; i++) {
        if (methods[method_idx][i].version > version) {
            continue; // Skip future versions
        }
        
        if (methods[method_idx][i].backward_compatible && 
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

static bool interpret_method(agent_t *agent, const char *message) {
    // Find the method
    method_t *method = find_method(agent->method_name, agent->method_version);
    if (!method) {
        printf("Error: Method %s version %d not found for agent %ld\n", 
               agent->method_name, agent->method_version, agent->id);
        return false;
    }
    
    printf("Agent %ld received message: %s\n", agent->id, message);
    
    // Call the interpreter function with the method instructions
    return interpret_agent_method(agent, message, method->instructions);
}

static value_t parse_value(const char *str) {
    value_t value;
    char *endptr;
    
    // Try to parse as integer
    int64_t int_val = strtoll(str, &endptr, 10);
    if (*endptr == '\0') {
        value.type = VALUE_INT;
        value.data.int_value = int_val;
        return value;
    }
    
    // Try to parse as double
    double double_val = strtod(str, &endptr);
    if (*endptr == '\0') {
        value.type = VALUE_DOUBLE;
        value.data.double_value = double_val;
        return value;
    }
    
    // Treat as string
    value.type = VALUE_STRING;
    value.data.string_value = strdup(str);
    return value;
}

static void free_value(value_t *value) {
    if (value->type == VALUE_STRING && value->data.string_value) {
        free(value->data.string_value);
        value->data.string_value = NULL;
    }
}

static bool queue_push(message_queue_t *queue, const char *message) {
    if (queue->size >= MAX_QUEUE_SIZE) {
        return false; // Queue is full
    }
    
    strncpy(queue->messages[queue->rear], message, MAX_MESSAGE_LENGTH - 1);
    queue->messages[queue->rear][MAX_MESSAGE_LENGTH - 1] = '\0';
    
    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    queue->size++;
    
    return true;
}

static bool queue_pop(message_queue_t *queue, char *message) {
    if (queue->size <= 0) {
        return false; // Queue is empty
    }
    
    strncpy(message, queue->messages[queue->front], MAX_MESSAGE_LENGTH);
    
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->size--;
    
    return true;
}

static bool memory_set(memory_dict_t *memory, const char *key, value_t value) {
    // Look for existing key
    for (int i = 0; i < memory->count; i++) {
        if (strcmp(memory->entries[i].key, key) == 0) {
            // Free previous value if it was a string
            free_value(&memory->entries[i].value);
            
            // Set new value
            memory->entries[i].value = value;
            return true;
        }
    }
    
    // Add new entry
    if (memory->count >= MAX_MEMORY_ENTRIES) {
        free_value(&value);
        return false; // No space for new entry
    }
    
    strncpy(memory->entries[memory->count].key, key, MAX_NAME_LENGTH - 1);
    memory->entries[memory->count].key[MAX_NAME_LENGTH - 1] = '\0';
    memory->entries[memory->count].value = value;
    memory->count++;
    
    return true;
}

static value_t* memory_get(memory_dict_t *memory, const char *key) {
    for (int i = 0; i < memory->count; i++) {
        if (strcmp(memory->entries[i].key, key) == 0) {
            return &memory->entries[i].value;
        }
    }
    return NULL;
}

static value_t* context_get(memory_dict_t *context, const char *key) {
    if (!context) {
        return NULL;
    }
    
    return memory_get(context, key);
}