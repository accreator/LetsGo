#include"letsgo.h"
#include"search.h"
#include<cstdio>
#include<cmath>
#include"utility.h"

struct GENLIST
{
	short move;
	short weighting;
//	short l, r; 
};
struct GENLIST GenList[LETSGO_MAX_DEPTH][32*32];

bool search_is_eye(int p, int side)
{
	int i, around;
	int count = 0;
	for(i=0; i<4; i++)
	{
		around = p + MoveP[i];
		if(GoBoard[around] != -1 && GoBoard[around] != side) return false;
	}
	for(i=4; i<8; i++)
	{
		around = p + MoveP[i];
		if(GoBoard[around] == letsgo_get_op(side)) count ++;
	}
	return count==0 || (PositionType[p]==0 && count<=1);
}

/*
bool search_is_eye(int p, int side)
{
	return false;
}
*/
/*
bool search_is_badmove(int p, int side)
{
	//return false;
	int i;
	int nxt;
	int c = 0;
	for(i=0; i<4; i++)
	{
		nxt = p + MoveP[i];
		if(GoBoard[nxt] == 0) c ++;
		else if(GoBoard[nxt] == side)
		{
			if(LifeBoard[nxt] >= 2) return false;
			c ++;
		}
	}
	return c <= 1;
}
*/
int search_moku_count(int side)
{
	static int vis[32*32] = {0};
	static int visflag = 0;
	static int que[32*32];
	int i, j, k;
	int pos, nxt;
	int p, q;
	int count = 0;

	search_vis_maintain(vis, visflag);
	visflag ++;

	for(i=1; i<=BoardSize; i++)
	{
		for(j=1; j<=BoardSize; j++)
		{
			pos = letsgo_get_p(i,j);
			if(GoBoard[pos] == 0)
			{
				int f = 0;
				que[0] = pos;
				vis[pos] = visflag;
				for(p=0,q=1; p<q; p++)
				{
					for(k=0; k<4; k++)
					{
						nxt = que[p] + MoveP[k];
						if(GoBoard[nxt] == 0)
						{
							if(vis[nxt] != visflag)
							{
								vis[nxt] = visflag;
								que[q] = nxt;
								q ++;
							}
						}
						else if(GoBoard[nxt] != -1)
						{
							f |= GoBoard[nxt];
						}
					}
				}
				switch(f)
				{
				case 1: count += q; break;
				case 2: count -= q; break;
				}
			}
			else
			{
				if(GoBoard[pos] == 1)
					count ++;
				else if(GoBoard[pos] == 2)
					count --;					
			}
		}
	}

	return (side==1 ? count : -count);
}

int search_evaluation(int side)
{
	int i, j, k;
	int p;
	int val = PieceCount[side] - PieceCount[letsgo_get_op(side)];
	for(i=1; i<=BoardSize; i++)
	{
		for(j=1; j<=BoardSize; j++)
		{
			p = letsgo_get_p(i, j);
			if(GoBoard[p] == 0)
			{
				for(k=0; k<4; k++)
				{
					if(GoBoard[p+MoveP[k]] != -1)
					{
						if(GoBoard[p+MoveP[k]] == side) val ++; else val --;
						break;
					}
				}
			}
		}
	}
	return val;
}

void search_vis_maintain(int vis[], int &visflag)
{
	if(visflag == 2000000000)
	{
		int i, j;
		for(i=1; i<=BoardSize; i++)
		{
			for(j=1; j<=BoardSize; j++)
			{
				vis[letsgo_get_p(i,j)] = 0;
			}
		}
		visflag = 0;
	}
}

