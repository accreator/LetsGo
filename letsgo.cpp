/*
  Copyright 2011 Sun Kai. All Rights Reserved.
  Let's Go
  A simple AI for go using UCT algorithm
*/
#include"letsgo.h"
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<ctime>
#include<iostream>
#include"utility.h"
#include"hash.h"
#include"search.h"
#include"gtp.h"

const int MoveP[8] = { -1,  1, 32,-32, 31, 33,-31,-33};
const int MoveX[8] = {  0,  0,  1, -1,  1,  1, -1, -1};
const int MoveY[8] = {  1, -1,  0,  0, -1,  1,  1, -1};
const int MoveP2[3][3] =
{
	{-33, -32, -31},
	{ -1,   0,   1},
	{ 31,  32,  33}
};

int BoardSize = 9; /* <=19 */
int GoBoard[32*32];
int LifeBoard[32*32];
//int StrBoard[32*32];
//int StrNumber = 0;
int PieceCount[3];
int Round;
int PositionType[32*32];
int TransBoard[8][32*32];

int TimeLimit = 10000;

struct MOVESTACK
{
	char side;
	char type; /* 0:do 1:undo 2:pass 3:blank */
	short pos;
};

struct LIFESTACK
{
	short life;
	short pos;
};

static struct MOVESTACK MoveStack[LETSGO_MAX_DEPTH * 32 * 32];
static struct MOVESTACK *MoveStackTop = MoveStack;

static struct LIFESTACK LifeStack[LETSGO_MAX_DEPTH * 32 * 32];
static struct LIFESTACK *LifeStackTop = LifeStack;

U64 ZobKey[LETSGO_MAX_DEPTH];
int KoRecord[LETSGO_MAX_DEPTH];

inline void letsgo_movestack_push(char side, char type, short pos)
{
	MoveStackTop->side = side;
	MoveStackTop->type = type;
	MoveStackTop->pos = pos;
	MoveStackTop ++;
}

inline void letsgo_movestack_pop()
{
	MoveStackTop --;
}

inline void letsgo_lifestack_push(short life, short pos)
{
	LifeStackTop->life = life;
	LifeStackTop->pos = pos;
	LifeStackTop ++;
}

inline void letsgo_lifestack_pop()
{
	LifeStackTop --;
}

int letsgo_last_move()
{
	if(MoveStackTop > MoveStack) return (MoveStackTop-1)->pos;
	return -1;
}

