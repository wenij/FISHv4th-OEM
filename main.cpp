$core_cm4.h
 extern void FM3_COLD();
//#ifdef USE_CMAIN
int main(void) {
  
	volatile static int i = 0 ;
	while(1) {
		FM3_COLD();		// ret2c returns here
		i++ ;
	}
	return 0 ;
}
//#endif