#include <stdio.h>
#include "vm.h"

int main() {
	Stack stack;
	MemBus mem;

	stack.ptr = 0;
	
	printf("Testing stack\n");
	stack_push(&stack, 100);
	stack_push(&stack, 200);
	printf("%i\n", stack_pop(&stack));
	printf("%i\n", stack_pop(&stack));

	stack_push(&stack, 500);
	stack_push(&stack, 400);
	stack_swap(&stack);
	printf("%i\n", stack_pop(&stack));
	printf("%i\n", stack_pop(&stack));


	printf("Testing mem\n");
	mem_set(&mem, 99, 0x10);
	printf("%i\n", mem_get(&mem, 0x10));

	for(Uint16 i = 65; i < 91; i++)
		mem_set(&mem, i, 0x19);

	Uint16 test[] = {1,2,3,4,5};

	mem_load(&mem, test, 0);
	printf("\n%i\n", mem_get(&mem, 0x0)+mem_get(&mem, 0x1));

	int length = sizeof(mem.ram) / sizeof(mem.ram[0]);
	printf("\n%i\n", length);	
	
	return 0;
}
