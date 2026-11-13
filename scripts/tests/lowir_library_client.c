#include "backend/lowir_c.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(CppgmValue) == 8, "value ABI");
_Static_assert(sizeof(CppgmInstruction) == 64, "instruction ABI");
_Static_assert(sizeof(CppgmDataItem) == 40, "data ABI");

static CppgmValue value(uint32_t kind, uint32_t index)
{
	CppgmValue result = { kind, index };
	return result;
}

static void check(CppgmLowir* module, int result)
{
	if (!result)
	{
		fprintf(stderr, "%s\n", cppgm_lowir_error(module));
		exit(1);
	}
}

static CppgmInstruction instruction(uint32_t block, uint8_t op, uint32_t result, CppgmType type)
{
	CppgmInstruction inst = { 0 };
	inst.block = block;
	inst.op = op;
	inst.result = result;
	inst.type = type;
	return inst;
}

static int finish(CppgmLowir* module, char** argv)
{
	check(module, cppgm_lowir_write(module, argv[2]));
	check(module, cppgm_lowir_compile(module, (uint32_t)atoi(argv[3]), 0));
	const uint8_t* bytes = cppgm_lowir_object_data(module);
	uint64_t size = cppgm_lowir_object_size(module);
	FILE* file = fopen(argv[1], "wb");
	if (file == NULL || size == 0 || fwrite(bytes, 1, size, file) != size || fclose(file))
		return 5;
	if (cppgm_lowir_compile(module, 0, 0) || cppgm_lowir_error(module)[0] == 0)
		return 6;
	cppgm_lowir_destroy(module);
	cppgm_lowir_destroy(NULL);
	return 0;
}

