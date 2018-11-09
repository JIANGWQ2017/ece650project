// based on the example from
// https://stackoverflow.com/questions/13041416/redirect-stdout-of-two-processes-to-another-processs-stdin-in-linux-c
#include<vector>

#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include<iostream>

/// Entry point of process A
int procA(void) {
	// Process A writing to C
	for (int i = 0; i < 100; i++)
	{
		std::cout << "Hi" << std::endl;
		usleep(5000);
	}
	std::cout << "[A] Sleeping" << std::endl;
	sleep(6);
	std::cout << "[A] Exiting" << std::endl;
	return 0;
}

/// Entry point of process B
int procB(void) {
	// Process B writing to C
	while (!std::cin.eof()) {
		// read a line of input until EOL and store in a string
		std::string line;
		std::getline(std::cin, line);
		if (line.size () > 0)
			std::cout << line << std::endl;
	}
	std::cout << "[B] saw EOF" << std::endl;
	return 0;
}

/// Entry point of process C
int procC(void) {
	// Process C reading from both A and B
	while (!std::cin.eof()) {
		// read a line of input until EOL and store in a string
		std::string line;
		std::getline(std::cin, line);
		if (line.size () > 0)
			std::cout << "[C]: " << line << std::endl;
	}
	return 0;
}








int main(void)
{
	std::vector<pid_t> kids;
	// create a pipe
	int ABtoC[2];
	pipe(ABtoC);


	pid_t child_pid;
	child_pid = fork ();
	if (child_pid == 0) // child process run proA
	{
		char* argv[6];
		argv[0] = (char*)"a1ece650.cpp";
		argv[1] = "a \"Weber Street\" (2,-1) (2,2) (5,5) (5,6) (3,8)";
		argv[2] = "a \"King Street S\" (4,2) (4,8)";
		argv[3] = "a \"Davenport Road\" (1,4) (5,8)";
		argv[4] = 'g';
		argv[5] = nullptr;
		// redirect stdout to the pipe
		dup2(ABtoC[1], STDOUT_FILENO);
		close(ABtoC[0]);
		close(ABtoC[1]);	 // Close this too!
		execv("a1ece650.cpp",argv);
		// start process A
		return 1;
	}
	else if (child_pid < 0) // fork function doesnot work
	{
		std::cerr << "Error: could not fork\n";
		return 1;
	}

	kids.push_back(child_pid);

	child_pid = fork();
	if (child_pid == 0)
	{
		// redirect stdin from the pipe
		dup2(ABtoC[0], STDIN_FILENO);
		close(ABtoC[1]);
		close(ABtoC[0]);

		// start process C
		return procC();
	}
	else if (child_pid < 0) {
		std::cerr << "Error: could not fork\n";
		return 1;
	}

	kids.push_back(child_pid);
	child_pid = 0;

	// redirect stdout to the pipe
	dup2(ABtoC[1], STDOUT_FILENO);
	close(ABtoC[0]);
	close(ABtoC[1]);

	// start process B
	int res =  procB();

	// send kill signal to all children
	for (pid_t k : kids) {
		int status;
		kill (k, SIGTERM);
		waitpid(k, &status, 0);
	}

	// exit with return code of process B
	return res;

}
