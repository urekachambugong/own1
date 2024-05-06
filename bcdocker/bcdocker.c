#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mount.h>

// Function to run the command in the new environment
int run_command(const char *cmd) {
    system(cmd);
    return 0; // Assuming command runs successfully
}

// Function to set up the container environment
int container_setup(void *args) {
    char *cmd = (char *)args;
    printf("Container initialized. Running command: %s\n", cmd);

    // Change hostname for the container
    sethostname("bcdocker", 9);

    // Mount a new proc filesystem for the container
    mount("proc", "/proc", "proc", 0, NULL) ;
    run_command(cmd);
    umount("/proc");
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <container_name> <command>\n", argv[0]);
        return 1;
    }

    printf("Starting the container: %s\n", argv[1]);

    // Create a new stack for the child process
    void *child_stack = malloc(4096) + 4096;
    if (!child_stack) {
        perror("Failed to allocate memory for child stack");
        exit(1);
    }

    // Flags for new namespaces (PID, mount, etc.)
    int flags = SIGCHLD | CLONE_NEWPID | CLONE_NEWNS;

    // Create a new child process in its own namespaces
    pid_t pid = clone(container_setup, child_stack, flags, argv[2]);
    if (pid == -1) {
        perror("clone failed");
        return 1;
    }

    // Wait for the child process to finish
    waitpid(pid, NULL, 0);
    free(child_stack - 4096); // Clean up the allocated stack

    return 0;
}

