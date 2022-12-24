#include "sim.h"


static void hook_block(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
	printf(">BB: 0x%"PRIx64 ", -> 0x%x\n", address, size);
}

static void hook_code(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
	printf("> exec: 0x%"PRIx64 ", -> 0x%x\n", address, size);
}

#define SPARC_CODE "\x86\x00\x40\x02" // add %g1, %g2, %g3;

static void test_sparc(void)
{
	uc_engine *uc;
	uc_err err;
	uc_hook trace1, trace2;

	const uint64_t ADDRESS = 0x10000;

	int g1 = 0x1230;     // G1 register
	int g2 = 0x6789;     // G2 register
	int g3 = 0x5555;     // G3 register

	// Initialize emulator in Sparc mode
	err = uc_open(UC_ARCH_SPARC, (uc_mode)(UC_MODE_SPARC32 | UC_MODE_BIG_ENDIAN), &uc);
	if (err) {
		printf("%u (%s)\n",
			err, uc_strerror(err));
		return;
	}

	// map 2MB memory for this emulation
	uc_mem_map(uc, ADDRESS, 2 * 1024 * 1024, UC_PROT_ALL);

	// write machine code to be emulated to memory
	uc_mem_write(uc, ADDRESS, SPARC_CODE, sizeof(SPARC_CODE) - 1);

	// initialize machine registers
	uc_reg_write(uc, UC_SPARC_REG_G1, &g1);
	uc_reg_write(uc, UC_SPARC_REG_G2, &g2);
	uc_reg_write(uc, UC_SPARC_REG_G3, &g3);

	// tracing all basic blocks with customized callback
	uc_hook_add(uc, &trace1, UC_HOOK_BLOCK, hook_block, NULL, 1, 0);

	// tracing all instructions with customized callback
	uc_hook_add(uc, &trace2, UC_HOOK_CODE, hook_code, NULL, 1, 0);

	// emulate machine code in infinite time (last param = 0), or when
	// finishing all the code.
	err = uc_emu_start(uc, ADDRESS, ADDRESS + sizeof(SPARC_CODE) - 1, 0, 0);
	if (err) {
		printf("%u (%s)\n", err, uc_strerror(err));
	}

	// now print out some registers
	printf("done\n");

	uc_close(uc);
}

#define X86_CODE16 "\x00\x00"   // add   byte ptr [bx + si], al

static void test_x86_16(void)
{
	uc_engine *uc;
	uc_err err;
	uint8_t tmp;

	int32_t eax = 7;
	int32_t ebx = 5;
	int32_t esi = 6;

	// Initialize emulator in X86-16bit mode
	err = uc_open(UC_ARCH_X86, UC_MODE_16, &uc);
	if (err) {
		printf("%u\n", err);
		return;
	}

	// map 8KB memory for this emulation
	uc_mem_map(uc, 0, 8 * 1024, UC_PROT_ALL);

	// write machine code to be emulated to memory
	if (uc_mem_write(uc, 0, X86_CODE16, sizeof(X86_CODE16) - 1)) {
		return;
	}

	// initialize machine registers
	uc_reg_write(uc, UC_X86_REG_EAX, &eax);
	uc_reg_write(uc, UC_X86_REG_EBX, &ebx);
	uc_reg_write(uc, UC_X86_REG_ESI, &esi);

	// emulate machine code in infinite time (last param = 0), or when
	// finishing all the code.
	err = uc_emu_start(uc, 0, sizeof(X86_CODE16) - 1, 0, 0);
	if (err) {
		printf("%u: %s\n",
			err, uc_strerror(err));
	}

	// now print out some registers
	printf("done\n");

	uc_close(uc);
}

#define ARM_CODE "\x37\x00\xa0\xe3\x03\x10\x42\xe0"

static void test_arm(void)
{
	uc_engine *uc;
	uc_err err;
	uc_hook trace1, trace2;

	int r0 = 0x1234;     // R0 register
	int r2 = 0x6789;     // R1 register
	int r3 = 0x3333;     // R2 register
	int r1;     // R1 register

	const uint64_t ADDRESS = 0x10000;

	// Initialize emulator in ARM mode
	err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
	if (err) {
		printf("%u (%s)\n",
			err, uc_strerror(err));
		return;
	}

	// map 2MB memory for this emulation
	uc_mem_map(uc, ADDRESS, 2 * 1024 * 1024, UC_PROT_ALL);

	// write machine code to be emulated to memory
	uc_mem_write(uc, ADDRESS, ARM_CODE, sizeof(ARM_CODE) - 1);

	// initialize machine registers
	uc_reg_write(uc, UC_ARM_REG_R0, &r0);
	uc_reg_write(uc, UC_ARM_REG_R2, &r2);
	uc_reg_write(uc, UC_ARM_REG_R3, &r3);

	// tracing all basic blocks with customized callback
	uc_hook_add(uc, &trace1, UC_HOOK_BLOCK, hook_block, NULL, 1, 0);

	// tracing one instruction at ADDRESS with customized callback
	uc_hook_add(uc, &trace2, UC_HOOK_CODE, hook_code, NULL, ADDRESS, ADDRESS);

	// emulate machine code in infinite time (last param = 0), or when
	// finishing all the code.
	err = uc_emu_start(uc, ADDRESS, ADDRESS + sizeof(ARM_CODE) - 1, 0, 0);
	if (err) {
		printf("%u\n", err);
	}

	// now print out some registers
	printf("done\n");

	uc_reg_read(uc, UC_ARM_REG_R0, &r0);
	uc_reg_read(uc, UC_ARM_REG_R1, &r1);

	uc_close(uc);
}


