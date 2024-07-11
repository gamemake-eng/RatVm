#include <stdio.h>

#pragma once

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;

#define PUSH_LIT 0x10
#define PUSH_ADDR 0x11
#define POP 0x12
//Command can be at 0x13
#define SWH 0x14
#define STO 0x15
#define STR 0x16
#define SWP 0x17
#define ROT 0x18
#define DUP 0x19
#define OVR 0x1A
#define DUP2 0x1B
#define LIM 0x1C
#define SIM 0x1D
#define GTR 0x1E
#define ADD 0x20
#define SUB 0x21
#define DIV 0x22
#define MUL 0x23
#define INC 0x24
#define DEC 0x25
#define AND 0x26
#define OR  0x27
#define XOR 0x28
#define LFS 0x29
#define RTS 0x2a
#define NOT 0x2B
#define CAL 0x30
#define RET 0x31
#define JMP 0x32
#define JPC 0x33
#define INT 0x34
#define RFI 0x35
#define JML 0x36
#define EQU 0x40
#define NEQ 0x41
#define GTH 0x42
#define LTH 0x43
#define DST 0x44
#define DRE 0x45
#define HLT 0xFF

typedef struct Stack {
	Uint16 data[256];
	Uint16 ptr;
} Stack;

void stack_push(Stack* s, Uint16 v);
Uint16 stack_pop(Stack* s);
void stack_rot(Stack* s);
void stack_swap(Stack* s);

typedef struct MemBus {
	Uint16 ram[0xffff], serial[0x100];
} MemBus;

void mem_set(MemBus* mem, Uint16 v, Uint16 addr);
Uint16 mem_get(MemBus* mem, Uint16 addr);
Uint16 mem_load(MemBus* mem, Uint16* data, Uint16 start);
void dev_set(MemBus* mem, Uint16 v, Uint16 port);
Uint16 dev_get(MemBus* mem, Uint16 port);

typedef struct RatVm {
	Stack ws1, ws2, rs;
	MemBus memory;
	int stack;

	Uint16 PC;

	Uint16 iv_addr;
	Uint16 IM;
	int isInInt;

	int halt;

} RatVm;

void ratvm_new(RatVm* vm, Uint16 int_addr);
Stack* ratvm_get_stack(RatVm* vm);
void ratvm_save_state(RatVm* vm);
void ratvm_restore_state(RatVm* vm);
Uint16 ratvm_fetch(RatVm* vm);
Uint16 ratvm_fetch_byte(RatVm* vm);
void ratvm_handle_int(RatVm* vm, Uint16 i);
void ratvm_exe(RatVm* vm, Uint16 inst);
void ratvm_step(RatVm *vm);
void ratvm_run(RatVm *vm);
