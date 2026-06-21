//SOUND FACTORY------------------

#define SF_TURN_ON 1
#define SF_TURN_OFF 2
#define SF_EMPTY 3

typedef struct soundFactory{
    struct soundNode *soundlistHead;
    struct soundNode *soundlistZero;
} SOUNDFACTORY;


//External Functions
void soundFactoryInitialize(struct soundFactory *t);
void soundFactoryModify(int tone, int mode, struct soundFactory *x);
double soundFactoryProduce(struct soundFactory *x);

LRESULT CALLBACK soundFactoryCallbackWindow(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


