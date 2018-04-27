// Use chip serial number to control cloning and track customers.
// Encode serial number such that REVW instruction needed to unpack it.
// Place it inline in :NONAME FISH or at the end of :NONAME primitive.

// 0xE000ED00 CPUID Base Register
// The CPUID Base Register characteristics are:
// Purpose  Provides identification information for the processor.
// Usage constraints  This register is word accessible only.
// Configurations  Always implemented.

// EX: 1RST 1114 USB BOARD CPUID AND REVW VALUE:
// 	410CC200
//	00C20C41

// It's label will be GOPRO:
 SECTION .text : CONST (2)

GOPRO:

 ALIGNROM 2,0xFFFFFFFF