//int search_history_table[32*32];
struct PATTERN
{
	int map[3][3];
};
struct PATTERN GenPattern1[4*8] =
{
	{
		     1,     2,     1,
		     4, 1|2|4,     4,
		 1|2|4, 1|2|4, 1|2|4
	},
	{
		     1,     2,     4,
		     4, 1|2|4,     4,
		 1|2|4,     4, 1|2|4
	},
	{
		     1,     2, 1|2|4,
		     1, 1|2|4,     4,
		 1|2|4,     4, 1|2|4
	},
	{
		     1,     2,     2,
		     4,   1|4,     4,
		 1|2|4,     4, 1|2|4
	}
};
struct PATTERN GenPattern2[3*8] =
{
	{
		     1,     2, 1|2|4,
		     2, 1|2|4, 1|2|4,
		 1|2|4, 1|2|4, 1|2|4
	},
	{
		     1,     2, 1|2|4,
		     2, 1|2|4,     2,
		 1|2|4,     4, 1|2|4
	},
	{
		     1,     2, 1|2|4,
		     2, 1|2|4,     4,
		 1|2|4,     2, 1|2|4
	}
};
struct PATTERN GenPattern3[1*8] =
{
	{
		 1|2|4,     1, 1|2|4,
		     2, 1|2|4,     2,
		   1|4,   1|4,   1|4
	}
};
struct PATTERN GenPattern4[5*8] =
{
	{
		     1,     4, 1|2|4,
		     2, 1|2|4, 1|2|4,
		     8,     8,     8
	},
	{
		 1|2|4,     1, 1|2|4,
		   2|4, 1|2|4,     2,
		     8,     8,     8
	},
	{
		 1|2|4,     1,     2,
		 1|2|4,   1|4, 1|2|4,
		     8,     8,     8
	},
	{
		 1|2|4,     1,     2,
		 1|2|4,   2|4,   2|4,
		     8,     8,     8
	},
	{
		 1|2|4,     1,     2,
		     2,   2|4,     1,
		     8,     8,     8
	}
};

void search_pattern_init()
{
	static int flag = 0;
	struct PATTERN *pattern[4] = {GenPattern1, GenPattern2, GenPattern3, GenPattern4};
	int patternlen[4] = {4, 3, 1, 5};
	int i, j, k, l;
	if(flag) return;
	//printf("%d %d\n", GenPattern3[1].map[0][0], GenPattern3[0].map[1][1]); while(1);
	flag = 1;
	for(l=0; l<4; l++)
	{
		struct PATTERN *pat = pattern[l];
		int len = patternlen[l];

		for(i=len; i<len*4; i++)
		{
			for(j=0; j<3; j++)
			{
				for(k=0; k<3; k++)
				{
					pat[i].map[k][2-j] = pat[i-len].map[j][k];
				}
			}
		}
		for(i=len*4; i<len*5; i++)
		{
			for(j=0; j<3; j++)
			{
				for(k=0; k<3; k++)
				{
					pat[i].map[j][k] = pat[i-len*4].map[2-j][k];
				}
			}
		}
		for(i=len*5; i<len*8; i++)
		{
			for(j=0; j<3; j++)
			{
				for(k=0; k<3; k++)
				{
					pat[i].map[k][2-j] = pat[i-len].map[j][k];
				}
			}
		}
	}
}

