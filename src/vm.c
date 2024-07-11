#include "vm.h"

/*typedef struct {
	Uint16 data[256];
	Uint16 ptr;
} Stack;*/

void stack_push(Stack* s, Uint16 v){
	if(s->ptr > 255){
		printf("Stack overflow");
	}
	s->data[s->ptr] = v;
	s->ptr++;
}
Uint16 stack_pop(Stack* s){
	if(s->ptr < 0){
		printf("Stack underflow");
		return 0;
	}
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
	Uint16 ram[0xffff], serial[0x100];
} MemBus;*/




void mem_set(MemBus* mem, Uint16 v, Uint16 addr){
	mem->ram[addr] = v;
	/*switch(addr) {
		case 0x19: fputc(mem->ram[0x19], stdout), fflush(stdout); return;
	}*/
}
Uint16 mem_get(MemBus* mem, Uint16 addr){
	return mem->ram[addr];
}
Uint16 mem_load(MemBus* mem, Uint16* data, Uint16 start){
	int len = *(&data + 1) - data;
	for(int i = 0; i < len; i++){
		mem->ram[start+i] = data[i];
	}
}
void dev_set(MemBus* mem, Uint16 v, Uint16 port){
	mem->serial[port] = v;
	switch(port){
		case 0x19: fputc(mem->serial[0x19], stdout); fflush(stdout); return;
	}
	
}
Uint16 dev_get(MemBus* mem, Uint16 port){
	return mem->serial[port];
}

/*typedef struct RatVm {
	Stack ws1, ws2, rs;
	MemBus memory;
	int stack;

	Uint16 PC;

	Uint16 iv_addr;
	Uint16 IM;
	int isInInt;

	int halt;

} RatVm;*/

void ratvm_new(RatVm* vm ,Uint16 int_addr){

	vm->ws1.ptr = 0;
	vm->ws2.ptr = 0;
	vm->rs.ptr = 0;

	vm->stack = 0;

	vm->iv_addr = int_addr;
	vm->isInInt = 0;
	vm->IM = 0xFFFF;

	vm->halt = 0;

	vm->PC = 0;

	
}

Stack* ratvm_get_stack(RatVm* vm){
	if(vm->stack == 0){
		return &vm->ws1;
	}else if(vm->stack == 1){
		return &vm->ws2;
	}
}

void ratvm_save_state(RatVm* vm){
	stack_push(&vm->rs, vm->PC);
}

void ratvm_restore_state(RatVm* vm){
	vm->PC = stack_pop(&vm->rs);
}

Uint16 ratvm_fetch(RatVm* vm){
	Uint16 inst = mem_get(&vm->memory, vm->PC);
	//printf("\nFetch:%d PC:%d\n", inst, vm->PC);
	vm->PC++;
	return inst;
}

Uint16 ratvm_fetch_byte(RatVm* vm){
	Uint16 v1 = ratvm_fetch(vm);
	Uint16 v2 = ratvm_fetch(vm);
	//printf("\n%d %d %d\n",v1, v2, vm->PC);	
	return (v1 << 8) + v2;
}

void ratvm_handle_int(RatVm* vm, Uint16 i){
	Uint16 index = i % 0xf;

	int isUnmasked = ((1 << index) & vm->IM);
	if(isUnmasked != 0) {
		return;
	}

	Uint16 pointer = vm->iv_addr + index;
	Uint16 addr = mem_get(&vm->memory, pointer);

	if(vm->isInInt != 0){
		ratvm_save_state(vm);
	}

	vm->isInInt=1;
	vm->PC=addr;
}

void ratvm_push_stack(RatVm* vm, Uint16 v) {
	stack_push(ratvm_get_stack(vm),v);
}
Uint16 ratvm_pop_stack(RatVm* vm) {
	return stack_pop(ratvm_get_stack(vm));
}
void ratvm_swap_stack(RatVm* vm) {
	stack_swap(ratvm_get_stack(vm));
}
void ratvm_rot_stack(RatVm* vm) {
	stack_rot(ratvm_get_stack(vm));
}



void ratvm_set_mem(RatVm* vm, Uint16 a, Uint16 v){
	//printf("\nval:%d addr: %d\n", v,a);
	mem_set(&vm->memory,v,a);
}
Uint16 ratvm_get_mem(RatVm* vm, Uint16 addr){
	return mem_get(&vm->memory,addr);
}

void ratvm_set_dev(RatVm* vm, Uint16 port, Uint16 v){
	dev_set(&vm->memory,v,port);
}
Uint16 ratvm_get_dev(RatVm* vm, Uint16 port){
	return dev_get(&vm->memory,port);
}

