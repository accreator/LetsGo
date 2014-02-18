#include"letsgo.h"
#include"hash.h"
#include<cstdio>
#include"utility.h"

U64 Zobrist[32*32][3];

void hash_zobrist_init()
{
	int i, k;
	utility_rand_init();
	for(i=0; i<32*32; i++)
	{
		for(k=1; k<=2; k++)
		{
			Zobrist[i][k] = (U64)utility_rand_get() ^ ((U64)utility_rand_get() << 15)
				^ ((U64)utility_rand_get() << 30) ^ ((U64)utility_rand_get() << 45) ^ ((U64)utility_rand_get() << 60);
		}
	}
}