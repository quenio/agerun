# GitHub Actions Debugging Workflow

## Learning

The GitHub CLI (`gh`) provides powerful tools for systematically investigating CI failures without relying on the web interface. This enables methodical debugging directly from the command line.

## Importance

Using gh CLI for CI debugging:
- Provides programmatic access to logs and artifacts
- Enables searching and filtering of large log files
- Allows downloading artifacts for local analysis
- Supports scripting and automation of investigations

## Example

```c
// Investigating a CI failure systematically
ar_data_t* own_run_id = ar_data__create_integer(16857175516);

// Step 1: List recent runs
// gh run list --limit=5

// Step 2: View specific run
// gh run view 16857175516

// Step 3: Get detailed logs
// gh run view 16857175516 --log > ci_log.txt

// Step 4: Download artifacts
// gh run download 16857175516 --name build-logs

// Step 5: Search logs for errors
// grep "error\|failed" ci_log.txt

ar_data__destroy(own_run_id);
```

## Generalization

Systematic CI investigation workflow:
1. List recent runs to understand overall status
2. Identify specific failing run ID
3. View run summary for high-level failure info
4. Download full logs for detailed analysis
5. Download artifacts for local reproduction
6. Search logs for specific error patterns
7. Compare with successful runs to identify differences

## Implementation

```bash
# Essential gh commands for CI debugging

# List all recent runs
gh run list --limit=10

# List only failed runs
gh run list --status=failure --limit=5

# View specific run details
gh run view <run_id>

# Download logs for analysis
gh run view <run_id> --log > /tmp/ci_log.txt

# Download build artifacts
gh run download <run_id> --name <artifact_name>

# View specific job in a run
gh run view <run_id> --job <job_id>

# Check workflow configuration
gh workflow view <workflow_name>
```

## Related Patterns

- [CI Debugging Artifact Upload](ci-debugging-artifact-upload.md)
- [Issue Currency Verification Pattern](issue-currency-verification-pattern.md)
- [GitHub Actions Deprecated Tool Migration](github-actions-deprecated-tool-migration.md)
- [CI Network Timeout Diagnosis](ci-network-timeout-diagnosis.md)