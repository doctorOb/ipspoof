#include "xxHash.h"

#define PRIME1 2654435761U
#define PRIME2 2246822519U
#define PRIME3 3266489917U
#define PRIME4  668265263U
#define PRIME5 0x165667b1

unsigned int XXH_small(const void* key, int len, unsigned int seed)
{
        const unsigned char* p = (unsigned char*)key;
        const unsigned char* const bEnd = p + len;
        unsigned int idx = seed + PRIME1;
        unsigned int crc = PRIME5;
        const unsigned char* const limit = bEnd - 4;

        while (p<limit)
        {
                crc += ((*(unsigned int*)p) + idx++);
                crc += (crc << 17) * PRIME4;
                crc *= PRIME1;
                p+=4;
        }

        while (p<bEnd)
        {
                crc += ((*p) + idx++);
                crc *= PRIME1;
                p++;
        }

        crc += len;

        crc ^= crc >> 15;
        crc *= PRIME2;
        crc ^= crc >> 13;
        crc *= PRIME3;
        crc ^= crc >> 16;

        return crc;
}
