/*
    Mini sudo.
    WARNING: this utility is a major security risk. Only use on systems that
    are shielded from all outside and inside threats. 
 */

#include	<errno.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<unistd.h>

int main(int argc, char **argv, char **envp) {
	char	    *args;
	struct stat	finfo;
	char		*shellPath, *shell;
	int 	    i;

	setuid(0);
	args = (char *) malloc(sizeof(char));
	args[0]='\0';

	for (i = 1; i < argc; i++) {
		args = realloc(args, (strlen(args) + 1) + 1 + strlen(argv[i]));
		strcat(args, " ");
		strcat(args, argv[i]);
	}
	putenv("PS1=# ");

	if (stat("/bin/bash", &finfo) < 0) {
		shellPath = "/bin/sh";
		shell = "sh";
		fprintf(stdout, "bash not found, using sh\n");
	} else {
		shellPath = "/bin/bash";
		shell = "bash";
	}
	if (argc > 1) {
		if (execlp(shellPath, shell, "-c", args, (char *) 0, envp) < 0) {
			fprintf(stdout, "execlp errno: %d\n", errno);
		}
	} else if (execlp(shellPath, shell, "-o", "vi", (char *) 0, envp) < 0) {
        fprintf(stdout, "execlp errno: %d\n", errno);
	}
	return 1;	
}
