#include <core/eos.h>
#include <core/eos_internal.h>

typedef struct _os_context {
	/* low address */
	int32u_t edi;
	int32u_t esi;
	int32u_t ebp;
	int32u_t esp;
	int32u_t ebx;
	int32u_t edx;
	int32u_t ecx;
	int32u_t eax;
	int32u_t eflags;
	int32u_t eip;
	/* high address */	
} _os_context_t;

void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
	PRINT("**print_context**\n");
	PRINT("eip  =0x%x\n", ctx->eip);
	PRINT("eflags  =0x%x\n", ctx->eflags);
	PRINT("eax  =0x%x\n", ctx->eax);
	PRINT("ecx  =0x%x\n", ctx->ecx);
	PRINT("edx  =0x%x\n", ctx->edx);
	PRINT("ebx  =0x%x\n", ctx->ebx);
	PRINT("esp  =0x%x\n", ctx->esp);
	PRINT("ebp  =0x%x\n", ctx->ebp);
	PRINT("esi  =0x%x\n", ctx->esi);
	PRINT("edi  =0x%x\n", ctx->edi);
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
//	PRINT("  _os_create_context called\n");
	
	// initializing stack pointer into highest stack boundary
	int32u_t *sp = (int32u_t*) (stack_base + stack_size);

	// saving initial context values into stack
	for(; sp >= stack_base + stack_size - 44 ; sp--){
		// initial arguement, entry, esp, and other NULL values
		if(sp == stack_base + stack_size) *sp = (int32u_t *) arg;
		else if(sp == stack_base + stack_size - 8) *sp = (int32u_t) entry;
		else if(sp == stack_base + stack_size - 32) *sp = sp + 1;
		else *sp = (int32u_t) NULL;
	}
	// adjusting stack pointer into right poisition
	sp++;
	
	return sp;
}

void _os_restore_context(addr_t sp) {
//	PRINT("  _os_restore_context called, sp: 0x%x\n", (int32u_t) sp);

	// moving stack pointer into restoring address
	__asm__ __volatile__("movl %0, %%esp"::"m"(sp));

	// restoring general purpose register in order
	__asm__ __volatile__("popl %%edi"::);
	__asm__ __volatile__("popl %%esi"::);
	__asm__ __volatile__("popl %%ebp"::);
	__asm__ __volatile__("popl %%esp"::);
	__asm__ __volatile__("popl %%ebx"::);
	__asm__ __volatile__("popl %%edx"::);	
	__asm__ __volatile__("popl %%ecx"::);
	__asm__ __volatile__("popl %%eax"::);

	// restoring eflags
	__asm__ __volatile__("popfl"::);

	// return
	__asm__ __volatile__("ret"::);
}

addr_t _os_save_context() {
//	PRINT("  _os_save_context called\n");

	// initializing %eax
	unsigned int a = 0;
	__asm__ __volatile__("movl %0, %%eax"::"m"(a));

	// setting resume_point for %eip
	__asm__ __volatile__("pushl $resume_point"::);
	__asm__ __volatile__("pushfl"::);
	__asm__ __volatile__("pushl %%eax"::);
	__asm__ __volatile__("pushl %%ecx"::);
	__asm__ __volatile__("pushl %%edx"::);
	__asm__ __volatile__("pushl %%ebx"::);
	__asm__ __volatile__("pushl %%esp"::);
	__asm__ __volatile__("pushl %%ebp"::);
	__asm__ __volatile__("pushl %%esi"::);
	__asm__ __volatile__("pushl %%edi"::);

	// setting %eax value
	__asm__ __volatile__("movl %%esp, %%eax"::);

	__asm__ __volatile__("pushl 4(%%ebp)"::);
	__asm__ __volatile__("pushl 0(%%ebp)"::);
	__asm__ __volatile__("movl %%esp, %%ebp"::);

	__asm__ __volatile__("resume_point:"::);
	__asm__ __volatile__("leave"::);
	__asm__ __volatile__("ret"::);
}

