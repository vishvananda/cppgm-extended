#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <wasmtime.h>

namespace {

using Clock = std::chrono::steady_clock;

double elapsed(Clock::time_point start)
{
	return std::chrono::duration<double>(Clock::now() - start).count();
}

int timed_command(char** command)
{
	const auto start = Clock::now();
	const pid_t child = fork();
	if (child == 0)
	{
		execvp(command[0], command);
		_exit(127);
	}
	if (child < 0)
		return 1;
	int status;
	rusage resources;
	pid_t waited;
	do
		waited = wait4(child, &status, 0, &resources);
	while (waited < 0 && errno == EINTR);
	if (waited < 0)
		return 1;
	const int exit = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
	const double cpu = resources.ru_utime.tv_sec + resources.ru_utime.tv_usec / 1e6 + resources.ru_stime.tv_sec +
		resources.ru_stime.tv_usec / 1e6;
#ifdef __APPLE__
	resources.ru_maxrss /= 1024;
#endif
	std::cout << std::setprecision(12) << "{\"seconds\":" << elapsed(start) << ",\"cpu_seconds\":" << cpu
			  << ",\"max_rss_kib\":" << resources.ru_maxrss << ",\"exit\":" << exit << "}\n";
	return exit == 0 ? 0 : 1;
}

void check(wasmtime_error_t* error, wasm_trap_t* trap)
{
	if (!error && !trap)
		return;
	wasm_byte_vec_t message;
	if (error)
		wasmtime_error_message(error, &message);
	else
		wasm_trap_message(trap, &message);
	std::cerr.write(message.data, message.size);
	std::cerr << '\n';
	wasm_byte_vec_delete(&message);
	std::exit(1);
}

struct Instance
{
	wasmtime_store_t* store;
	wasmtime_instance_t instance;
	double seconds;
	wasmtime_context_t* context() const { return wasmtime_store_context(store); }
};

Instance instantiate(wasm_engine_t* engine, wasmtime_module_t* module)
{
	Instance result;
	result.store = wasmtime_store_new(engine, nullptr, nullptr);
	wasmtime_store_limiter(result.store, 268435456, 100000, 16, 16, 16);
	wasm_trap_t* trap = nullptr;
	const auto start = Clock::now();
	wasmtime_error_t* error = wasmtime_instance_new(result.context(), module, nullptr, 0, &result.instance, &trap);
	result.seconds = elapsed(start);
	check(error, trap);
	return result;
}

uint32_t call(const Instance& instance, const char* name, std::initializer_list<uint32_t> arguments, bool returns)
{
	wasmtime_extern_t function;
	if (!wasmtime_instance_export_get(instance.context(), &instance.instance, name, std::strlen(name), &function) ||
		function.kind != WASMTIME_EXTERN_FUNC)
	{
		std::cerr << "Missing function export: " << name << '\n';
		std::exit(1);
	}
	std::vector<wasmtime_val_t> args(arguments.size());
	size_t index = 0;
	for (uint32_t value : arguments)
	{
		args[index].kind = WASMTIME_I32;
		args[index++].of.i32 = static_cast<int32_t>(value);
	}
	wasmtime_val_t result;
	wasm_trap_t* trap = nullptr;
	wasmtime_error_t* error = wasmtime_func_call(instance.context(), &function.of.func, args.data(), args.size(),
		returns ? &result : nullptr, returns ? 1 : 0, &trap);
	wasmtime_extern_delete(&function);
	check(error, trap);
	if (returns && result.kind != WASMTIME_I32)
	{
		std::cerr << "Benchmark result is not i32\n";
		std::exit(1);
	}
	return returns ? static_cast<uint32_t>(result.of.i32) : 0;
}

} // namespace

int main(int argc, char** argv)
{
	if (argc >= 3 && std::strcmp(argv[1], "--time") == 0)
		return timed_command(argv + 2);
	if (argc != 3)
	{
		std::cerr << "usage: wasm_benchmark_runner module-A module-B < requests\n";
		return 1;
	}
	auto* config = wasm_config_new();
	wasmtime_config_strategy_set(config, WASMTIME_STRATEGY_CRANELIFT);
	wasmtime_config_cranelift_opt_level_set(config, WASMTIME_OPT_LEVEL_SPEED);
	wasmtime_config_parallel_compilation_set(config, false);
	wasmtime_config_wasm_simd_set(config, true);
	auto* engine = wasm_engine_new_with_config(config);
	std::vector<wasmtime_module_t*> modules;
	std::vector<Instance> instances;
	std::cout << std::setprecision(12);
	for (int i = 1; i < argc; ++i)
	{
		std::ifstream file(argv[i], std::ios::binary);
		const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		if (bytes.empty())
			return 1;
		wasmtime_module_t* module = nullptr;
		const auto start = Clock::now();
		check(wasmtime_module_new(engine, bytes.data(), bytes.size(), &module), nullptr);
		std::cout << "{\"event\":\"compile\",\"slot\":" << i - 1 << ",\"seconds\":" << elapsed(start) << "}\n";
		modules.push_back(module);
		instances.push_back(instantiate(engine, module));
	}
	unsigned slot, kind, repeats, seed;
	while (std::cin >> slot >> kind >> repeats >> seed)
	{
		if (slot >= instances.size() || kind > 4)
			return 1;
		double seconds;
		double setup_seconds = 0;
		uint32_t result = 0;
		if (kind == 4)
		{
			if (repeats == 0)
				return 1;
			seconds = 0;
			for (unsigned i = 0; i < repeats; ++i)
			{
				const Instance instance = instantiate(engine, modules[slot]);
				seconds += instance.seconds;
				wasmtime_store_delete(instance.store);
			}
			seconds /= repeats;
		}
		else
		{
			const Instance& instance = instances[slot];
			call(instance, "prepare", { seed }, false);
			call(instance, "run", { kind, 1, seed }, true);
			const auto setup = Clock::now();
			call(instance, "prepare", { seed }, false);
			setup_seconds = elapsed(setup);
			const auto start = Clock::now();
			result = call(instance, "run", { kind, repeats, seed }, true);
			seconds = elapsed(start);
		}
		std::cout << "{\"event\":\"run\",\"slot\":" << slot << ",\"kind\":" << kind << ",\"repeats\":" << repeats
				  << ",\"seed\":" << seed << ",\"seconds\":" << seconds << ",\"setup_seconds\":" << setup_seconds
				  << ",\"result\":" << result << "}\n";
	}
	if (!std::cin.eof())
		return 1;
	for (const Instance& instance : instances)
		wasmtime_store_delete(instance.store);
	for (auto* module : modules)
		wasmtime_module_delete(module);
	wasm_engine_delete(engine);
	return 0;
}
