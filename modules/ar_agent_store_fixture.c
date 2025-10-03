#include "ar_agent_store_fixture.h"
#include "ar_heap.h"
#include "ar_method.h"
#include "ar_agent.h"
#include <stdio.h>
#include <string.h>

struct ar_agent_store_fixture_s {
    bool initialized;
};

ar_agent_store_fixture_t* ar_agent_store_fixture__create(void) {
    ar_agent_store_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_agent_store_fixture_t), "Agent store fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    own_fixture->initialized = true;
    
    return own_fixture;
}

void ar_agent_store_fixture__destroy(ar_agent_store_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    AR__HEAP__FREE(own_fixture);
}

ar_methodology_t* ar_agent_store_fixture__create_test_methodology(ar_agent_store_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return NULL;
    }
    
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

bool ar_agent_store_fixture__create_multiple_agents_yaml(const ar_agent_store_fixture_t *ref_fixture, const char *ref_store_path) {
    if (!ref_fixture || !ref_store_path) {
        return false;
    }
    
    FILE *fp = fopen(ref_store_path, "w");
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

bool ar_agent_store_fixture__verify_agent(const ar_agent_registry_t *ref_registry, int64_t agent_id, const char *ref_expected_method_name) {
    if (!ref_registry || !ref_expected_method_name) {
        return false;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    const ar_method_t *ref_method = ar_agent__get_method(ref_agent);
    if (!ref_method) {
        return false;
    }
    
    return strcmp(ar_method__get_name(ref_method), ref_expected_method_name) == 0;
}

void ar_agent_store_fixture__destroy_all_agents(ar_agent_registry_t *mut_registry, const int64_t *agent_ids, int count) {
    if (!mut_registry || !agent_ids) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_registry, agent_ids[i]);
        if (own_agent) {
            ar_agent_registry__unregister_id(mut_registry, agent_ids[i]);
            ar_agent__destroy(own_agent);
        }
    }
}
