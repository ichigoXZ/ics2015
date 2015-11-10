#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute () {
			cpu.eip +=op_src->val;
#if DATA_BYTE == 2 
		cpu.eip = cpu.eip & 0x0000ffff;
#endif

	print_asm_template1();
}

make_instr_helper(i)
make_instr_helper(rm) 

#include "cpu/exec/template-end.h"