int search_move_gen(struct GENLIST *plist, int depth, int side, int type)
{
	static int vis[32*32] = {0};
	static int visflag = 0;
	int num = 0;
	int i, j, k, l, r;
	int mv, nxt;
	int lastmove;
	int flag;

	search_vis_maintain(vis, visflag);

	visflag ++;

	//printf("%d ", type);
	lastmove = letsgo_last_move();
	switch(type)
	{
	/*
	case 0: //abandoned
		for(i=1; i<=BoardSize; i++)
		{
			for(j=1; j<=BoardSize; j++)
			{
				mv = letsgo_get_p(i, j);
				if(GoBoard[mv]!=0  && LifeBoard[mv]<4)
				{
					if(GoBoard[mv] == side)
					{
						for(k=0; k<4; k++)
						{
							nxt = mv + MoveP[k];
							if(GoBoard[nxt]==0 && vis[nxt]!=visflag)
							{
								for(l=0; l<4; l++)
								{
									int nnxt = nxt + MoveP[l];
									if(GoBoard[nnxt] == 0) break;
									if(GoBoard[nnxt] == side)
									{
										if(LifeBoard[nnxt]!=LifeBoard[mv] && LifeBoard[nnxt]>=3) break; //LifeBoard[nnxt] != LifeBoard[mv] => StrBoard[nnxt] != StrBoard[mv]
									}
								}
								if(l < 4)
								{
									vis[nxt] = visflag;
									plist[num].move = nxt;
									num ++;
								}
							}
						}
					}
					else //GoBoard[mv] == letsgo_get_op(side)
					{
						for(k=0; k<4; k++)
						{
							nxt = mv + MoveP[k];
							if(GoBoard[nxt]==0 && vis[nxt]!=visflag)
							{
								vis[nxt] = visflag;
								plist[num].move = nxt;
								num ++;
							}
						}
					}
				}
			}
		}
		break;
	*/
	case 0:
		if(lastmove == -1) break;
		flag = 0;
		for(i=0; i<4; i++)
		{
			mv = lastmove + MoveP[i];
			if(GoBoard[mv] == side && LifeBoard[mv] == 1)
			{
				for(k=0; k<4; k++)
				{
					nxt = mv + MoveP[k];
					if(vis[nxt] != visflag && GoBoard[nxt] == 0)
					{
						int c = 0, f = 0;
						for(l=0; l<4; l++)
						{
							if(GoBoard[nxt+MoveP[l]] == 0) c ++;
						}
						if(c > 1)
						{
							flag = 1;
							plist[num].move = nxt;
							num ++;
							vis[nxt] = visflag;
							continue;
						}
						for(l=0; l<4; l++)
						{
							int nnxt = nxt + MoveP[l];
							if(GoBoard[nnxt] == side)
							{
								if(LifeBoard[nnxt] >= 2 && c > 0) break;
								if(LifeBoard[nnxt] >= 3) break;
							}
						}
						if(l < 4)
						{
							flag = 1;
							plist[num].move = nxt;
							num ++;
							vis[nxt] = visflag;
							continue;
						}
						for(l=0; l<4; l++)
						{
							int nnxt = nxt + MoveP[l];
							if(GoBoard[nnxt] == side && LifeBoard[nnxt] >= 2) c ++;
						}
						if(c > 1)
						{
							plist[num].move = nxt;
							num ++;
							vis[nxt] = visflag;
							continue;
						}						
					}
				}
				for(j=1; j<=BoardSize; j++)
				{
					for(k=1; k<=BoardSize; k++)
					{
						nxt = letsgo_get_p(j, k);
						if(vis[nxt] != visflag && GoBoard[nxt] == 0)
						{
							for(l=0; l<4; l++)
							{
								int nnxt = nxt + MoveP[l];
								if(GoBoard[nnxt] == letsgo_get_op(side) && LifeBoard[nnxt] == 1) break;
							}
							if(l < 4)
							{
								if(letsgo_make_move(depth+1, side, nxt))
								{
									if(LifeBoard[mv] > 1)
									{
										flag = 1;
										plist[num].move = nxt;
										num ++;
										vis[nxt] = visflag;
									}
									letsgo_unmake_move();
								}
							}
						}
					}
				}
			}
		}
		if(flag == 0) num = 0;
		break;
	case 1:
		if(lastmove == -1) break;
		for(i=0; i<8; i++)
		{
			flag = 0;
			mv = lastmove + MoveP[i];
			if(GoBoard[mv] != 0) continue;
			for(j=0; j<=3; j+=3) //side
			{
				for(k=0; k<4*8; k++)
				{
					if(GenPattern1[k].map[1][1] & (j^side))
					{
						int f = 0;
						for(l=0; l<3; l++)
						{
							for(r=0; r<3; r++)
							{
								nxt = mv + MoveP2[l][r];
								if(GoBoard[nxt]==0 && !(GenPattern1[k].map[l][r]&4)) { f=1; break; }
								if(GoBoard[nxt]==1 && !(GenPattern1[k].map[l][r]&(j^1))) { f=1; break; }
								if(GoBoard[nxt]==2 && !(GenPattern1[k].map[l][r]&(j^2))) { f=1; break; }
								if(GoBoard[nxt]==-1) { f=1; break; }
							}
							if(f) break;
						}
						if(f == 0)
						{
							plist[num].move = mv;
							num ++;
							flag = 1;
							break;
						}
					}
				}
				if(flag) break;

				for(k=0; k<3*8;)
				{
					int s;
					for(s=0; s<3; s++)
					{
						int f = 1;
						if(GenPattern4[k].map[1][1] & (j^side))
						{
							for(l=0; l<3; l++)
							{
								for(r=0; r<3; r++)
								{
									nxt = mv + MoveP2[l][r];
									if(GoBoard[nxt]==0 && !(GenPattern4[k].map[l][r]&4)) { f=0; break; }
									if(GoBoard[nxt]==1 && !(GenPattern4[k].map[l][r]&(j^1))) { f=0; break; }
									if(GoBoard[nxt]==2 && !(GenPattern4[k].map[l][r]&(j^2))) { f=0; break; }
									if(GoBoard[nxt]==-1 && !(GenPattern4[k].map[l][r]&8)) { f=0; break; }
								}
								if(f==0) break;
							}
						}
						else f = 0;
						if(s==0 && f==0) { k+=3; break; }
						if(s!=0 && f==1) { k+=3-s; break; }
						k ++;
					}
					if(s == 3)
					{
						plist[num].move = mv;
						num ++;
						flag = 1;
						break;
					}
				}
				if(flag) break;

				for(k=0; k<1*8; k++)
				{
					if(GenPattern3[k].map[1][1] & (j^side))
					{
						int f = 0;
						for(l=0; l<3; l++)
						{
							for(r=0; r<3; r++)
							{
								nxt = mv + MoveP2[l][r];
								if(GoBoard[nxt]==0 && !(GenPattern3[k].map[l][r]&4)) { f=1; break; }
								if(GoBoard[nxt]==1 && !(GenPattern3[k].map[l][r]&(j^1))) { f=1; break; }
								if(GoBoard[nxt]==2 && !(GenPattern3[k].map[l][r]&(j^2))) { f=1; break; }
								if(GoBoard[nxt]==-1) { f=1; break; }
							}
							if(f) break;
						}
						if(f == 0)
						{
							plist[num].move = mv;
							num ++;
							flag = 1;
							break;
						}
					}
				}
				if(flag) break;

				for(k=0; k<5*8; k++)
				{
					if(GenPattern4[k].map[1][1] & (j^side))
					{
						int f = 0;
						for(l=0; l<3; l++)
						{
							for(r=0; r<3; r++)
							{
								nxt = mv + MoveP2[l][r];
								if(GoBoard[nxt]==0 && !(GenPattern4[k].map[l][r]&4)) { f=1; break; }
								if(GoBoard[nxt]==1 && !(GenPattern4[k].map[l][r]&(j^1))) { f=1; break; }
								if(GoBoard[nxt]==2 && !(GenPattern4[k].map[l][r]&(j^2))) { f=1; break; }
								if(GoBoard[nxt]==-1 && !(GenPattern4[k].map[l][r]&8)) { f=1; break; }
							}
							if(f) break;
						}
						if(f == 0)
						{
							plist[num].move = mv;
							num ++;
							flag = 1;
							break;
						}
					}
				}
				if(flag) break;
			}
		}
		break;
	case 2:
		for(i=1; i<=BoardSize; i++)
		{
			for(j=1; j<=BoardSize; j++)
			{
				mv = letsgo_get_p(i, j);
				if(GoBoard[mv] == 0)
				{
					plist[num].move = mv;
					num ++;
				}
			}
		}
		break;
	}
	return num;
}

