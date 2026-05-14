#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifdef TEST_RUNNER_ENABLE
int test_runner_real_main(int argc, char** argv);

namespace {

const int kDefaultTextBatchTimeoutMs = 10000;
const int kDefaultBuildBatchTimeoutMs = 30000;
typedef std::vector<std::pair<std::string, std::string> > EnvOverrides;

int parse_positive_int_value(const char* value, int default_value)
{
	if (value == NULL || *value == '\0')
	{
		return default_value;
	}
	char* end = NULL;
	long parsed = strtol(value, &end, 10);
	if (end == value || *end != '\0' || parsed <= 0)
	{
		return default_value;
	}
	return parsed > INT_MAX ? INT_MAX : static_cast<int>(parsed);
}

int parse_positive_int_env(const char* name, int default_value)
{
	return parse_positive_int_value(getenv(name), default_value);
}

int get_text_batch_timeout_ms()
{
	const int seconds = parse_positive_int_env("CPPGM_TEXT_TEST_TIMEOUT_SEC",
	                                           kDefaultTextBatchTimeoutMs / 1000);
	return seconds > INT_MAX / 1000 ? INT_MAX : seconds * 1000;
}

int get_build_batch_timeout_ms()
{
	const int seconds = parse_positive_int_env("CPPGM_BUILD_TEST_TIMEOUT_SEC",
	                                           kDefaultBuildBatchTimeoutMs / 1000);
	return seconds > INT_MAX / 1000 ? INT_MAX : seconds * 1000;
}

int get_batch_timeout_override_ms(const EnvOverrides& env_overrides, int default_timeout_ms)
{
	for (size_t i = 0; i < env_overrides.size(); ++i)
	{
		if (env_overrides[i].first == "CPPGM_BATCH_TIMEOUT_SEC")
		{
			const int seconds = parse_positive_int_value(env_overrides[i].second.c_str(),
			                                             default_timeout_ms / 1000);
			return seconds > INT_MAX / 1000 ? INT_MAX : seconds * 1000;
		}
	}
	return default_timeout_ms;
}

int run_real_main(int argc, char** argv)
{
	return test_runner_real_main(argc, argv);
}

std::vector<std::string> split_tab_fields(const std::string& line)
{
	std::vector<std::string> fields;
	size_t start = 0;
	while (true)
	{
		size_t tab = line.find('\t', start);
		if (tab == std::string::npos)
		{
			fields.push_back(line.substr(start));
			return fields;
		}
		fields.push_back(line.substr(start, tab - start));
		start = tab + 1;
	}
}

void chomp_cr(std::string* s)
{
	if (!s->empty() && (*s)[s->size() - 1] == '\r')
	{
		s->erase(s->size() - 1);
	}
}

int open_redirect_fd(int target_fd, const std::string& path, int flags, mode_t mode);
bool wait_for_pid_exit(pid_t pid, int* status, int wait_ms);

void clear_standard_stream_state()
{
	clearerr(stdin);
	clearerr(stdout);
	clearerr(stderr);
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();
	std::clog.clear();
}

int execute_child(
	int argc,
	char** argv,
	const std::string& stdin_file,
	const std::string& stdout_file,
	const std::string& stderr_file,
	bool use_real_main,
	const EnvOverrides& env_overrides)
{
	if (open_redirect_fd(STDIN_FILENO, stdin_file, O_RDONLY, 0) < 0)
	{
		_exit(EXIT_FAILURE);
	}
	if (stdout_file != "-" && !stdout_file.empty() &&
	    stdout_file == stderr_file)
	{
		int fd = open(stdout_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fd < 0)
		{
			perror(stdout_file.c_str());
			_exit(EXIT_FAILURE);
		}
		if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0)
		{
			perror("dup2");
			close(fd);
			_exit(EXIT_FAILURE);
		}
		close(fd);
	}
	else
	{
		if (open_redirect_fd(STDOUT_FILENO, stdout_file, O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0 ||
		    open_redirect_fd(STDERR_FILENO, stderr_file, O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0)
		{
			_exit(EXIT_FAILURE);
		}
	}

	for (size_t i = 0; i < env_overrides.size(); ++i)
	{
		setenv(env_overrides[i].first.c_str(), env_overrides[i].second.c_str(), 1);
	}

	if (use_real_main)
	{
		clear_standard_stream_state();
		const int ret = run_real_main(argc, argv);
		fflush(NULL);
		exit(ret);
	}

	execvp(argv[0], argv);
	perror(argv[0]);
	_exit(errno == ENOENT ? 127 : EXIT_FAILURE);
}

int run_with_timeout(
	int argc,
	char** argv,
	const std::string& stdin_file,
	const std::string& stdout_file,
	const std::string& stderr_file,
	int timeout_ms,
	bool use_real_main,
	const EnvOverrides& env_overrides = EnvOverrides())
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork");
		return EXIT_FAILURE;
	}

	if (pid == 0)
	{
		setpgid(0, 0);
		execute_child(argc, argv, stdin_file, stdout_file, stderr_file, use_real_main, env_overrides);
	}
	else
	{
		setpgid(pid, pid);
	}