#define ARM64_CODE "\xab\x05\x00\xb8\xaf\x05\x40\x38" // str w11, [x13]; ldrb w15, [x13]

static void test_arm64(void)
{
	uc_engine *uc;
	uc_err err;
	uc_hook trace1, trace2;

	int64_t x11 = 0x12345678;        // X11 register
	int64_t x13 = 0x10000 + 0x8;     // X13 register
	int64_t x15 = 0x33;              // X15 register

	const uint64_t ADDRESS = 0x10000;

	// Initialize emulator in ARM mode
	err = uc_open(UC_ARCH_ARM64, UC_MODE_ARM, &uc);
	if (err) {
		printf("%u (%s)\n",
			err, uc_strerror(err));
		return;
	}

	// map 2MB memory for this emulation
	uc_mem_map(uc, ADDRESS, 2 * 1024 * 1024, UC_PROT_ALL);

	// write machine code to be emulated to memory
	uc_mem_write(uc, ADDRESS, ARM64_CODE, sizeof(ARM64_CODE) - 1);

	// initialize machine registers
	uc_reg_write(uc, UC_ARM64_REG_X11, &x11);
	uc_reg_write(uc, UC_ARM64_REG_X13, &x13);
	uc_reg_write(uc, UC_ARM64_REG_X15, &x15);

	// tracing all basic blocks with customized callback
	uc_hook_add(uc, &trace1, UC_HOOK_BLOCK, hook_block, NULL, 1, 0);

	// tracing one instruction at ADDRESS with customized callback
	uc_hook_add(uc, &trace2, UC_HOOK_CODE, hook_code, NULL, ADDRESS, ADDRESS);

	// emulate machine code in infinite time (last param = 0), or when
	// finishing all the code.
	err = uc_emu_start(uc, ADDRESS, ADDRESS + sizeof(ARM64_CODE) - 1, 0, 0);
	if (err) {
		printf("%u\n", err);
	}

	uc_reg_read(uc, UC_ARM64_REG_X15, &x15);

	uc_close(uc);
}


#define M68K_CODE "\x76\xed" // movq #-19, %d3