void letsgo_life_update(U64 &key, int move)
{
	static int que1[32*32];
	static int que2[32*32];
	static int vis1[32*32] = {0};
	static int vis2[32*32] = {0};
	static int visflag1 = 0;
	static int visflag2 = 0;
	int i, j, k, l, t;
	int p, q, nxt, nnxt;
	int pos, side;

	search_vis_maintain(vis1, visflag1);
	search_vis_maintain(vis2, visflag2);

	side = letsgo_get_op(GoBoard[move]);
	++visflag1;
	p = 0;
	q = 0;
	for(i=0; i<4; i++)
	{
		pos = move + MoveP[i];
		if(GoBoard[pos] == side && LifeBoard[pos] == 1)
		{		
			que1[q] = pos;
			vis1[pos] = visflag1;
			q ++;
			
		}
	}
	for(; p<q; ++p)
	{
		for(i=0; i<4; i++)
		{
			nxt = que1[p] + MoveP[i];
			if(vis1[nxt] != visflag1 && GoBoard[nxt] == side)
			{
				que1[q] = nxt;
				q ++;
				vis1[nxt] = visflag1;
			}
		}
	}

	if(q != 0)
	{
		l = q;
		for(i=0; i<l; i++)
		{
			letsgo_movestack_push(side, 1, que1[i]);
			GoBoard[que1[i]] = 0;
			letsgo_lifestack_push(LifeBoard[que1[i]], que1[i]);
			LifeBoard[que1[i]] = 0;
			key ^= Zobrist[que1[i]][side];
			PieceCount[side] --;
		}
		for(i=0; i<l; i++)
		{
			for(j=0; j<4; j++)
			{
				nxt = que1[i] + MoveP[j];
				if(GoBoard[nxt] == letsgo_get_op(side) && vis1[nxt] != visflag1)
				{					
					que2[0] = nxt;
					vis1[nxt] = visflag1;
					visflag2 ++;
					vis2[nxt] = visflag2;
					t = 0;
					for(p=0,q=1; p<q; ++p)
					{
						for(k=0; k<4; k++)
						{
							nnxt = que2[p] + MoveP[k];
							if(vis2[nnxt] != visflag2)
							{
								if(GoBoard[nnxt] == 0) t ++;
								else if(GoBoard[nnxt] == letsgo_get_op(side))
								{
									que2[q] = nnxt;
									q ++;
									vis1[nnxt] = visflag1;
								}
								vis2[nnxt] = visflag2;
							}
						}
					}
					for(k=0; k<q; k++)
					{
						letsgo_lifestack_push(LifeBoard[que2[k]], que2[k]);
						LifeBoard[que2[k]] = t;
					}
				}
			}
		}
	}
	else
	{
		++ visflag1;
		que1[0] = move;
		vis1[move] = visflag1;
		l = 0;
		for(p=0,q=1; p<q; ++p)
		{
			for(i=0; i<4; i++)
			{
				nxt = que1[p] + MoveP[i];
				if(vis1[nxt] != visflag1)
				{
					if(GoBoard[nxt] == 0) l ++;
					else if(GoBoard[nxt] == letsgo_get_op(side))
					{
						que1[q] = nxt;
						q ++;
					}
					vis1[nxt] = visflag1;
				}
			}
		}
		for(i=0; i<q; i++)
		{
			letsgo_lifestack_push(LifeBoard[que1[i]], que1[i]);
			LifeBoard[que1[i]] = l;
		}
	}

	++ visflag1;
	for(k=0; k<4; k++)
	{
		pos = move + MoveP[k];
		if(GoBoard[pos] == side && vis1[pos] != visflag1)
		{	
			que1[0] = pos;
			vis1[pos] = visflag1;
			for(p=0,q=1; p<q; ++p)
			{
				letsgo_lifestack_push(LifeBoard[que1[p]], que1[p]);
				LifeBoard[que1[p]] --;
				for(i=0; i<4; i++)
				{
					nxt = que1[p] + MoveP[i];
					if(vis1[nxt] != visflag1 && GoBoard[nxt] == side)
					{
						que1[q] = nxt;
						q ++;
						vis1[nxt] = visflag1;	
					}
				}
			}
		}
	}
}

bool letsgo_check_forbid(int depth, int side, int move)
{
	int i, nxt;
	if(move == KoRecord[depth-1]) return true; //forbid
	for(i=0; i<4; i++)
	{
		nxt = move + MoveP[i];
		if(GoBoard[nxt] == -1) continue;
		if(GoBoard[nxt] == 0) break;
		if(GoBoard[nxt] == side)
		{
			if(LifeBoard[nxt] > 1) break;
		}
		else
		{
			if(LifeBoard[nxt] == 1) break;
		}		
	}
	return (i == 4);
}

int letsgo_make_move(int depth, int side, int move)
{
	int i, nxt;
	int t;
	bool f;

	if(move == -1)
	{
		ZobKey[depth] = ZobKey[depth-1];
		KoRecord[depth] = -1;
		letsgo_movestack_push(0, 3, 0);
		letsgo_lifestack_push(-1, 0);
		return 1;
	}

	if(letsgo_check_forbid(depth, side, move)) return 0; //unsuccessful	

	for(i=0; i<4; i++)
	{
		nxt = move + MoveP[i];
		if(GoBoard[nxt] == 0 || GoBoard[nxt] == side) break;
	}
	f = (i == 4);

	ZobKey[depth] = ZobKey[depth-1] ^ Zobrist[move][side];

	letsgo_movestack_push(0, 3, 0);
	letsgo_lifestack_push(-1, 0);
	GoBoard[move] = side;

	t = PieceCount[letsgo_get_op(side)];
	letsgo_life_update(ZobKey[depth], move);
	t -= PieceCount[letsgo_get_op(side)];

	if(f && t==1)
	{
		for(i=0; i<4; i++)
		{
			nxt = move + MoveP[i];
			if(GoBoard[nxt] == 0)
			{
				KoRecord[depth] = nxt;
				break;
			}
		}
	}
	else
		KoRecord[depth] = -1;

	PieceCount[side] ++;
	letsgo_movestack_push(side, 0, move);

	ASSERT(ZobKey[depth] != ZobKey[depth-2]);

	if(ZobKey[depth] == ZobKey[depth-2]) //Ó¦¸ÃºãÎª¼Ù
	{
		letsgo_unmake_move();
		return 0; //unsuccessful
	}

	return 1;
}

