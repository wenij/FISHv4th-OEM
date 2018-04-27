// 2018 the syntax will evolve to t_r0. n_r1 etc.
// Valid thru v1.8
#define t       r0      //                          AX
#define n	r1      // v4th
#define w	r2      // v4th NOS DPUSH           DX
#define x	r3      // Execution
#define y	r4      //                          BX as scratch (ROT)
#define i	r5      // IP                       DX:SI
#define r	r6      // Return Stack Pointer
#define p	r7      // Parameter Stack Pointer
//              r8      // Link register
//              r9      // Program Counter
//	ARMv6 Cortex M0 LPC 1114 is 16 bit thumb ISA.
//	Use of the following registers is limited to local variables mostly.
//      ST doesn't seem to have this issue?
// speculation
#define ra	r10     // 
#define rb	r11     // 
#define k	r12     // 
