#ifndef AGERUN_EXECUTABLE_H
#define AGERUN_EXECUTABLE_H

/**
 * Execute the main AgeRun example application.
 * 
 * @return Exit code (0 for success, non-zero for failure)
 * @note Ownership: The function manages all memory internally and
 *       cleans up resources before returning.
 */
int ar_executable__main(void);

#endif // AGERUN_EXECUTABLE_H
