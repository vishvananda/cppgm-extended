int host_byte(char c) { return (unsigned char)c; }
char host_identity(char c) { return c; }
int host_callback(char (*f)(char)) { return (unsigned char)f((char)255); }
