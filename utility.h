//#define DEBUG

#ifdef DEBUG
#	define ASSERT(X) { if(!(X)) { printf("ERROR: file \"%s\", line \"%d\", \"" #X "\"\n", __FILE__, __LINE__); while(1); } }
#else
#	define ASSERT(X)
#endif

#define UTILITY_EXIT_FUNC_MAX_NUM 1024

void utility_atexit(void (*func)(void));
void utility_exit();
void utility_rand_init();
int utility_rand_get();
int utility_clock();