#include<conio.h> //for debug

int search_mc_play(int depth, int side, bool pass)
{
	struct GENLIST *plist = GenList[depth];
	int num = 0;
	int stage;
	int val;
	int mv, p;
	if(depth == LETSGO_MAX_DEPTH-1) return SEARCH_VALUE_UNKNOWN;
	
	//letsgo_board_draw(); printf("- %d %d %d %d", (int)pass, side, PieceCount[1], PieceCount[2]); getch();

	for(stage=0; stage<3; stage+=2)
	{
		num = search_move_gen(plist, depth, side, stage);
		while(num > 0)
		{
			int t = letsgo_last_move();
			p = utility_rand_get() % num;
			mv = plist[p].move;
			if(search_is_eye(mv, side) /*|| search_is_badmove(mv, side)*/ || letsgo_make_move(depth+1, side, mv)==0)
			{
				plist[p] = plist[num-1];
				num --;
				continue;
			}
			//letsgo_board_draw(); printf("- %d %d %d %d stage=%d %d %d", (int)pass, side, PieceCount[1], PieceCount[2], stage, letsgo_get_x(t), letsgo_get_y(t)); getch();
			val = -search_mc_play(depth+1, letsgo_get_op(side), false);
			letsgo_unmake_move();
			break;
		}
		if(num != 0) break;
	}
	if(stage==4)
	{
		if(pass)
		{
			val = search_evaluation(side);
			//letsgo_board_draw(); printf("%d %d %d %d", (int)pass, side, PieceCount[1], PieceCount[2]); getch();
		}
		else
		{
			letsgo_make_move(depth+1, side, -1);
			val = -search_mc_play(depth+1, letsgo_get_op(side), true);
			letsgo_unmake_move();
		}		
	}
	return val;
}