static void test_m68k(void)
{
	uc_engine *uc;
	uc_hook trace1, trace2;
	uc_err err;

	int d0 = 0x0000;     // d0 data register
	int d1 = 0x0000;     // d1 data register
	int d2 = 0x0000;     // d2 data register
	int d3 = 0x0000;     // d3 data register
	int d4 = 0x0000;     // d4 data register
	int d5 = 0x0000;     // d5 data register
	int d6 = 0x0000;     // d6 data register
	int d7 = 0x0000;     // d7 data register

	int a0 = 0x0000;     // a0 address register
	int a1 = 0x0000;     // a1 address register
	int a2 = 0x0000;     // a2 address register
	int a3 = 0x0000;     // a3 address register
	int a4 = 0x0000;     // a4 address register
	int a5 = 0x0000;     // a5 address register
	int a6 = 0x0000;     // a6 address register
	int a7 = 0x0000;     // a6 address register

	int pc = 0x0000;     // program counter
	int sr = 0x0000;     // status register

	const uint64_t ADDRESS = 0x10000;

	// Initialize emulator in M68K mode
	err = uc_open(UC_ARCH_M68K, UC_MODE_BIG_ENDIAN, &uc);
	if (err) {
		printf("%u (%s)\n",
			err, uc_strerror(err));
		return;
	}

	// map 2MB memory for this emulation
	uc_mem_map(uc, ADDRESS, 2 * 1024 * 1024, UC_PROT_ALL);

	// write machine code to be emulated to memory
	uc_mem_write(uc, ADDRESS, M68K_CODE, sizeof(M68K_CODE) - 1);

	// initialize machine registers
	uc_reg_write(uc, UC_M68K_REG_D0, &d0);
	uc_reg_write(uc, UC_M68K_REG_D1, &d1);
	uc_reg_write(uc, UC_M68K_REG_D2, &d2);
	uc_reg_write(uc, UC_M68K_REG_D3, &d3);
	uc_reg_write(uc, UC_M68K_REG_D4, &d4);
	uc_reg_write(uc, UC_M68K_REG_D5, &d5);
	uc_reg_write(uc, UC_M68K_REG_D6, &d6);
	uc_reg_write(uc, UC_M68K_REG_D7, &d7);

	uc_reg_write(uc, UC_M68K_REG_A0, &a0);
	uc_reg_write(uc, UC_M68K_REG_A1, &a1);
	uc_reg_write(uc, UC_M68K_REG_A2, &a2);
	uc_reg_write(uc, UC_M68K_REG_A3, &a3);
	uc_reg_write(uc, UC_M68K_REG_A4, &a4);
	uc_reg_write(uc, UC_M68K_REG_A5, &a5);
	uc_reg_write(uc, UC_M68K_REG_A6, &a6);
	uc_reg_write(uc, UC_M68K_REG_A7, &a7);

	uc_reg_write(uc, UC_M68K_REG_PC, &pc);
	uc_reg_write(uc, UC_M68K_REG_SR, &sr);

	// tracing all basic blocks with customized callback
	uc_hook_add(uc, &trace1, UC_HOOK_BLOCK, hook_block, NULL, 1, 0);

	// tracing all instruction
	uc_hook_add(uc, &trace2, UC_HOOK_CODE, hook_code, NULL, 1, 0);

	// emulate machine code in infinite time (last param = 0), or when
	// finishing all the code.
	err = uc_emu_start(uc, ADDRESS, ADDRESS + sizeof(M68K_CODE) - 1, 0, 0);
	if (err) {
		printf("%u\n", err);
	}

	uc_reg_read(uc, UC_M68K_REG_D0, &d0);
	uc_reg_read(uc, UC_M68K_REG_D1, &d1);
	uc_reg_read(uc, UC_M68K_REG_D2, &d2);
	uc_reg_read(uc, UC_M68K_REG_D3, &d3);
	uc_reg_read(uc, UC_M68K_REG_D4, &d4);
	uc_reg_read(uc, UC_M68K_REG_D5, &d5);
	uc_reg_read(uc, UC_M68K_REG_D6, &d6);
	uc_reg_read(uc, UC_M68K_REG_D7, &d7);

	uc_reg_read(uc, UC_M68K_REG_A0, &a0);
	uc_reg_read(uc, UC_M68K_REG_A1, &a1);
	uc_reg_read(uc, UC_M68K_REG_A2, &a2);
	uc_reg_read(uc, UC_M68K_REG_A3, &a3);
	uc_reg_read(uc, UC_M68K_REG_A4, &a4);
	uc_reg_read(uc, UC_M68K_REG_A5, &a5);
	uc_reg_read(uc, UC_M68K_REG_A6, &a6);
	uc_reg_read(uc, UC_M68K_REG_A7, &a7);

	uc_reg_read(uc, UC_M68K_REG_PC, &pc);
	uc_reg_read(uc, UC_M68K_REG_SR, &sr);
	uc_close(uc);
}

#define MIPS_CODE_EB "\x34\x21\x34\x56" // ori $at, $at, 0x3456;
#define MIPS_CODE_EL "\x56\x34\x21\x34" // ori $at, $at, 0x3456;


static void test_mips_eb(void)
{
	uc_engine *uc;
	uc_err err;
	uc_hook trace1, trace2;

	int r1 = 0x6789;     // R1 register

	const uint64_t ADDRESS = 0x10000;

	printf("Emulate MIPS code (big-endian)\n");

	// Initialize emulator in MIPS mode
	err = uc_open(UC_ARCH_MIPS, (uc_mode)(UC_MODE_MIPS32 + UC_MODE_BIG_ENDIAN), &uc);
	if (err) {
		printf("Failed on uc_open() with error returned: %u (%s)\n",
			err, uc_strerror(err));
		return;
	}

	// map 2MB memory for this emulation
	uc_mem_map(uc, ADDRESS, 2 * 1024 * 1024, UC_PROT_ALL);

	// write machine code to be emulated to memory
	uc_mem_write(uc, ADDRESS, MIPS_CODE_EB, sizeof(MIPS_CODE_EB) - 1);

	// initialize machine registers
	uc_reg_write(uc, UC_MIPS_REG_1, &r1);

	// tracing all basic blocks with customized callback
	uc_hook_add(uc, &trace1, UC_HOOK_BLOCK, hook_block, NULL, 1, 0);

	// tracing one instruction at ADDRESS with customized callback
	uc_hook_add(uc, &trace2, UC_HOOK_CODE, hook_code, NULL, ADDRESS, ADDRESS);

	// emulate machine code in infinite time (last param = 0), or when
	// finishing all the code.
	err = uc_emu_start(uc, ADDRESS, ADDRESS + sizeof(MIPS_CODE_EB) - 1, 0, 0);
	if (err) {
		printf("%u (%s)\n", err, uc_strerror(err));
	}

	uc_reg_read(uc, UC_MIPS_REG_1, &r1);

	uc_close(uc);
}


void BeginEmu()
{
	test_sparc();
	test_x86_16();
	test_arm();
	test_arm64();
	test_m68k();
	test_mips_eb();
}