	int status = 0;
	while (timeout_ms > 0)
	{
		pid_t res = waitpid(pid, &status, WNOHANG);
		if (res == pid)
		{
			if (WIFEXITED(status))
			{
				return WEXITSTATUS(status);
			}
			if (WIFSIGNALED(status))
			{
				return 128 + WTERMSIG(status);
			}
			return EXIT_FAILURE;
		}
		if (res < 0)
		{
			perror("waitpid");
			return EXIT_FAILURE;
		}
		usleep(10000);
		timeout_ms -= 10;
	}

	kill(-pid, SIGTERM);
	if (!wait_for_pid_exit(pid, &status, 200))
	{
		kill(-pid, SIGKILL);
		waitpid(pid, &status, 0);
	}
	return 124;
}

bool wait_for_pid_exit(pid_t pid, int* status, int wait_ms)
{
	while (wait_ms > 0)
	{
		pid_t res = waitpid(pid, status, WNOHANG);
		if (res == pid)
		{
			return true;
		}
		if (res < 0)
		{
			return false;
		}
		usleep(10000);
		wait_ms -= 10;
	}
	return false;
}

int open_redirect_fd(int target_fd, const std::string& path, int flags, mode_t mode)
{
	if (path == "-" || path.empty())
	{
		return 0;
	}
	int fd = open(path.c_str(), flags, mode);
	if (fd < 0)
	{
		perror(path.c_str());
		return -1;
	}
	if (dup2(fd, target_fd) < 0)
	{
		perror("dup2");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

EnvOverrides parse_env_overrides(const std::string& encoded)
{
	EnvOverrides env;
	size_t start = 0;
	while (start < encoded.size())
	{
		size_t end = encoded.find(';', start);
		std::string entry = encoded.substr(
			start,
			end == std::string::npos ? std::string::npos : end - start);
		size_t eq = entry.find('=');
		if (eq != std::string::npos && eq != 0)
		{
			env.push_back(std::make_pair(entry.substr(0, eq), entry.substr(eq + 1)));
		}
		if (end == std::string::npos)
		{
			break;
		}
		start = end + 1;
	}
	return env;
}

std::vector<std::string> batch_base_args(int argc, char** argv)
{
	std::vector<std::string> base_args;
	for (int i = 0; i < argc; ++i)
	{
		if (std::string(argv[i]) == "--batch-stdin")
		{
			continue;
		}
		base_args.push_back(argv[i]);
	}
	if (base_args.empty())
	{
		base_args.push_back("__SELF__");
	}
	return base_args;
}

int run_batch_stdin(int argc, char** argv)
{
	const std::vector<std::string> base_args = batch_base_args(argc, argv);
	std::string line;
	while (std::getline(std::cin, line))
	{
		chomp_cr(&line);
		if (line.empty())
		{
			continue;
		}

		int status = EXIT_FAILURE;
		std::vector<std::string> fields = split_tab_fields(line);
		if (fields.size() == 3)
		{
			std::vector<std::string> args = base_args;
			args.push_back("-o");
			args.push_back(fields[0]);
			args.push_back(fields[2]);

			std::vector<char*> argv;
			for (size_t i = 0; i < args.size(); ++i)
			{
				argv.push_back(const_cast<char*>(args[i].c_str()));
			}
			argv.push_back(NULL);

				status = run_with_timeout(
					static_cast<int>(args.size()),
					argv.data(),
					"-",
					fields[1],
					fields[1],
					get_text_batch_timeout_ms(),
					true);
		}
		else if (fields.size() >= 5)
		{
			EnvOverrides env_overrides = parse_env_overrides(fields[3]);
			std::vector<std::string> args = base_args;
			for (size_t i = 4; i < fields.size(); ++i)
			{
				args.push_back(fields[i]);
			}
			std::vector<char*> argv;
			for (size_t i = 0; i < args.size(); ++i)
			{
				argv.push_back(const_cast<char*>(args[i].c_str()));
			}
			argv.push_back(NULL);
			if (argv.size() >= 2)
			{
				// Wrapped batch requests still reuse the outer worker process,
				// but execute each compiler invocation in a fresh child. Use
				// execvp rather than re-entering run_real_main in-process so the
				// request follows the same path as direct compiler invocations.
				status = run_with_timeout(
					static_cast<int>(argv.size()) - 1,
					argv.data(),
					fields[2],
					fields[0],
					fields[1],
					get_batch_timeout_override_ms(env_overrides, get_build_batch_timeout_ms()),
					false,
					env_overrides);
			}
		}
		else if (fields.size() == 4)
		{
			EnvOverrides env_overrides = parse_env_overrides(fields[3]);
			std::vector<char*> argv;
			for (size_t i = 0; i < base_args.size(); ++i)
			{
				argv.push_back(const_cast<char*>(base_args[i].c_str()));
			}
			argv.push_back(NULL);
				status = run_with_timeout(
					static_cast<int>(argv.size()) - 1,
					argv.data(),
					fields[2],
					fields[0],
					fields[1],
					get_batch_timeout_override_ms(env_overrides, get_text_batch_timeout_ms()),
					true,
					env_overrides);
		}

		std::cout << status << std::endl;
	}
	return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv)
{
	if (getenv("WRAPPED_BATCH_STDIN") != NULL)
	{
		for (int i = 1; i < argc; ++i)
		{
			if (std::string(argv[i]) == "--batch-stdin")
			{
				return run_batch_stdin(argc, argv);
			}
		}
	}
	return run_real_main(argc, argv);
}
#endif
