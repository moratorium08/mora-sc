
#ifndef TEST_INCLUDE
#define TEST_INCLUDE
#define expect_int(a, b) fexpect_int(__FILE__, __LINE__, a, b)
#define expect_str(a, b) fexpect_str(__FILE__, __LINE__, a, b)
#define error_msg(msg) ferror_msg(__FILE__, __LINE__, msg)

void fexpect_int(char *filename, int line, int a, int b);
void fexpect_str(char *filename, int line, char *a, char *b);
void ferror_msg(char *filename, int line, char *c);
void describe(char *description);
void described();
int summary();
#endif
