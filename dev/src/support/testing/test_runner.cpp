#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <cctype>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifdef __APPLE__
// These libproc entry points only need the types from sys/resource.h. Including
// libproc.h also brings in unrelated Mach message language extensions.
extern "C" int proc_listpgrppids(pid_t group, void* buffer, int bytes);
extern "C" int proc_pid_rusage(int pid, int flavor, rusage_info_t* buffer);
#endif

#ifdef TEST_RUNNER_ENABLE
int test_runner_real_main(int argc, char** argv);

namespace {

const int kDefaultTextBatchTimeoutMs = 10000;
const int kDefaultBuildBatchTimeoutMs = 30000;
typedef std::vector<std::pair<std::string, std::string>> EnvOverrides;

volatile sig_atomic_t active_child = 0;

void cancel_worker(int signal)
{
	if (active_child > 0)
		kill(-active_child, SIGKILL);
	_exit(128 + signal);
}

void install_worker_cancellation()
{
	struct sigaction action = {};
	action.sa_handler = cancel_worker;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}

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
	const int seconds = parse_positive_int_env("CPPGM_TEXT_TEST_TIMEOUT_SEC", kDefaultTextBatchTimeoutMs / 1000);
	return seconds > INT_MAX / 1000 ? INT_MAX : seconds * 1000;
}

int get_build_batch_timeout_ms()
{
	const int seconds = parse_positive_int_env("CPPGM_BUILD_TEST_TIMEOUT_SEC", kDefaultBuildBatchTimeoutMs / 1000);
	return seconds > INT_MAX / 1000 ? INT_MAX : seconds * 1000;
}