struct UCTNODE
{
	int mov;
	int val;
	int cnt;
	int l, r;
};
struct UCTNODE UCTTree[SEARCH_UCT_TREE_MAX_SIZE];
int UCTTreeP;

#include<conio.h> //for debug
int search_uct_play(int depth, int side, struct UCTNODE *p)
{
	double bestv = -INF;
	int bestn = -1;
	int val;
	int i, j;
	//letsgo_board_draw(); printf("%d %d %d %d", depth, side, PieceCount[1], PieceCount[2]); getch();
	if(p->cnt == 0)
	{
		int t;
		val = 0;
		for(i=0; i<SEARCH_UCT_A; i++)
		{
			t = search_mc_play(depth, side, false);
			if(t == SEARCH_VALUE_UNKNOWN)
			{
				i --;
				continue;
			}
			val += t;
		}
		p->val += val;
		p->cnt += SEARCH_UCT_A;
		return p->val;
	}
	if(p->l == 0)
	{
		int nxt;
		p->l = UCTTreeP;
		for(i=1; i<=BoardSize; i++)
		{
			for(j=1; j<=BoardSize; j++)
			{
				nxt = letsgo_get_p(i, j);
				if(GoBoard[nxt]!=0 || search_is_eye(nxt, side)) continue;
				if(letsgo_check_forbid(depth+1, side, nxt) /*|| search_is_badmove(nxt, side)*/) continue;
				UCTTree[UCTTreeP].mov = nxt;
				UCTTree[UCTTreeP].cnt = 0;
				UCTTree[UCTTreeP].val = 0;
				UCTTree[UCTTreeP].l = UCTTree[UCTTreeP].r = 0;
				UCTTreeP ++;
			}
		}
		if(p->mov != -1 && UCTTreeP == p->l)
		{
			UCTTree[UCTTreeP].mov = -1;
			UCTTree[UCTTreeP].cnt = 0;
			UCTTree[UCTTreeP].val = 0;
			UCTTree[UCTTreeP].l = UCTTree[UCTTreeP].r = 0;
			UCTTreeP ++;
		}
		p->r = UCTTreeP;
	}
	for(i=p->l; i<p->r; i++)
	{
		double v;
		if(UCTTree[i].cnt == 0)
			v = 10000.0 + 1000.0 * utility_rand_get() / 32767.0;
		else
			v = -(double)UCTTree[i].val/((double)UCTTree[i].cnt*BoardSize*BoardSize) + SEARCH_UCT_K * sqrt(log((double)p->cnt) / (double)UCTTree[i].cnt);
		if(v > bestv)
		{
			bestv = v;
			bestn = i;
		}
	}
	if(bestn != -1)
	{
		letsgo_make_move(depth+1, side, UCTTree[bestn].mov);
		val = -search_uct_play(depth+1, letsgo_get_op(side), &UCTTree[bestn]);
		letsgo_unmake_move();
		p->val += val;
		p->cnt += SEARCH_UCT_A;
	}
	else
	{
		val = search_evaluation(side) * SEARCH_UCT_A;
		p->cnt += SEARCH_UCT_A;
	}
	return val;
}

