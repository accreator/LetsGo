#define SEARCH_VALUE_UNKNOWN 10000
#define SEARCH_UCT_TREE_MAX_SIZE 3000000

#define SEARCH_BOOK_MAX_SIZE 50000
#define SEARCH_BOOK_REC_MAX_SIZE (SEARCH_BOOK_MAX_SIZE * 30)

#define SEARCH_UCT_A 2
#define SEARCH_UCT_K 1.0

extern int UCTTreeP;

bool search_is_eye(int p, int side);
bool search_is_badmove(int p, int side);

int search_moku_count(int side);
int search_evaluation(int side);
void search_vis_maintain(int vis[], int &visflag);
void search_pattern_init();
int search_mc_play(int depth, int side, bool pass);

int search_uct_play(int depth, int side, struct UCTNODE *p);
int search_uct_main(int depth, int side, int *bestm);

void search_book_init();
int search_book_lookup(int side, int *bestm);
