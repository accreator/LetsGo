#define VERSION "0.0.5"

#define INF 1000000000

typedef __int64 I64;
typedef unsigned __int64 U64;
typedef int I32;
typedef unsigned int U32;
typedef short I16;
typedef unsigned short U16;
typedef char I8;
typedef unsigned char U8;

#define LETSGO_MAX_DEPTH 4096

inline int letsgo_get_x(int p) { return p >> 5; }
inline int letsgo_get_y(int p) { return p & 31; }
inline int letsgo_get_p(int x, int y) { return (x<<5)|y; }
inline int letsgo_get_op(int s) { return s^3; }

extern const int MoveP[8];
extern const int MoveP2[3][3];
extern int BoardSize;
extern int GoBoard[32*32];
extern int LifeBoard[32*32];
extern int PieceCount[3];
extern int Round;
extern int PositionType[32*32];
extern int TransBoard[8][32*32];
extern U64 ZobKey[LETSGO_MAX_DEPTH];

extern int TimeLimit;
extern long TimeUsed;
extern int CurrentVal;

int letsgo_last_move();

void letsgo_life_update(U64 &key, int move);
bool letsgo_check_forbid(int depth, int side, int move);
int letsgo_make_move(int depth, int side, int move);
void letsgo_unmake_move();

void letsgo_init();
void letsgo_board_draw();