void letsgo_unmake_move()
{
	while(MoveStackTop--)
	{
		if(MoveStackTop->type == 3) break;
		if(MoveStackTop->type == 2) continue;
		if(MoveStackTop->type == 1)
		{
			GoBoard[MoveStackTop->pos] = MoveStackTop->side;
			PieceCount[MoveStackTop->side] ++;
		}
		else
		{
			GoBoard[MoveStackTop->pos] = 0;
			PieceCount[MoveStackTop->side] --;
		}
	}
	while(LifeStackTop--)
	{
		if(LifeStackTop->life == -1) break;
		LifeBoard[LifeStackTop->pos] = LifeStackTop->life;
	}
}

void letsgo_board_init()
{
	int i, j;
	memset(GoBoard, -1, sizeof(GoBoard));
	memset(PositionType, -1, sizeof(PositionType));
	for(i=1; i<=BoardSize; i++)
	{
		for(j=1; j<=BoardSize; j++)
		{
			GoBoard[letsgo_get_p(i,j)] = 0;
			PositionType[letsgo_get_p(i,j)] = 0;
			if(i==1 || i==BoardSize || j==1 || j==BoardSize)
			{
				PositionType[letsgo_get_p(i,j)] = 1;
				if(i==1 && (j==1 || j==BoardSize)) PositionType[letsgo_get_p(i,j)] = 2;
				if(i==BoardSize && (j==1 || j==BoardSize)) PositionType[letsgo_get_p(i,j)] = 2;
			}
			TransBoard[0][letsgo_get_p(i,j)] = letsgo_get_p(i,j);
			TransBoard[1][letsgo_get_p(BoardSize-i+1, j)] = letsgo_get_p(i,j);
			TransBoard[2][letsgo_get_p(i, BoardSize-j+1)] = letsgo_get_p(i,j);
			TransBoard[3][letsgo_get_p(BoardSize-i+1, BoardSize-j+1)] = letsgo_get_p(i,j);
			TransBoard[4][letsgo_get_p(j,i)] = letsgo_get_p(i,j);
			TransBoard[5][letsgo_get_p(BoardSize-j+1, i)] = letsgo_get_p(i,j);
			TransBoard[6][letsgo_get_p(j, BoardSize-i+1)] = letsgo_get_p(i,j);
			TransBoard[7][letsgo_get_p(BoardSize-j+1, BoardSize-i+1)] = letsgo_get_p(i,j);
		}
	}
}

void letsgo_init()
{
	utility_rand_init();
	letsgo_board_init();
	hash_zobrist_init();
	search_book_init();
	search_pattern_init();
	Round = 1;
	memset(PieceCount, 0, sizeof(PieceCount));
	MoveStackTop = MoveStack;
	LifeStackTop = LifeStack;
	memset(LifeBoard, 0, sizeof(LifeBoard));
}

long TimeUsed;
int CurrentVal;
int CurrentMove;
int CurrentStatus;

void letsgo_board_draw()
{
	int i, j;
	char wordsboard[21][44] =
	{
		 "  £Á£Â£Ã£Ä£Å£Æ£Ç£È£É£Ê£Ë£Ì£Í£Î£Ï£Ð£Ñ£Ò£Ó£Ô",
		 " 0©°©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©´",
		 " 1©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 2©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 3©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 4©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 5©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 6©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 7©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 8©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 " 9©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "10©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "11©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "12©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "13©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "14©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "15©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "16©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "17©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "18©À©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©à©È",
		 "19©¸©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©¼"
	};
	char tmp[21][44] = {{0}};
	
	system("cls");
	printf("LetsGo %s\n", VERSION);

	memcpy(tmp[0], wordsboard[0], BoardSize*2+2);
	for(i=1; i<BoardSize+1; i++)
	{
		memcpy(tmp[i], wordsboard[i], BoardSize*2);
		memcpy(tmp[i]+BoardSize*2, wordsboard[i]+40, 2);
	}
	memcpy(tmp[BoardSize]+2, wordsboard[20]+2, BoardSize*2-2);
	memcpy(tmp[BoardSize]+BoardSize*2, wordsboard[20]+40, 2);

	for(i=1; i<=BoardSize; i++)
	{
		for(j=1; j<=BoardSize; j++)
		{
			int p = letsgo_get_p(i,j);
			switch(GoBoard[letsgo_get_p(i,j)])
			{
				case 1:
					memcpy(&tmp[i][j*2], (p == CurrentMove ? "¡ô" : "¡ñ"), 2); break;
				case 2:
					memcpy(&tmp[i][j*2], (p == CurrentMove ? "¡ó" : "¡ð"), 2); break;
			}
		}
	}
	for(i=0; i<BoardSize+1; i++)
	{
		printf("%s  ", tmp[i]);
		switch(i)
		{
			case 0: printf("Time: %ldms\n", TimeUsed); break;
			case 1: printf("Val: %d\n", CurrentVal); break;
			case 2: printf("Node: %d\n", UCTTreeP * SEARCH_UCT_A); break;
			case 3: printf("Speed: %dknps\n", UCTTreeP * SEARCH_UCT_A / (TimeUsed+1)); break;
			case 4: printf("Status: %s\n", CurrentStatus ? "END" : "ING"); break;
			default: printf("\n");
		}
	}
	for(i=1; i<=BoardSize; i++)
	{
		for(j=1; j<=BoardSize; j++)
		{
			printf("%d ", LifeBoard[letsgo_get_p(i,j)]);
		}
		puts("");
	}
}

