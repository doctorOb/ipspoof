typedef union uwb {
    unsigned w;
    unsigned char b[4];
} WBunion;

unsigned *md5( const char *msg, int mlen);