static int compile_case(int argc, char** argv)
{
	if (argc != 5)
		return 2;
	CppgmLowir* module = cppgm_lowir_create(CPPGM_LOWIR_ABI_VERSION, strcmp(argv[4], "macos") == 0 ? 1 : 0);
	check(module, module != NULL);
	if (cppgm_lowir_create(CPPGM_LOWIR_ABI_VERSION + 1, 0) != NULL || cppgm_lowir_create(1, 99) != NULL)
		return 3;
	if (strcmp(argv[4], "entry") == 0)
	{
		uint32_t main_symbol = cppgm_lowir_symbol(module, "main", 4);
		uint32_t main_signature = cppgm_lowir_signature(module, CPPGM_TYPE_I32, NULL, 0, 0);
		check(module, cppgm_lowir_function(module, main_symbol, main_signature, 2, 1));
		CppgmInstruction ret = instruction(0, CPPGM_OP_RETURN, CPPGM_NO_ID, CPPGM_TYPE_I32);
		ret.operands[0] = value(CPPGM_VALUE_INTEGER, 0);
		CppgmLiteral zero = { 0, 0 };
		uint32_t layout = 0;
		CppgmBody body = { 0, 0, 1, 1, 0, 1, NULL, &layout, &ret, NULL, &zero };
		check(module, cppgm_lowir_body(module, main_symbol, &body));
		return finish(module, argv);
	}
	uint32_t bias = cppgm_lowir_symbol(module, "bias", 4);
	CppgmDataItem data = { 0 };
	data.type = CPPGM_TYPE_I64;
	data.bits.low = 7;
	check(module, cppgm_lowir_global(module, bias, 0, 2, 1, &data, 1, 0));
	uint32_t host = cppgm_lowir_symbol(module, "host_adjust", 11);
	uint32_t compute = cppgm_lowir_symbol(module, "compute", 7);
	CppgmType parameter = CPPGM_TYPE_I64;
	uint32_t sig = cppgm_lowir_signature(module, CPPGM_TYPE_I64, &parameter, 1, 0);
	check(module, sig != CPPGM_NO_ID);
	check(module, cppgm_lowir_function(module, host, sig, 2, 0));
	check(module, cppgm_lowir_function(module, compute, sig, 2, 0));
	check(module, cppgm_lowir_function(module, compute, sig, 2, 1));
	uint32_t layout[] = { 0, 1, 2, 3 };
	CppgmLiteral literals[] = { { 0, 0 }, { 1, 0 } };
	CppgmValue operands[] = { { CPPGM_VALUE_BLOCK, 0 }, { CPPGM_VALUE_INTEGER, 0 }, { CPPGM_VALUE_BLOCK, 2 },
		{ CPPGM_VALUE_TEMP, 5 }, { CPPGM_VALUE_BLOCK, 0 }, { CPPGM_VALUE_TEMP, 1 }, { CPPGM_VALUE_BLOCK, 2 },
		{ CPPGM_VALUE_TEMP, 4 }, { CPPGM_VALUE_TEMP, 3 } };
	CppgmInstruction code[11];
	code[0] = instruction(0, CPPGM_OP_LOAD, 1, CPPGM_TYPE_I64);
	code[0].operands[0] = value(CPPGM_VALUE_SYMBOL, bias);
	code[1] = instruction(0, CPPGM_OP_JUMP, CPPGM_NO_ID, 0);
	code[1].operands[0] = value(CPPGM_VALUE_BLOCK, 1);
	code[2] = instruction(1, CPPGM_OP_PHI, 2, CPPGM_TYPE_I64);
	code[2].extra_count = 4;
	code[3] = instruction(1, CPPGM_OP_PHI, 3, CPPGM_TYPE_I64);
	code[3].extra_first = 4;
	code[3].extra_count = 4;
	code[4] = instruction(1, CPPGM_OP_COMPARE, 6, CPPGM_TYPE_I64);
	code[4].sub = CPPGM_CMP_LT;
	code[4].operands[0] = value(CPPGM_VALUE_TEMP, 2);
	code[4].operands[1] = value(CPPGM_VALUE_TEMP, 0);
	code[5] = instruction(1, CPPGM_OP_BRANCH, CPPGM_NO_ID, 0);
	code[5].operands[0] = value(CPPGM_VALUE_TEMP, 6);
	code[5].operands[1] = value(CPPGM_VALUE_BLOCK, 2);
	code[5].operands[2] = value(CPPGM_VALUE_BLOCK, 3);
	code[6] = instruction(2, CPPGM_OP_BINARY, 4, CPPGM_TYPE_I64);
	code[6].sub = CPPGM_BINARY_ADD;
	code[6].operands[0] = value(CPPGM_VALUE_TEMP, 3);
	code[6].operands[1] = value(CPPGM_VALUE_TEMP, 2);
	code[7] = instruction(2, CPPGM_OP_BINARY, 5, CPPGM_TYPE_I64);
	code[7].sub = CPPGM_BINARY_ADD;
	code[7].operands[0] = value(CPPGM_VALUE_TEMP, 2);
	code[7].operands[1] = value(CPPGM_VALUE_INTEGER, 1);
	code[8] = instruction(2, CPPGM_OP_JUMP, CPPGM_NO_ID, 0);
	code[8].operands[0] = value(CPPGM_VALUE_BLOCK, 1);
	code[9] = instruction(3, CPPGM_OP_CALL, 7, CPPGM_TYPE_I64);
	code[9].operands[0] = value(CPPGM_VALUE_SYMBOL, host);
	code[9].extra_first = 8;
	code[9].extra_count = 1;
	code[10] = instruction(3, CPPGM_OP_RETURN, CPPGM_NO_ID, CPPGM_TYPE_I64);
	code[10].operands[0] = value(CPPGM_VALUE_TEMP, 7);
	CppgmBody body = { 8, 0, 4, 11, 9, 2, NULL, layout, code, operands, literals };
	if (strcmp(argv[4], "bad-operand") == 0)
		code[10].operands[0].index = 99;
	else if (strcmp(argv[4], "bad-layout") == 0)
		layout[3] = 0;
	else if (strcmp(argv[4], "bad-result") == 0)
		code[7].result = 4;
	else if (strcmp(argv[4], "bad-extra") == 0)
		code[9].extra_count = UINT32_MAX;
	else if (strcmp(argv[4], "bad-type") == 0)
		code[0].type = 255;
	else if (strcmp(argv[4], "bad-kind") == 0)
		code[10].operands[0].kind = 255;
	else if (strcmp(argv[4], "bad-float-kind") == 0)
		code[10].operands[0].kind = CPPGM_VALUE_FLOAT | (255u << 8);
	else if (strcmp(argv[4], "bad-conversion") == 0)
	{
		code[0].op = CPPGM_OP_CONVERT;
		code[0].immediate = 255;
	}
	int result = cppgm_lowir_body(module, compute, &body);
	if (result)
		result = cppgm_lowir_validate(module);
	if (strncmp(argv[4], "bad-", 4) == 0)
	{
		if (result || cppgm_lowir_error(module)[0] == 0 || cppgm_lowir_compile(module, 0, 0))
			return 4;
		cppgm_lowir_destroy(module);
		return 0;
	}
	check(module, result);
	return finish(module, argv);
}

struct Worker
{
	char object[1024];
	char text[1024];
	int status;
};

static void* worker(void* opaque)
{
	struct Worker* work = (struct Worker*)opaque;
	char* args[] = { "client", work->object, work->text, "3", "valid" };
	work->status = compile_case(5, args);
	return NULL;
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "parallel") == 0)
	{
		pthread_t threads[8];
		struct Worker workers[8];
		for (int i = 0; i < 8; ++i)
		{
			snprintf(workers[i].object, sizeof workers[i].object, "%s/thread-%d.o", argv[2], i);
			snprintf(workers[i].text, sizeof workers[i].text, "%s/thread-%d.lowir", argv[2], i);
			workers[i].status = -1;
			if (pthread_create(&threads[i], NULL, worker, &workers[i]))
				return 7;
		}
		for (int i = 0; i < 8; ++i)
		{
			if (pthread_join(threads[i], NULL) || workers[i].status)
				return 8;
		}
		return 0;
	}
	return compile_case(argc, argv);
}