int search_uct_main(int depth, int side, int *bestm)
{
	int k;
	int i;
	double bestv = -INF;
	*bestm = -1;
	UCTTree[0].cnt = 0;
	UCTTree[0].val = 0;
	UCTTree[0].l = UCTTree[0].r = 0;
	UCTTreeP = 1;

	for(k=0; k<90000 && UCTTreeP<SEARCH_UCT_TREE_MAX_SIZE-BoardSize*BoardSize; k++)
	{
		if((k & 1023) == 0)
		{
			if(utility_clock() - TimeUsed >= TimeLimit) break;
		}
		search_uct_play(depth, side, &UCTTree[0]);
	} 
	for(i=UCTTree[0].l; i<UCTTree[0].r; i++)
	{
		double v = -(double)UCTTree[i].val/(double)UCTTree[i].cnt;
		//printf("%d %d\n", UCTTree[i].val, UCTTree[i].cnt);
		if(v > bestv)
		{
			bestv = v;
			*bestm = UCTTree[i].mov;
		}
	}
	return (int)bestv;
}

int search_alphabeta()
{
	return 0;
}

struct BOOKLIST
{
	short size;
	short move;
	short number;
	int p;
};

struct BOOKLIST BookList[SEARCH_BOOK_MAX_SIZE];
int BookListNum;
int	BookListRec[SEARCH_BOOK_REC_MAX_SIZE];
int BookListRecNum;

void search_book_init()
{
	static bool flag = false;
	FILE *in;
	char x;
	int y;
	int num;

	if(flag) return;
	flag = true;

	if((in = fopen("book.dat", "r")) == NULL) return;
	while(fscanf(in, "%d", &BookList[BookListNum].size) != EOF)
	{
		BookList[BookListNum].p = BookListRecNum;
		num = 0;
		while(fscanf(in, " %c", &x) != EOF)
		{
			if(x == '|')
			{
				fscanf(in, " %c%d", &x, &y);
				BookList[BookListNum].move = letsgo_get_p(x-'A'+1, y);
				BookList[BookListNum].number = num;
				break;
			}
			else
			{
				fscanf(in, "%d", &y);
				BookListRec[BookListRecNum] = letsgo_get_p(x-'A'+1, y);
				BookListRecNum ++;
				if(BookListRecNum >= SEARCH_BOOK_REC_MAX_SIZE)
				{
					printf("ERROR: openbook is too large (r)\n");
					fclose(in);
					return;
				}
				num ++;
			}
		}
		BookListNum ++;
		if(BookListNum >= SEARCH_BOOK_MAX_SIZE)
		{
			printf("ERROR: openbook is too large\n");
			fclose(in);
			return;
		}
	}
	fclose(in);
}

int search_book_lookup(int side, int *bestm)
{
	int i, k;
	for(i=0; i<BookListNum; i++)
	{
		if(BookList[i].size==BoardSize && BookList[i].number==PieceCount[1]+PieceCount[2])
		{
			for(k=0; k<8; k++)
			{
				bool f = true;
				int p = BookList[i].p+BookList[i].number-1;
				int s = side;
				while(p >= BookList[i].p)
				{
					s = letsgo_get_op(s);
					if(GoBoard[TransBoard[k][BookListRec[p]]] != s)
					{
						f = false;
						break;
					}
					p --;
				}
				if(f)
				{
					*bestm = TransBoard[k][BookList[i].move];
					return 1;
				}
			}
		}
	}
	return 0;
}