// 2018 the syntax will evolve to t_r0. n_r1 etc.
// Valid thru v1.8
#define t   	r0      // t_r0
#define t_r0	r0

#define n				r1      // n_r1		v4th
#define n_r1		r1

#define w				r2      // w_r2		FISH next v4th NOS DPUSH
#define w_r2		r2

#define x				r3      // x_r3		Execution
#define x_r3		r3

#define y				r4      // y_r4		BX as scratch (ROT)
#define y_r4		r4

#define i				r5      // i_r5		IP
#define i_r5		r5

#define r				r6      // r_r6		Return Stack Pointer
#define r_r6    r6

#define p				r7      // p_r7		Parameter Stack Pointer
#define p_r7		r7

//              r8      // Link register
//              r9      // Program Counter
//	ARMv6 Cortex M0 LPC 1114 is 16 bit thumb ISA.
//	Use of the following registers is limited to local variables mostly.
//      ST doesn't seem to have this issue?
// speculation

#define ra			r10     // ra_r10
#define ra_r10	r10

#define rb			r11     // rb_r11
#define rb_r11	r11

#define k				r12     // k_r12
#define k_r12		r12