int get_batch_timeout_override_ms(const EnvOverrides& env_overrides, int default_timeout_ms)
{
	for (size_t i = 0; i < env_overrides.size(); ++i)
	{
		if (env_overrides[i].first == "CPPGM_BATCH_TIMEOUT_SEC")
		{
			const int seconds = parse_positive_int_value(env_overrides[i].second.c_str(), default_timeout_ms / 1000);
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
int reap_child(pid_t pid, int* status, int timeout_ms);

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

// fork() does not honour FD_CLOEXEC; execve() does. When a request runs
// in-process we have to close those descriptors by hand, or the compiler sees
// worker plumbing that a directly invoked compiler would never see.
void close_cloexec_descriptors()
{
	DIR* dir = opendir("/proc/self/fd");
	if (dir == NULL)
	{
		return;
	}
	const int dir_fd = dirfd(dir);
	std::vector<int> doomed;
	for (dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir))
	{
		char* end = NULL;
		const long fd = strtol(entry->d_name, &end, 10);
		if (end == entry->d_name || *end != '\0')
		{
			continue;
		}
		if (fd <= STDERR_FILENO || fd == dir_fd)
		{
			continue;
		}
		const int flags = fcntl(static_cast<int>(fd), F_GETFD);
		if (flags >= 0 && (flags & FD_CLOEXEC) != 0)
		{
			doomed.push_back(static_cast<int>(fd));
		}
	}
	closedir(dir);
	for (size_t i = 0; i < doomed.size(); ++i)
	{
		close(doomed[i]);
	}
}

// execve() resets caught signals to SIG_DFL but leaves ignored signals
// ignored. fork() inherits both. Mirror the execve() behaviour so an
// in-process request starts from the same dispositions as a fresh process.
void reset_caught_signal_dispositions()
{
#ifdef NSIG
	const int signal_limit = NSIG;
#else
	const int signal_limit = 65;
#endif
	for (int sig = 1; sig < signal_limit; ++sig)
	{
		if (sig == SIGKILL || sig == SIGSTOP)
		{
			continue;
		}
		struct sigaction current;
		memset(&current, 0, sizeof(current));
		if (sigaction(sig, NULL, &current) != 0)
		{
			continue;
		}
		if ((current.sa_flags & SA_SIGINFO) == 0 && (current.sa_handler == SIG_DFL || current.sa_handler == SIG_IGN))
		{
			continue;
		}
		struct sigaction reset;
		memset(&reset, 0, sizeof(reset));
		reset.sa_handler = SIG_DFL;
		sigemptyset(&reset.sa_mask);
		sigaction(sig, &reset, NULL);
	}
}

// Escape hatch: CPPGM_BATCH_EXEC=1 forces every batch request back through
// execvp(). Keep this working; it is how in-process regressions get bisected.
bool batch_force_exec()
{
	const char* value = getenv("CPPGM_BATCH_EXEC");
	if (value == NULL || *value == '\0')
	{
		return false;
	}
	return strcmp(value, "0") != 0;
}

[[noreturn]] void child_setup_failure(int status, int error_fd)
{
	if (error_fd >= 0)
	{
		const char failed = 1;
		const ssize_t sent = write(error_fd, &failed, 1);
		(void)sent;
	}
	_exit(status);
}

int execute_child(int argc, char** argv, const std::string& stdin_file, const std::string& stdout_file,
	const std::string& stderr_file, bool use_real_main, const EnvOverrides& env_overrides,
	int setup_failure = EXIT_FAILURE, int error_fd = -1)
{
	if (open_redirect_fd(STDIN_FILENO, stdin_file, O_RDONLY, 0) < 0)
	{
		child_setup_failure(setup_failure, error_fd);
	}
	if (stdout_file != "-" && !stdout_file.empty() && stdout_file == stderr_file)
	{
		int fd = open(stdout_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fd < 0)
		{
			perror(stdout_file.c_str());
			child_setup_failure(setup_failure, error_fd);
		}
		if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0)
		{
			perror("dup2");
			close(fd);
			child_setup_failure(setup_failure, error_fd);
		}
		close(fd);
	}
	else
	{
		if (open_redirect_fd(STDOUT_FILENO, stdout_file, O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0 ||
			open_redirect_fd(STDERR_FILENO, stderr_file, O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0)
		{
			child_setup_failure(setup_failure, error_fd);
		}
	}

	for (size_t i = 0; i < env_overrides.size(); ++i)
	{
		if (setenv(env_overrides[i].first.c_str(), env_overrides[i].second.c_str(), 1) != 0)
			child_setup_failure(setup_failure, error_fd);
	}

	if (use_real_main)
	{
		close_cloexec_descriptors();
		reset_caught_signal_dispositions();
		clear_standard_stream_state();
		const int ret = run_real_main(argc, argv);
		fflush(NULL);
		exit(ret);
	}

	execvp(argv[0], argv);
	perror(argv[0]);
	child_setup_failure(errno == ENOENT ? 127 : setup_failure, error_fd);
}

int run_with_timeout(int argc, char** argv, const std::string& stdin_file, const std::string& stdout_file,
	const std::string& stderr_file, int timeout_ms, bool use_real_main,
	const EnvOverrides& env_overrides = EnvOverrides())
{
	// Drain buffered parent output before forking. A child that inherits a
	// non-empty stdio buffer re-emits it on exit, duplicating worker output.
	fflush(NULL);
	std::cout.flush();
	sigset_t blocked, previous;
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGINT);
	sigaddset(&blocked, SIGTERM);
	sigprocmask(SIG_BLOCK, &blocked, &previous);

	pid_t pid = fork();
	if (pid < 0)
	{
		sigprocmask(SIG_SETMASK, &previous, NULL);
		perror("fork");
		return EXIT_FAILURE;
	}

	if (pid == 0)
	{
		setpgid(0, 0);
		reset_caught_signal_dispositions();
		sigprocmask(SIG_SETMASK, &previous, NULL);
		execute_child(argc, argv, stdin_file, stdout_file, stderr_file, use_real_main, env_overrides);
	}
	else
	{
		setpgid(pid, pid);
	}
	active_child = pid;
	sigprocmask(SIG_SETMASK, &previous, NULL);

	int status = 0;
	const int reap = reap_child(pid, &status, timeout_ms);
	if (reap > 0)
	{
		kill(-pid, SIGKILL);
		active_child = 0;
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
	if (reap < 0)
	{
		kill(-pid, SIGKILL);
		active_child = 0;
		perror("waitpid");
		return EXIT_FAILURE;
	}

	kill(-pid, SIGTERM);
	if (!wait_for_pid_exit(pid, &status, 200))
	{
		kill(-pid, SIGKILL);
		waitpid(pid, &status, 0);
	}
	// A group leader can exit on TERM while its descendants ignore it.
	kill(-pid, SIGKILL);
	active_child = 0;
	return 124;
}

double monotonic_ms()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	{
		return 0.0;
	}
	return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

// Waits for a child to exit. Returns >0 once it has been reaped, 0 if
// timeout_ms elapsed first, <0 on error.
//
// A pidfd lets the parent sleep until the child actually exits. The previous
// implementation polled waitpid(WNOHANG) on a fixed 10 ms tick, so every test
// was rounded up to a 10 ms multiple -- several times what a whole compiler
// invocation costs. Note that this also makes the timeout accurate: the old
// loop charged 10 ms per iteration regardless of how long the iteration really
// took, so it always waited somewhat longer than asked.
int reap_child(pid_t pid, int* status, int timeout_ms)
{
	if (timeout_ms <= 0)
	{
		return 0;
	}

#if defined(__linux__) && defined(__NR_pidfd_open)
	const int pidfd = static_cast<int>(syscall(__NR_pidfd_open, pid, 0));
	if (pidfd >= 0)
	{
		int remaining_ms = timeout_ms;
		int outcome = 0;
		while (remaining_ms > 0)
		{
			const double started_ms = monotonic_ms();
			struct pollfd pfd;
			pfd.fd = pidfd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			const int ready = poll(&pfd, 1, remaining_ms);
			if (ready > 0)
			{
				outcome = 1;
				break;
			}
			if (ready == 0)
			{
				break;
			}
			if (errno != EINTR)
			{
				outcome = -1;
				break;
			}
			const int elapsed_ms = static_cast<int>(monotonic_ms() - started_ms);
			remaining_ms -= elapsed_ms > 0 ? elapsed_ms : 1;
		}
		close(pidfd);
		if (outcome > 0)
		{
			return waitpid(pid, status, 0) == pid ? 1 : -1;
		}
		return outcome;
	}
#endif

	// No pidfd available: back off adaptively rather than sleeping a fixed
	// 10 ms, which is longer than most tests take to run start to finish.
	long long remaining_us = static_cast<long long>(timeout_ms) * 1000;
	int sleep_us = 50;
	while (remaining_us > 0)
	{
		const pid_t res = waitpid(pid, status, WNOHANG);
		if (res == pid)
		{
			return 1;
		}
		if (res < 0)
		{
			return -1;
		}
		const long long nap_us = sleep_us < remaining_us ? sleep_us : remaining_us;
		usleep(static_cast<useconds_t>(nap_us));
		remaining_us -= nap_us;
		if (sleep_us < 2000)
		{
			sleep_us *= 2;
		}
	}
	return 0;
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
		std::string entry = encoded.substr(start, end == std::string::npos ? std::string::npos : end - start);
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

// The exec-v1 channel is deliberately separate from compiler requests. Every
// string is hex-encoded bytes, including empty arguments and UTF-8 names with
// tabs/newlines. Only this channel pays for resource supervision.
struct ExecRequest
{
	unsigned long long wall_ms, cpu_seconds, rss_bytes, file_bytes;
	std::string input, output, error;
	std::vector<std::string> args;
	EnvOverrides env;
};

bool exec_number(const std::string& text, unsigned long long& value)
{
	value = 0;
	if (text.empty())
		return false;
	for (char c : text)
	{
		if (c < '0' || c > '9' || value > (ULLONG_MAX - (c - '0')) / 10)
			return false;
		value = value * 10 + c - '0';
	}
	return true;
}

bool exec_bytes(const std::string& encoded, std::string& value)
{
	if (encoded.size() % 2 != 0)
		return false;
	value.clear();
	for (size_t i = 0; i < encoded.size(); i += 2)
	{
		unsigned byte = 0;
		for (size_t j = i; j < i + 2; ++j)
		{
			const char c = encoded[j];
			if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
				return false;
			byte = byte * 16 + (c <= '9' ? c - '0' : c - 'a' + 10);
		}
		if (byte == 0)
			return false; // execve strings cannot contain NUL.
		value.push_back(static_cast<char>(byte));
	}
	return true;
}

bool parse_exec_request(const std::string& line, ExecRequest& request)
{
	const auto f = split_tab_fields(line);
	unsigned long long argc = 0, envc = 0;
	if (f.size() < 11 || f[0] != "exec-v1" || !exec_number(f[1], request.wall_ms) ||
		!exec_number(f[2], request.cpu_seconds) || !exec_number(f[3], request.rss_bytes) ||
		!exec_number(f[4], request.file_bytes) || !request.wall_ms || request.wall_ms > INT_MAX ||
		!request.cpu_seconds || request.cpu_seconds > INT_MAX || !request.rss_bytes ||
		request.rss_bytes > (1ULL << 50) || !request.file_bytes || request.file_bytes > (1ULL << 50) ||
		!exec_bytes(f[5], request.input) || !exec_bytes(f[6], request.output) || !exec_bytes(f[7], request.error) ||
		!exec_number(f[8], argc) || argc == 0 || argc > 4096 || f.size() < 10 + argc ||
		!exec_number(f[9 + argc], envc) || envc > 4096 || f.size() != 10 + argc + 2 * envc)
		return false;
	if (request.input.empty() || request.input == "-" || request.output.empty() || request.output == "-" ||
		request.error.empty() || request.error == "-")
		return false;
	for (size_t i = 0; i < argc; ++i)
	{
		std::string value;
		if (!exec_bytes(f[9 + i], value) || (i == 0 && value.empty()))
			return false;
		request.args.push_back(value);
	}
	for (size_t i = 0; i < envc; ++i)
	{
		std::string key, value;
		if (!exec_bytes(f[10 + argc + 2 * i], key) || !exec_bytes(f[11 + argc + 2 * i], value) || key.empty() ||
			key.find('=') != std::string::npos)
			return false;
		request.env.push_back(std::make_pair(key, value));
	}
	return true;
}

bool set_child_limit(int resource, unsigned long long value)
{
	struct rlimit old_limit;
	if (getrlimit(resource, &old_limit) != 0)
		return false;
	struct rlimit limit;
	limit.rlim_cur = static_cast<rlim_t>(value);
	if (old_limit.rlim_max != RLIM_INFINITY && limit.rlim_cur > old_limit.rlim_max)
		limit.rlim_cur = old_limit.rlim_max;
	limit.rlim_max = limit.rlim_cur;
	return setrlimit(resource, &limit) == 0;
}

// Virtual-address ceilings interfere with Wasmtime's guard/reservation ranges
// and Darwin's shared mappings. Sample the resident bytes of the entire child
// process group instead, without changing the engine's address-space policy.
bool group_resident_bytes(pid_t group, unsigned long long& bytes)
{
	bytes = 0;
#ifdef __APPLE__
	std::vector<pid_t> pids(64);
	int count;
	for (;;)
	{
		errno = 0;
		count = proc_listpgrppids(group, pids.data(), static_cast<int>(pids.size() * sizeof(pid_t)));
		if (count < 0 || (count == 0 && errno != 0))
			return false;
		if (static_cast<size_t>(count) < pids.size())
			break;
		pids.resize(pids.size() * 2);
	}
	for (size_t i = 0; i < static_cast<size_t>(count); ++i)
	{
		if (pids[i] <= 0)
			continue;
		struct rusage_info_v0 info;
		if (proc_pid_rusage(pids[i], RUSAGE_INFO_V0, reinterpret_cast<rusage_info_t*>(&info)) != 0)
		{
			if (errno == ESRCH || getpgid(pids[i]) != group)
				continue;
			return false;
		}
		bytes += info.ri_resident_size;
	}
#else
	DIR* dir = opendir("/proc");
	if (dir == NULL)
		return false;
	bool ok = true;
	const long page_size = sysconf(_SC_PAGESIZE);
	for (dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir))
	{
		char* end = NULL;
		const long pid = strtol(entry->d_name, &end, 10);
		if (*entry->d_name == '\0' || *end != '\0' || pid <= 0 || getpgid(static_cast<pid_t>(pid)) != group)
			continue;
		const std::string path = std::string("/proc/") + entry->d_name + "/statm";
		FILE* file = fopen(path.c_str(), "r");
		if (!file)
		{
			ok = ok && (errno == ENOENT || errno == ESRCH);
			continue;
		}
		unsigned long long virtual_pages = 0, resident_pages = 0;
		const int fields = fscanf(file, "%llu %llu", &virtual_pages, &resident_pages);
		fclose(file);
		if (fields == 2 && page_size > 0)
			bytes += resident_pages * static_cast<unsigned long long>(page_size);
		else if (getpgid(static_cast<pid_t>(pid)) == group)
			ok = false;
	}
	closedir(dir);
	return ok;
#endif
	return true;
}

struct ExecOutcome
{
	int status;
	const char* kind;
};

bool captured_limit(const ExecRequest& request)
{
	struct stat info;
	for (const auto& path : { request.output, request.error })
		if (stat(path.c_str(), &info) == 0 && static_cast<unsigned long long>(info.st_size) >= request.file_bytes)
			return true;
	return false;
}

ExecOutcome supervise_exec(pid_t pid, const ExecRequest& request, int setup_fd)
{
	const double deadline = monotonic_ms() + request.wall_ms;
	int status = 0;
	bool started = false;
	for (;;)
	{
		struct rusage usage = {};
		const pid_t waited = wait4(pid, &status, WNOHANG, &usage);
		const int wait_error = errno;
		if (!started)
		{
			// EOF means exec closed the pipe after preparing the capture files.
			// Before that, those paths may still contain a previous request's output.
			char failed;
			const ssize_t setup = read(setup_fd, &failed, 1);
			if (setup > 0)
				return { 125, "setup" };
			if (setup < 0 && errno != EAGAIN && errno != EINTR)
				return { 125, "supervision" };
			started = setup == 0;
		}
		if (waited == pid)
		{
#ifdef __APPLE__
			const unsigned long long peak = usage.ru_maxrss;
#else
			const unsigned long long peak = static_cast<unsigned long long>(usage.ru_maxrss) * 1024;
#endif
			if (peak > request.rss_bytes)
				return { 125, "memory" };
			if ((started && captured_limit(request)) || (WIFSIGNALED(status) && WTERMSIG(status) == SIGXFSZ))
				return { 125, "output" };
			if (WIFSIGNALED(status))
				return { WTERMSIG(status), "signal" };
			return { WEXITSTATUS(status), "exit" };
		}
		if (waited < 0 && wait_error != EINTR)
			return { 125, "supervision" };
		if (monotonic_ms() >= deadline)
			return { 124, "timeout" };
		unsigned long long resident = 0;
		if (!group_resident_bytes(pid, resident))
			return { 125, "supervision" };
		if (resident > request.rss_bytes)
			return { 125, "memory" };
		if (started && captured_limit(request))
			return { 125, "output" };
		// The timer also supervises descendants and intercepted limit signals.
		usleep(10000);
	}
}

ExecOutcome run_exec_request(const ExecRequest& request)
{
	int setup_pipe[2];
	if (pipe(setup_pipe) != 0)
		return { 125, "setup" };
	if (fcntl(setup_pipe[0], F_SETFD, FD_CLOEXEC) < 0 || fcntl(setup_pipe[1], F_SETFD, FD_CLOEXEC) < 0 ||
		fcntl(setup_pipe[0], F_SETFL, O_NONBLOCK) < 0)
	{
		close(setup_pipe[0]);
		close(setup_pipe[1]);
		return { 125, "setup" };
	}
	std::vector<char*> args;
	for (const auto& arg : request.args)
		args.push_back(const_cast<char*>(arg.c_str()));
	args.push_back(NULL);
	fflush(NULL);
	sigset_t blocked, previous;
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGINT);
	sigaddset(&blocked, SIGTERM);
	sigprocmask(SIG_BLOCK, &blocked, &previous);
	const pid_t pid = fork();
	if (pid == 0)
	{
		close(setup_pipe[0]);
		if (setpgid(0, 0) != 0)
			child_setup_failure(125, setup_pipe[1]);
		reset_caught_signal_dispositions();
		sigprocmask(SIG_SETMASK, &previous, NULL);
		if (!set_child_limit(RLIMIT_CPU, request.cpu_seconds) || !set_child_limit(RLIMIT_FSIZE, request.file_bytes) ||
			!set_child_limit(RLIMIT_CORE, 0))
			child_setup_failure(125, setup_pipe[1]);
		execute_child(static_cast<int>(request.args.size()), args.data(), request.input, request.output, request.error,
			false, request.env, 125, setup_pipe[1]);
	}
	close(setup_pipe[1]);
	if (pid > 0)
	{
		setpgid(pid, pid);
		active_child = pid;
	}
	sigprocmask(SIG_SETMASK, &previous, NULL);
	if (pid < 0)
	{
		close(setup_pipe[0]);
		return { 125, "setup" };
	}
	const ExecOutcome outcome = supervise_exec(pid, request, setup_pipe[0]);
	// Always kill the group, even if its leader exited normally or handled TERM.
	kill(-pid, SIGKILL);
	while (waitpid(pid, NULL, 0) < 0 && errno == EINTR)
	{
	}
	active_child = 0;
	close(setup_pipe[0]);
	return outcome;
}

int run_exec_stdin()
{
	install_worker_cancellation();
	std::string line;
	for (;;)
	{
		line.clear();
		bool overlong = false;
		char c;
		while (std::cin.get(c) && c != '\n')
		{
			if (line.size() < 4 * 1024 * 1024)
				line.push_back(c);
			else
				overlong = true;
		}
		if (line.empty() && !std::cin)
			return 0;
		ExecRequest request;
		const ExecOutcome outcome = !overlong && parse_exec_request(line, request) ? run_exec_request(request)
																				   : ExecOutcome{ 125, "protocol" };
		std::cout << "exec-v1\t" << outcome.status << '\t' << outcome.kind << std::endl;
	}
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
	install_worker_cancellation();
	const std::vector<std::string> base_args = batch_base_args(argc, argv);
	const bool run_wrapped_in_process = !batch_force_exec();
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

			status = run_with_timeout(static_cast<int>(args.size()), argv.data(), "-", fields[1], fields[1],
				get_text_batch_timeout_ms(), true);
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
				// Wrapped batch requests reuse the outer worker process but
				// execute each compiler invocation in a fresh child, so no test
				// can observe state left behind by an earlier one: the worker
				// itself never runs compiler code, and the child's copy dies
				// with it. That makes re-entering run_real_main in-process
				// equivalent to execvp for everything except the descriptor,
				// signal and stdio state fork leaves behind, which
				// execute_child normalises before dispatching. Skipping the
				// exec saves the image reload and dynamic-link work per test.
				//
				// The compiler does read the environment (cpp_toolchain.cpp
				// consults CPPGM_HOST_CXX, CXX, CPPGM_STDLIB_FLAGS and
				// CPPGM_OBJECT_ROOT) and does spawn subprocesses (std::system
				// there, popen in preprocessor.cpp). Both stay correct here:
				// execute_child applies the per-request env overrides before
				// dispatching, and every one of those getenv calls sits inside a
				// function rather than a namespace-scope initialiser, so each
				// forked child reads the test's environment rather than a value
				// captured once in the worker. Adding a static-init-time getenv
				// would break that invariant.
				// Set CPPGM_BATCH_EXEC=1 to force the execvp path back.
				status = run_with_timeout(static_cast<int>(argv.size()) - 1, argv.data(), fields[2], fields[0],
					fields[1], get_batch_timeout_override_ms(env_overrides, get_build_batch_timeout_ms()),
					run_wrapped_in_process, env_overrides);
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
			status = run_with_timeout(static_cast<int>(argv.size()) - 1, argv.data(), fields[2], fields[0], fields[1],
				get_batch_timeout_override_ms(env_overrides, get_text_batch_timeout_ms()), true, env_overrides);
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
		if (argc == 2 && std::string(argv[1]) == "--batch-exec-v1")
			return run_exec_stdin();
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
