## docs/api.md

```markdown
# LWThread API Documentation

## Scheduler Functions

### lwt_scheduler_t* lwt_scheduler_create(int num_threads)

Creates a new scheduler with the specified number of worker threads.

**Parameters:**
- `num_threads`: Number of OS threads to create (similar to GOMAXPROCS)

**Returns:**
- Pointer to scheduler or NULL on error

**Example:**
```c
lwt_scheduler_t* scheduler = lwt_scheduler_create(4);
if (!scheduler) {
    perror("Failed to create scheduler");
    return 1;
}