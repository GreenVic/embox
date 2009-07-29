/**
 * \file testmem.c
 * \date Jul 24, 2009
 * \author afomin
 * \details
 */

#include "types.h"
#include "conio.h"
#include "shell.h"
#include "testmem.h"

static char mem_keys[] = {
#include "mem_keys.inc"
		};

void print_memory(WORD *addr, long int amount) {
	int i = 0, j;
	addr = (WORD *) ((WORD) addr & 0xFFFFFFFC);
	while (i < amount) {
		if (0 == (i % 4)) {
			printf("0x%08x:\t", (int) (addr + i));
		}
		printf("0x%08x\t", *(addr + i));
		if (3 == (i % 4)) {
			printf("\n");
		}
		i++;
	}

}
static void testmem_print_help(void) {
	printf(
#include "mem_help.inc"
	);
}


void test_memory(WORD *addr, long int amount) {
	int i = 0, percent_complited = 0;
	WORD old_value;

	printf("Memory test .");
	addr = (WORD *) ((WORD) addr & 0xFFFFFFFC);
	while (i++ < amount) {
		old_value = *addr;
		*addr = 0x12345678;
		if (0x12345678 != *addr) {
			printf(" FALSE!\nAt address: 0x%x", addr);
			return;
		}
		*addr = old_value;
		addr++;
		while ((i * 100 > amount * percent_complited)) {
			if (percent_complited++ % 2) {
				printf(".");
			}
		}
	}
	printf("OK\n");
}

typedef void TEST_MEM_FUNC(WORD *addr, long int amount);

int mem_shell_handler(int argsc, char **argsv) {
	SHELL_KEY keys[MAX_SHELL_KEYS];
	int i;
	char *key_value;
	bool is_correct = TRUE;
	TEST_MEM_FUNC *test_mem_func;
	WORD *address = (WORD *) 0x70000000L;
	long int amount = 1000000L;
	int keys_amount = parse_arg("mem", argsc, argsv, mem_keys,
			sizeof(mem_keys), keys);

	if (keys_amount <= 0) {
		mem_print_help();
		return -1;
	}

	if (get_key('t', keys, keys_amount, &key_value)) {
		// In test mode
		switch(key_value) {
		case "runzero":
			test_mem_func = &test_memory_run0;
			break;
		case "runone":
			test_mem_func = &test_memory_run1;
			break;
		case "loop":
			amount = 0;
			test_mem_func = &test_memory_loop;
			break;
		default:
			LOG_ERROR("mem: test name expected.\n");
			mem_print_help();
		}

	} else {
		// In read memory mode

		address = 0x0;
		amount = 50;
		test_mem_func = &print_memory;
	}

	if (get_key('h', keys, keys_amount, &key_value)) {
		mem_print_help();
		return 0;
	}

	if (get_key('a', keys, keys_amount, &key_value)) {
		if ((key_value == NULL) || // addr empty
				((!sscanf(key_value, "0x%x", &address)) // addr not in hex
				&& (!sscanf(key_value, "%d", (int *) &address)))) { // addr not in decimal
			LOG_ERROR("mem: address value expected.\n");
			mem_print_help();
			return -1;
		} else {
			address = key_value;
		}

	}

	if (get_key('n', keys, keys_amount, &key_value)) {
		if ((key_value == NULL) || // amount empty
				((!sscanf(key_value, "0x%x", &address)) // amount not in hex
				&& (!sscanf(key_value, "%d", (int *) &address)))) { // amount not in decimal
			LOG_ERROR("mem: amount value expected.\n");
			mem_print_help();
			return -1;
		} else {
			amount = key_value;
		}

	}



	cache_disable();
	(*test_mem_func)(address, amount);
	cache_enable();
	return 0;
}
