---
description: Analyze session for new learnings and create properly validated kb articles
---

# New Learnings Analysis and Guidelines Update

## Step 1: Identify New Learnings

Please analyze this session and identify any new learnings, patterns, or insights that emerged. Consider:

1. **Technical discoveries**: New tools, techniques, or patterns discovered
2. **Process improvements**: Better ways of doing things that were identified
3. **Error patterns**: Common mistakes or issues that were encountered
4. **User feedback**: Important corrections or suggestions from the user
5. **Tool usage**: New or better ways of using existing tools

For each learning, provide:
- A clear description of what was learned
- Why it's important
- Specific examples from this session (if applicable)
- How it can be generalized for future use

## Step 2: Knowledge Base Article Creation

**CRITICAL: All code examples MUST use real AgeRun types and functions** ([details](kb/validated-documentation-examples.md))

### Format for Knowledge Base Files
Each learning should be saved as an individual .md file in `./kb/` directory with this structure:

```markdown
# Learning Title

## Learning
[What was discovered]

## Importance
[Why it matters]

## Example
[Code example using REAL AgeRun types/functions - see guidelines below]

## Generalization
[How to apply broadly]

## Implementation
[Commands/code when applicable - using REAL functions]

## Related Patterns
[Connected concepts with links: [name](kb/filename.md)]
```

### Code Example Guidelines (MANDATORY)

**Use REAL AgeRun types and functions only:**

**✅ PREFERRED Real Types:**
- `ar_data_t*` (most common data structure)
- `ar_agent_t*` (agents)
- `ar_method_t*` (methods)
- `ar_expression_ast_t*` (expressions)
- `ar_instruction_ast_t*` (instructions)
- `ar_list_t*` (lists)
- `ar_map_t*` (maps)
- `ar_agent_registry_t*` (agent management)

**✅ PREFERRED Real Functions:**
- `ar_data__create_*()`, `ar_data__get_*()`, `ar_data__destroy()`
- `ar_agency__create_agent()`, `ar_agency__send_to_agent()`
- `ar_agent__get_*()`, `ar_agent__set_*()`
- `ar_methodology__*()` functions
- `ar_heap__malloc()`, `ar_heap__free()` (memory management)
- `ar_expression_evaluator__*()`, `ar_instruction_evaluator__*()`

**✅ EXAMPLE Tag Usage:**
When you need hypothetical examples for teaching:
```c
// Good: Real function with teaching context
ar_data_t* data = ar_data__create_string("hello");  // EXAMPLE: Using real type for demonstration

// Good: Hypothetical function marked appropriately
ar_data_t* result = ar_example__process(data);  // EXAMPLE: Hypothetical function for teaching

// Good: Bad practice example
ar_data_t* leaked = ar_data__create_string("oops");  // BAD: Memory leak - not destroyed
```

**❌ NEVER use hypothetical types without EXAMPLE tags:**
```c
// Wrong: Don't create fake types
custom_type_t* data;  // EXAMPLE: This will fail validation

// Right: Use real types or mark appropriately  
ar_data_t* data;  // Real type - preferred
custom_type_t* data;  // EXAMPLE: Hypothetical type for teaching
```

### Type Replacement Guide

If you're tempted to use hypothetical types, replace with real ones:
- `string_t` → `ar_data_t*` (with string content)  // EXAMPLE: Hypothetical type mapping
- `list_t` → `ar_list_t*` or `ar_data_t*` (with list content)  // EXAMPLE: Hypothetical type mapping  
- `agent_t` → `ar_agent_t*`  // EXAMPLE: Hypothetical type mapping
- `config_t` → `ar_data_t*` (with map content)  // EXAMPLE: Hypothetical type mapping
- `processor_t` → `ar_expression_evaluator_t*` or `ar_instruction_evaluator_t*`  // EXAMPLE: Hypothetical type mapping
- `context_t` → `ar_data_t*`  // EXAMPLE: Hypothetical type mapping

## Step 3: Validation Before Saving

**MANDATORY: Test articles before committing**

1. **Create the .md file** with your content
2. **Run validation**:
   ```bash
   make check-docs
   ```
3. **Fix any validation errors** - the script will tell you exactly what's wrong
4. **Re-run validation** until it passes

**Common validation fixes:**
- Replace hypothetical types with real ones from the list above
- Add `// EXAMPLE:` tags to hypothetical functions
- Use `ar_data_t*` as the universal fallback type
- Reference actual functions from `modules/*.h` files

## Step 4: Review Existing Guidelines

Check CLAUDE.md to see if these learnings are already documented. If they are:
- Determine if the existing documentation is sufficient
- Identify if updates or clarifications are needed
- Note any gaps in the current documentation

## Step 5: Update Guidelines

If updates are needed to CLAUDE.md:

1. **Space-conscious writing** (CRITICAL):
   - Keep additions under 50 bytes when possible
   - Prefer updating existing lines over adding new ones
   - Use abbreviations and symbols where clear (e.g., → instead of "leads to")
   - Combine related points into single lines
   - Remove any redundancy with existing content

2. **Make learnings generic**: 
   - Remove session-specific details
   - Use specific examples only as illustrations
   - Focus on patterns that apply broadly

3. **Find appropriate sections**:
   - Locate existing sections that relate to the learnings
   - PREFER: Add to existing bullet points rather than creating new sections
   - If new section needed, keep header concise

4. **Link to detailed articles**:
   ```markdown
   - Guideline text ([details](kb/filename.md))
   ```

## Step 6: Commit Process

**MANDATORY SEQUENCE:**

1. **Validate first**:
   ```bash
   make check-docs
   ```
   Fix any errors before proceeding.

2. **Commit knowledge base files**:
   ```bash
   git add kb/
   git commit -m "feat: add knowledge base with [topic] learnings

   - [Brief description of main learning]
   - All code examples use real AgeRun types and functions
   - Validated with make check-docs"
   ```

3. **Commit guidelines updates** (if CLAUDE.md was updated):
   ```bash
   git add CLAUDE.md
   git commit -m "docs: enhance guidelines with session learnings on [topic]

   - [Summary of key updates]  
   - [Links to detailed kb/ articles]"
   ```

## Documentation Validation Details

The enhanced documentation system ensures:
- **Real code**: All examples use actual AgeRun types and functions
- **Validation**: Automated checking prevents hypothetical examples
- **Consistency**: Same standards across all documentation
- **Maintenance**: Changes to code automatically flag outdated docs

**Available for reference:**
- Real types and functions: `python3 scripts/get_real_types.py --guide`
- Complete type list: `python3 scripts/get_real_types.py --types`
- Complete function list: `python3 scripts/get_real_types.py --functions`
- Type mappings: See `scripts/batch_fix_docs.py` TYPE_REPLACEMENTS

## Knowledge Base Benefits

The enhanced documentation system provides:
- **CLAUDE.md**: Concise, actionable guidelines for quick reference
- **./kb/ files**: Detailed context with VALIDATED examples
- **Real code**: All examples work with actual AgeRun codebase
- **Searchable**: `grep -r "keyword" ./kb/` finds relevant patterns
- **Linked**: Guidelines link to detailed articles when more context needed
- **Validated**: `make check-docs` ensures all references are correct

Remember: The goal is to create learnings with working code examples that developers can actually use, making future sessions more efficient and error-free.