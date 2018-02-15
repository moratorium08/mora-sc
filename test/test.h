
#define expect_int(a, b) fexpect_int(__FILE__, __LINE__, a, b)
#define expect_str(a, b) fexpect_str(__FILE__, __LINE__, a, b)

void fexpect_int(char *filename, int line, int a, int b);
void fexpect_str(char *filename, int line, char *a, char *b);
void describe(char *description);
void described();
void summary();