void letsgo_text_ui()
{
	int playerside = 1;
	char c;
	int x, y;
	int move, val;
	letsgo_board_draw();

	letsgo_make_move(Round+1, letsgo_get_op(playerside), letsgo_get_p(BoardSize/2+1, BoardSize/2+1));
	letsgo_board_draw();
	while(scanf("%d%c", &x, &c) != EOF)
	{
		y = c;
		if(y >= 'a' && y <= 'z') y = y - 'a' + 'A';
		x = x + 1;
		y = y - 'A' + 1;
		if(x == 0)
		{
			letsgo_unmake_move();
			Round --;
			letsgo_board_draw();
			continue;
		}
		if(x != -1) // x == -1 : pass
		{
			if(x<1 || x>BoardSize || y<1 || y>BoardSize || GoBoard[letsgo_get_p(x,y)]) continue;
			if(letsgo_make_move(Round+1, playerside, letsgo_get_p(x, y)) == 0) continue;
			Round ++;
		}
		else
		{
			letsgo_make_move(Round+1, playerside, -1);
			Round ++;
		}
		if(x != -1)
			CurrentMove = letsgo_get_p(x, y);
		else
			CurrentMove = -1;

		letsgo_board_draw();

		TimeUsed = utility_clock();
		if((val = search_book_lookup(letsgo_get_op(playerside), &move)) == 0)
		{
			val = search_uct_main(Round, letsgo_get_op(playerside), &move);	
		}
		TimeUsed = utility_clock() - TimeUsed;

		letsgo_make_move(Round+1, letsgo_get_op(playerside), move);
		Round ++;

		if(move == -1 && CurrentMove == -1)
		{
			CurrentVal = search_moku_count(letsgo_get_op(playerside));
			CurrentStatus = 1;
		}
		if(move != -1)
			CurrentVal = val;
		CurrentMove = move;
		
		letsgo_board_draw();
	}
}

int main(int argc, char *argv[])
{
	int flag = 0;
	int i;
	for(i=1; i<argc; i++)
	{
		if(strcmp(argv[i], "--mode") == 0)
		{
			if(i+1<argc)
			{
				i ++;
				if(strcmp(argv[i], "gtp") == 0) flag = 1;
			}
		}
		else if(strcmp(argv[i], "--level") == 0)
		{
			if(i+1<argc)
			{
				int c = 0;
				i ++;
				sscanf(argv[i], "%d", &c);
				switch(c)
				{
					case 1: TimeLimit = 1000; break;
					case 2: TimeLimit = 2500; break;
					case 3: TimeLimit = 5000; break;
					case 4: TimeLimit = 10000; break;
					case 5: TimeLimit = 15000; break;
					case 6: TimeLimit = 22500; break;
					case 7: TimeLimit = 30000; break;
					case 8: TimeLimit = 40000; break;
					case 9: TimeLimit = 60000; break;
					case 10: TimeLimit = 90000; break;
					default: TimeLimit = 120000; break;
				}
			}
		}
		else if(strcmp(argv[i], "--help") == 0)
		{
			printf("\n");
			return 0;
		}
		else if(strcmp(argv[i], "--version") == 0)
		{
			printf("Lets Go "VERSION"\n");
			return 0;
		}
	}
	if(flag == 0)
	{
		letsgo_init();
		letsgo_text_ui();
	}
	else
	{
		play_gtp();
	}
	return 0;
}
