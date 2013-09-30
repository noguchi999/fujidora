#ifndef __fujidora__GameScene__
#define __fujidora__GameScene__

#include "cocos2d.h"
#include "Config.h"
#include "BlockSprite.h"

#include <math.h>

using namespace std;
using namespace cocos2d;

const int kMaxBlockLeft        = 8;
const int kMaxBlockTop         = 8;
const int kBlockPurgeThreshold = 3;

const float kMovingTime           = 0.4f;
const float kRemovingTime         = 0.3f;
const float kParticalRemovingTime = 0.5f;

const char* const kHighScore = "HighScore";

const char* const kImgBackground = "background.png";
const char* const kImgGameOver   = "gameover.png";
const char* const kImgResetOut   = "reset1.png";
const char* const kImgResetOn    = "reset2.png";

const char* const kSEMoveBlock   = "moveBlock.mp3";
const char* const kSERemoveBlock = "ta_ge_kotaiko02.mp3";

const char* const kBGMMain = "bgm_main.mp3";

const char* const kFontRed    = "redFont.fnt";
const char* const kFontBlue   = "blueFont.fnt";
const char* const kFontYellow = "yellowFont.fnt";
const char* const kFontGreen  = "greenFont.fnt";
const char* const kFontGray   = "grayFont.fnt";
const char* const kFontWhite  = "whiteFont.fnt";

enum kTag
{
    kTagSweptBlock = 0,
    kTagBackground = 1,
    kTagRedLabel,
    kTagBlueLabel,
    kTagYellowLabel,
    kTagGreenLabel,
    kTagGrayLabel,
    kTagScoreLabel,
    kTagGameOver,
    kTagHighScoreLabel,
    kTagBaseBlock = 10000
};

enum kZOrder
{
    kZOrderBackground,
    kZOrderLabel,
    kZOrderBlock,
    kZOrderPartical,
    kZOrderGameOver
};

class GameScene : public CCLayer
{
public:
    GameScene();

    virtual bool init();
    static CCScene* scene();
    CREATE_FUNC(GameScene);
    
    virtual bool ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent);
    virtual void ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent);
    virtual void ccTouchEnded(CCTouch* pTouch, CCEvent* pEvent);

private:
    // ToDo BlockSpriteを直接返すように変更すること
    struct TouchedBlock
    {
        TouchedBlock(int _tag, kBlock _type)
        {
            tag  = _tag;
            type = _type;
        }
        int    tag;
        kBlock type;
    };
    
    struct BlockFiledsPositionIndex
    {
        BlockFiledsPositionIndex(int _x, int _y)
        {
            x = _x;
            y = _y;
        }
        int x;
        int y;
    };
    
    CCSprite* background;
    float blockSize;
    vector<kBlock> blockTypes;
    int score;
    int currentTag;
    int tmpCurrentTag;
    int previousTag;
    map<kBlock, int> removedBlockTypesCounter;
    vector<vector<BlockSprite*> > blockFields;
    vector<BlockSprite*> removedBlocks;
    vector<int> removedTags;
    CCSize winSize;
    CCPoint blockAreaStartPoint;
    CCPoint blockAreaEndPoint;

    void initForVariables();
    void createBackground();
    void createBlock();
    void createHighScoreLabel();
    void createResetButton();

    CCPoint getPosition(int posIndexX, int posIndexY);
    TouchedBlock getTouchedBlock(CCPoint touch_point);
    void saveHighScore();
    void menuResetCallback(CCObject* pSender);
    CCNode* createParticle(const char* plistFile, const CCPoint &tagPoint);
    void removingParticle(CCNode* particle);
    void landLeveling(int top, int left, kBlock block_type, int* counter);
    void removeTempTag(CCNode* sender, void* target_tag);
    BlockFiledsPositionIndex getBlockFieldsPositionIndex(int tag);
    void prependRemoveBlocks();
    void removeBlocksActionAnimation(CCNode* node);
    void removeBlocksAction();
    void removeBlocks();
    void moveBlocks();
    void appendBlocks();
    void recursiveRemoveBlocks();
    void removeBlockAnimation();
    void animationStart();
    void animationEnd();
    vector<BlockFiledsPositionIndex> horizontalBlockCounting(int left, int top, kBlock block_type);
    vector<BlockFiledsPositionIndex> crossVerticalBlockCounting(int left, int top, kBlock block_type);
    vector<BlockFiledsPositionIndex> verticalBlockCounting(int left, int top, kBlock block_type);
    vector<BlockFiledsPositionIndex> crossHorizontalBlockCounting(int left, int top, kBlock block_type);
    int crossBlockCounting(int left, int top, kBlock block_type);
    bool isReverseSwipe(CCTouch* pTouch);
};

#endif /* defined(__fujidora__GameScene__) */
