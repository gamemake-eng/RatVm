#include "vm.h"

/*typedef struct {
	Uint16 data[256];
	Uint16 ptr;
} Stack;*/

void stack_push(Stack* s, Uint16 v){
	s->data[s->ptr] = v;
	s->ptr++;
}
Uint16 stack_pop(Stack* s){
	s->ptr--;
	return s->data[s->ptr];
}
void stack_rot(Stack* s){
	Uint16 a = stack_pop(s);
	Uint16 b = stack_pop(s);
	Uint16 c = stack_pop(s);

	stack_push(s,b);
	stack_push(s,a);
	stack_push(s,c);
}
void stack_swap(Stack* s){
	Uint16 a = stack_pop(s);
	Uint16 b = stack_pop(s);

	stack_push(s,a);
	stack_push(s,b);

}

/*typedef struct {
	Uint16 ram[0xffff];
} MemBus;*/

void mem_set(MemBus* mem, Uint16 v, Uint16 addr){
	mem->ram[addr] = v;
	switch(addr) {
		case 0x19: fputc(mem->ram[0x19], stdout), fflush(stdout); return;
	}
}
Uint16 mem_get(MemBus* mem, Uint16 addr){
	return mem->ram[addr];
}
Uint16 mem_load(MemBus* mem, Uint16* data, Uint16 start){
	int len = sizeof(data) / sizeof(data[0]);
	for(int i = 0; i < len; i++){
		mem->ram[start+i] = data[i];
	}
}