void ratvm_exe(RatVm* vm, Uint16 inst){
	//printf("%d pc:%d\n ", inst, vm->PC);
	switch(inst){
	//Pushes literal value (msb, lsb) onto WS
	//PSH msb lsb
	case PUSH_LIT:
		Uint16 val= ratvm_fetch_byte(vm);
		ratvm_push_stack(vm,val);
		break;
	//Pushes value of address onto WS
	//PSH msb lsb
	case PUSH_ADDR:	
		Uint16 addr = ratvm_fetch_byte(vm);
		ratvm_push_stack(vm,mem_get(&vm->memory, addr));
		break;
	//Removes top value at WS
	//POP
	case POP:
		ratvm_pop_stack(vm);
		break;
	//Switches current WS between WS 1 and 2 (WS 1 is the default on boot)
	//SWH
	case SWH:
		if(vm->stack == 1){
			vm->stack = 0;
		}else{
			vm->stack = 1;
		}
		break;
	//Stores top of WS at memory address a
	//STO msb lsb
	case STO:
		Uint16 ad = ratvm_fetch_byte(vm);
		Uint16 vl = ratvm_pop_stack(vm);
		ratvm_set_mem(vm, ad, vl);
		break;
	//Stores top of WS at memory address a plus the 2nd value of the WS
	//STR msb lsb
	case STR:
			
		Uint16 a = ratvm_fetch_byte(vm);
		Uint16 v = ratvm_pop_stack(vm);
		Uint16 o = ratvm_pop_stack(vm);
		ratvm_set_mem(vm,a+o,v);
		break;
	//Writes top of WS to serial port a
	//DST a
	case DST:
		Uint16 dev_st = ratvm_pop_stack(vm);
		Uint16 dev_st_p = ratvm_fetch_byte(vm);
		ratvm_set_dev(vm,dev_st_p,dev_st);
		break;
	//Pushes value of serial port a to top of WS
	//DRE a
	case DRE:
		Uint16 dev_re_p = ratvm_fetch_byte(vm);
		ratvm_push_stack(vm,ratvm_get_dev(vm,dev_re_p));
		break;


	//Swaps last 2 values on WS
	//SWP
	case SWP:
		ratvm_swap_stack(vm);
		break;
	//Puts 3rd value on WS on top
	//ROT
	case ROT:
		ratvm_rot_stack(vm);
		break;
	//Dupulcates top of WS
	//DUP
	case DUP:
		Uint16 dupl = ratvm_pop_stack(vm);
		ratvm_push_stack(vm, dupl);
		ratvm_push_stack(vm, dupl);
		break;
	//Dupulcates first 2 values of of WS
	//DUP2
	case DUP2:
		Uint16 adupl = ratvm_pop_stack(vm);
		Uint16 bdupl = ratvm_pop_stack(vm);
		
		ratvm_push_stack(vm, bdupl);
		ratvm_push_stack(vm, adupl);
		ratvm_push_stack(vm, bdupl);
		ratvm_push_stack(vm, adupl);
		break;
	//Saves top of WC to Interrupt mask
	//SIM
	case SIM:	
		vm->IM = ratvm_pop_stack(vm);
		break;
	//Loads interrupt mask to top of WS
	//LIM
	case LIM:
		ratvm_push_stack(vm,vm->IM);
		break;
	//Stores memory address plus the current top of the WS at top of WS
	//GTR msb lsb
	case GTR:
		Uint16 base = ratvm_fetch_byte(vm);
		
		Uint16 offset = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,ratvm_get_mem(vm,a+o));
		
		break;
	//Pops and Adds the last 2 numbers on the WS and then pushes the result onto WS
	//ADD
	case ADD:
		Uint16 add1 = ratvm_pop_stack(vm);
		Uint16 add2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,add1+add2);
		break;
	//Pops and subtracts the last 2 numbers on the WS and then pushes the result onto WS
	//SUB
	case SUB:
		Uint16 sub1 = ratvm_pop_stack(vm);
		Uint16 sub2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,sub1+sub2);	
		break;
	//Pops and multiplies the last 2 numbers on the WS and then pushes the result onto WS
	//MUL
	case MUL:
		Uint16 mul1 = ratvm_pop_stack(vm);
		Uint16 mul2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,mul1*mul2);	
		break;
	//Pops and divides the last 2 numbers on the WS and then pushes the result onto WS
	//DIV
	case DIV:
		Uint16 div1 = ratvm_pop_stack(vm);
		Uint16 div2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,div1/div2);	
		
		break;
	//Pops and applies bitwise and to the last 2 numbers on the WS and then pushes the result onto WS
	//AND
	case AND:
		Uint16 and1 = ratvm_pop_stack(vm);
		Uint16 and2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,and1&and2);	
		
		break;
	//Pops and applies bitwise or to the last 2 numbers on the WS and then pushes the result onto WS
	//OR
	case OR:
		Uint16 or1 = ratvm_pop_stack(vm);
		Uint16 or2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,or1|or2);	
		
		break;
	//Pops and applies bitwise XOR to the last 2 numbers on the WS and then pushes the result onto WS
	//XOR
	case XOR:
		Uint16 xor1 = ratvm_pop_stack(vm);
		Uint16 xor2 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,xor1^xor2);	
		
		break;
	//Applies not to top of stack
	//NOT
	case NOT:
		Uint16 not1 = ratvm_pop_stack(vm);
		
		ratvm_push_stack(vm,~not1);
		break;
	//Left Shifts the top of WS by literal v
	//LFS msb lsb
	case LFS:
		Uint16 lfs1 = ratvm_pop_stack(vm);
		Uint16 lfsv = ratvm_fetch_byte(vm);
		ratvm_push_stack(vm,lfs1<<lfsv);
		break;
	//Right Shifts the top of WS by literal v
	//RTS msb lsb
	case RTS:
		Uint16 rts1 = ratvm_pop_stack(vm);
		Uint16 rtsv = ratvm_fetch_byte(vm);
		ratvm_push_stack(vm,lfs1<<lfsv);	
		break;
	//Increases the value on top of the WS by 1
	//INC
	case INC:
		Uint16 inc1 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,inc1+1);
		break;
	//Decreases the value on top of the WS by 1
	//DEC
	case DEC:
		Uint16 dec1 = ratvm_pop_stack(vm);
		ratvm_push_stack(vm,dec1-1);	
		break;
	//Saves the state to the RS and sets PC to the address at the top of the WS
	//CAL
	case CAL:
		ratvm_save_state(vm);
		vm->PC = ratvm_pop_stack(vm);
		break;
	//Restores the state from the RS
	//RET
	case RET:
		ratvm_restore_state(vm);
		break;
	//Sets PC to the top of WS
	//JMP
	case JMP:
		Uint16 jmpaddr = ratvm_pop_stack(vm);
		vm->PC = jmpaddr;
		break;
	//Sets PC to literal value
	//JML msb lsb
	case JML:
		Uint16 jmpl = ratvm_fetch_byte(vm);
		vm->PC = jmpl;
		break;
	//Jumps to a if top of WS is equal to 1
	//JPC msb lsb
	case JPC:
		Uint16 jmpc = ratvm_pop_stack(vm);
		Uint16 jmpd = ratvm_fetch_byte(vm);
		//console.log(a)
		if (jmpc==1) {
			vm->PC = jmpd;
		}
		
		break;
	//Trigger a interrupt
	//INT a
	case INT:
		Uint16 intaddr = ratvm_fetch_byte(vm);
		ratvm_handle_int(vm, intaddr);
		break;
	//Returns from interrupt
	//RFI
	case RFI:
		vm->isInInt = 0;
		ratvm_restore_state(vm);
		break;
	//Pushes 1 if the 1st and 2nd values on the WS are equal. else, it pushes 0
	//EQU
	case EQU:
		Uint16 eqa = ratvm_pop_stack(vm);
		Uint16 eqb = ratvm_pop_stack(vm);
		if (eqa==eqb) {
			ratvm_push_stack(vm,1);
		}else{
			ratvm_push_stack(vm,0);
		}
		break;
	//Pushes 1 if the 1st and 2nd values on the WS are not equal. else, it pushes 0
	//NEQ
	case NEQ:
		Uint16 neqa = ratvm_pop_stack(vm);
		Uint16 neqb = ratvm_pop_stack(vm);
		if (neqa!=neqb) {
			ratvm_push_stack(vm,1);
		}else{
			ratvm_push_stack(vm,0);
		}	
		break;
	//Pushes 1 if the 1st value of WS is greater than the second value on WS. else, it pushes 0
	//GTH
	case GTH:
		Uint16 gta = ratvm_pop_stack(vm);
		Uint16 gtb = ratvm_pop_stack(vm);
		if (gta>gtb) {
			ratvm_push_stack(vm,1);
		}else{
			ratvm_push_stack(vm,0);
		}
		break;
	//Pushes 1 if the 1st value of WS is less than the second value on WS. else, it pushes 0
	//LTH
	case LTH:
		Uint16 lta = ratvm_pop_stack(vm);
		Uint16 ltb = ratvm_pop_stack(vm);
		if (lta<ltb) {
			ratvm_push_stack(vm,1);
		}else{
			ratvm_push_stack(vm,0);
		}
		
		break;

	case HLT:
		vm->halt = 1;
		break;
	default:
		break;
	
	
	}
}

void ratvm_step(RatVm* vm){
	ratvm_exe(vm, ratvm_fetch(vm));
}

void ratvm_run(RatVm* vm){
	
	while(vm->halt==0){
		//for(int dd = 5; dd>=0; dd--){
			ratvm_step(vm);
		//}
	}
}

