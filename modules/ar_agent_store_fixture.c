#include "ar_agent_store_fixture.h"
#include "ar_agent_store.h"
#include "ar_agent_registry.h"
#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_agent.h"
#include "ar_heap.h"
#include <stdio.h>
#include <string.h>

struct ar_agent_store_fixture_s {
    ar_methodology_t *own_methodology;
    ar_agent_registry_t *own_registry;
    ar_agent_store_t *own_store;
};

static ar_methodology_t* _create_test_methodology(void) {
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    if (!own_methodology) {
        return NULL;
    }
    
    ar_method_t *own_echo_method = ar_method__create("echo", "send(sender, message)", "1.0.0");
    if (!own_echo_method) {
        ar_methodology__destroy(own_methodology);
        return NULL;
    }
    ar_methodology__register_method(own_methodology, own_echo_method);
    
    ar_method_t *own_calc_method = ar_method__create("calculator", "send(sender, result)", "1.0.0");
    if (!own_calc_method) {
        ar_methodology__destroy(own_methodology);
        return NULL;
    }
    ar_methodology__register_method(own_methodology, own_calc_method);
    
    return own_methodology;
}

ar_agent_store_fixture_t* ar_agent_store_fixture__create_full(void) {
    ar_agent_store_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_agent_store_fixture_t), "Agent store fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    own_fixture->own_methodology = _create_test_methodology();
    if (!own_fixture->own_methodology) {
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_registry = ar_agent_registry__create();
    if (!own_fixture->own_registry) {
        ar_methodology__destroy(own_fixture->own_methodology);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_store = ar_agent_store__create(own_fixture->own_registry, own_fixture->own_methodology);
    if (!own_fixture->own_store) {
        ar_agent_registry__destroy(own_fixture->own_registry);
        ar_methodology__destroy(own_fixture->own_methodology);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    return own_fixture;
}

ar_agent_store_fixture_t* ar_agent_store_fixture__create_empty(void) {
    ar_agent_store_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_agent_store_fixture_t), "Agent store fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    own_fixture->own_methodology = ar_methodology__create(NULL);
    if (!own_fixture->own_methodology) {
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_registry = ar_agent_registry__create();
    if (!own_fixture->own_registry) {
        ar_methodology__destroy(own_fixture->own_methodology);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_store = ar_agent_store__create(own_fixture->own_registry, own_fixture->own_methodology);
    if (!own_fixture->own_store) {
        ar_agent_registry__destroy(own_fixture->own_registry);
        ar_methodology__destroy(own_fixture->own_methodology);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    return own_fixture;
}

void ar_agent_store_fixture__destroy(ar_agent_store_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    if (own_fixture->own_store) {
        ar_agent_store__delete(own_fixture->own_store);
        ar_agent_store__destroy(own_fixture->own_store);
    }
    
    if (own_fixture->own_registry) {
        // Destroy all agents before destroying registry
        int64_t agent_id = ar_agent_registry__get_first(own_fixture->own_registry);
        while (agent_id != 0) {
            int64_t next_id = ar_agent_registry__get_next(own_fixture->own_registry, agent_id);
            ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(own_fixture->own_registry, agent_id);
            if (own_agent) {
                ar_agent_registry__unregister_id(own_fixture->own_registry, agent_id);
                ar_agent__destroy(own_agent);
            }
            agent_id = next_id;
        }
        ar_agent_registry__destroy(own_fixture->own_registry);
    }
    
    if (own_fixture->own_methodology) {
        ar_methodology__destroy(own_fixture->own_methodology);
    }
    
    AR__HEAP__FREE(own_fixture);
}

int64_t ar_agent_store_fixture__create_agent(ar_agent_store_fixture_t *mut_fixture, const char *ref_method_name, const char *ref_method_version) {
    if (!mut_fixture || !ref_method_name || !ref_method_version) {
        return 0;
    }
    
    const ar_method_t *ref_method = ar_methodology__get_method(mut_fixture->own_methodology, ref_method_name, ref_method_version);
    if (!ref_method) {
        return 0;
    }
    
    ar_agent_t *own_agent = ar_agent__create_with_method(ref_method, NULL);
    if (!own_agent) {
        return 0;
    }
    
    int64_t agent_id = ar_agent_registry__allocate_id(mut_fixture->own_registry);
    if (agent_id == 0) {
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    ar_agent__set_id(own_agent, agent_id);
    
    if (!ar_agent_registry__register_id(mut_fixture->own_registry, agent_id)) {
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    ar_agent_registry__track_agent(mut_fixture->own_registry, agent_id, own_agent);
    
    return agent_id;
}

ar_data_t* ar_agent_store_fixture__get_agent_memory(ar_agent_store_fixture_t *mut_fixture, int64_t agent_id) {
    if (!mut_fixture) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_fixture->own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_mutable_memory(ref_agent);
}

bool ar_agent_store_fixture__verify_agent(const ar_agent_store_fixture_t *ref_fixture, int64_t agent_id, const char *ref_expected_method_name) {
    if (!ref_fixture || !ref_expected_method_name) {
        return false;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_fixture->own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    const ar_method_t *ref_method = ar_agent__get_method(ref_agent);
    if (!ref_method) {
        return false;
    }
    
    return strcmp(ar_method__get_name(ref_method), ref_expected_method_name) == 0;
}

void ar_agent_store_fixture__destroy_agent(ar_agent_store_fixture_t *mut_fixture, int64_t agent_id) {
    if (!mut_fixture) {
        return;
    }
    
    ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_fixture->own_registry, agent_id);
    if (own_agent) {
        ar_agent_registry__unregister_id(mut_fixture->own_registry, agent_id);
        ar_agent__destroy(own_agent);
    }
}

void ar_agent_store_fixture__destroy_agents(ar_agent_store_fixture_t *mut_fixture, const int64_t *agent_ids, int count) {
    if (!mut_fixture || !agent_ids) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        ar_agent_store_fixture__destroy_agent(mut_fixture, agent_ids[i]);
    }
}

int64_t ar_agent_store_fixture__get_agent_count(const ar_agent_store_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return 0;
    }
    
    return ar_agent_registry__count(ref_fixture->own_registry);
}

bool ar_agent_store_fixture__save(ar_agent_store_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    return ar_agent_store__save(mut_fixture->own_store);
}

bool ar_agent_store_fixture__load(ar_agent_store_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    return ar_agent_store__load(mut_fixture->own_store);
}

void ar_agent_store_fixture__delete_file(ar_agent_store_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return;
    }
    
    ar_agent_store__delete(mut_fixture->own_store);
}

const char* ar_agent_store_fixture__get_store_path(const ar_agent_store_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    
    return ar_agent_store__get_path(ref_fixture->own_store);
}

bool ar_agent_store_fixture__create_yaml_file_single(const ar_agent_store_fixture_t *ref_fixture, const char *ref_path) {
    (void)ref_fixture;
    
    if (!ref_path) {
        return false;
    }
    
    FILE *fp = fopen(ref_path, "w");
    if (!fp) {
        return false;
    }
    
    fprintf(fp, "# AgeRun YAML File\n");
    fprintf(fp, "agents:\n");
    fprintf(fp, "- id: 42\n");
    fprintf(fp, "  method_name: echo\n");
    fprintf(fp, "  method_version: 1.0.0\n");
    fprintf(fp, "  memory:\n");
    fprintf(fp, "    count: 5\n");
    fprintf(fp, "    name: test_agent\n");
    
    fclose(fp);
    
    return true;
}

bool ar_agent_store_fixture__create_yaml_file(const ar_agent_store_fixture_t *ref_fixture, const char *ref_path) {
    (void)ref_fixture;
    
    if (!ref_path) {
        return false;
    }
    
    FILE *fp = fopen(ref_path, "w");
    if (!fp) {
        return false;
    }
    
    fprintf(fp, "# AgeRun YAML File\n");
    fprintf(fp, "agents:\n");
    fprintf(fp, "- id: 10\n");
    fprintf(fp, "  method_name: echo\n");
    fprintf(fp, "  method_version: 1.0.0\n");
    fprintf(fp, "  memory:\n");
    fprintf(fp, "    message: first_agent\n");
    fprintf(fp, "- id: 20\n");
    fprintf(fp, "  method_name: calculator\n");
    fprintf(fp, "  method_version: 1.0.0\n");
    fprintf(fp, "  memory:\n");
    fprintf(fp, "    result: 100\n");
    fprintf(fp, "- id: 30\n");
    fprintf(fp, "  method_name: echo\n");
    fprintf(fp, "  method_version: 1.0.0\n");
    fprintf(fp, "  memory:\n");
    fprintf(fp, "    message: third_agent\n");
    
    fclose(fp);
    
    return true;
}

int64_t ar_agent_store_fixture__get_first_agent_id(const ar_agent_store_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return 0;
    }
    
    return ar_agent_registry__get_first(ref_fixture->own_registry);
}

int64_t ar_agent_store_fixture__get_next_agent_id(const ar_agent_store_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return 0;
    }
    
    return ar_agent_registry__get_next_id(ref_fixture->own_registry);
}
