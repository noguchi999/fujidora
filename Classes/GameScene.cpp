#include "GameScene.h"
#include "SimpleAudioEngine.h"
#include "CCPlaySE.h"

using namespace cocos2d;
using namespace CocosDenshion;
using namespace std;

GameScene::GameScene()
:currentTag(0)
,tmpCurrentTag(0)
,previousTag(0)
,score(0)
,isAnimating(false)
,winSize(CCDirector::sharedDirector()->getWinSize())
{
    srand((unsigned)time(NULL));
}

CCScene* GameScene::scene()
{
    CCScene* scene   = CCScene::create();
    GameScene* layer = GameScene::create();
    scene->addChild(layer);
    
    return scene;
}

bool GameScene::init()
{
    if(!CCLayer::init())
    {
        return false;
    }
    
    setTouchEnabled(true);
    setTouchMode(kCCTouchesOneByOne);

    initForVariables();
    createBackground();
    //createBlock();
    testCreateBlock();
    createHighScoreLabel();
    createResetButton();
    
    SimpleAudioEngine::sharedEngine()->preloadEffect(kSEMoveBlock);
    SimpleAudioEngine::sharedEngine()->preloadEffect(kSERemoveBlock);
    SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic(kBGMMain);
    
    SimpleAudioEngine::sharedEngine()->playBackgroundMusic(kBGMMain, true);
    
    return true;
}

bool GameScene::ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent)
{    
    if (currentTag == 0)
    {
        CCPoint touchPoint = background->convertTouchToNodeSpace(pTouch);
        TouchedBlock touched_block = getTouchedBlock(touchPoint);
        if (touched_block.tag != 0)
        {
            currentTag = touched_block.tag;
            CCNode* current = background->getChildByTag(currentTag);
            current->setVisible(false);
            
            tmpCurrentTag = currentTag + kTagBaseBlock;
            BlockSprite* block = BlockSprite::create(tmpCurrentTag, touched_block.type, kStatusNormal);
            block->setPosition(current->getPosition());
            block->setScale(1.2f);
            block->setOpacity(128);
            block->setAnchorPoint(ccp(0.3, 0.3));
            background->addChild(block, kZOrderBlock+1, tmpCurrentTag);
        }
    }
    
    return !isAnimating;
}

void GameScene::ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent)
{
    CCPoint touchPoint = background->convertTouchToNodeSpace(pTouch);
    TouchedBlock touched_block = getTouchedBlock(touchPoint);
    if (currentTag != 0 && touchPoint.x >= (blockAreaStartPoint.x - blockSize / 2) && touchPoint.y >= (blockAreaStartPoint.y - blockSize / 2) && touchPoint.x <= (blockAreaEndPoint.x + blockSize / 2) && touchPoint.y <= (blockAreaEndPoint.y + blockSize / 2))
    {
        CCNode* tmp_current = background->getChildByTag(tmpCurrentTag);
        tmp_current->setPosition(touchPoint);
        if (touched_block.tag != 0 && touched_block.tag != previousTag)
        {
            previousTag = touched_block.tag;
            CCNode* current = background->getChildByTag(currentTag);
            CCNode* target  = background->getChildByTag(touched_block.tag);

            // ブロックを入れ替える
            BlockFiledsPositionIndex target_position_idex  = getBlockFieldsPositionIndex(touched_block.tag);
            BlockFiledsPositionIndex current_position_idex = getBlockFieldsPositionIndex(currentTag);
            blockFields[target_position_idex.x][target_position_idex.y]   = (BlockSprite*)current;
            blockFields[current_position_idex.x][current_position_idex.y] = (BlockSprite*)target;

            CCPoint prePoint = getPosition(current_position_idex.x, current_position_idex.y);
            current->setPosition(getPosition(target_position_idex.x, target_position_idex.y));
            
            CCLog("ccTouchMoved currentTag: %i, touched_block_tag: %i", currentTag, touched_block.tag);
            CCLog("ccTouchMoved CurrentX: %i, CurrentY: %i", current_position_idex.x, current_position_idex.y);
            CCLog("ccTouchMoved prePointX: %f, prePointY: %f", prePoint.x, prePoint.y);
            
            CCMoveTo* targetMove = CCMoveTo::create(0.2f, prePoint);
            CCPlaySE* playSe = CCPlaySE::create(kSEMoveBlock);
            CCFiniteTimeAction* moveAction = CCSpawn::create(targetMove, playSe, NULL);
            
            target->runAction(moveAction);
        }
    }
}

void GameScene::ccTouchEnded(CCTouch* pTouch, CCEvent* pEvent)
{
    if (currentTag != 0)
    {
        background->removeChildByTag(tmpCurrentTag);
        CCNode* current = background->getChildByTag(currentTag);
        current->setVisible(true);
        background->reorderChild(current, kZOrderBlock);
        
        prependRemoveBlocks();
        removeBlockAnimation();
    }
    
    currentTag  = 0;
    previousTag = 0;
}

void GameScene::initForVariables()
{
    blockSize = BlockSprite::create(kTagBaseBlock, kBlockRed, kStatusNormal)->getContentSize().height;
    
    blockTypes.push_back(kBlockRed);
    blockTypes.push_back(kBlockBlue);
    blockTypes.push_back(kBlockYellow);
    blockTypes.push_back(kBlockGreen);
    blockTypes.push_back(kBlockGray);
    
    removedBlockTypesCounter.insert(make_pair(kBlockRed, 0));
    removedBlockTypesCounter.insert(make_pair(kBlockBlue, 0));
    removedBlockTypesCounter.insert(make_pair(kBlockYellow, 0));
    removedBlockTypesCounter.insert(make_pair(kBlockGreen, 0));
    removedBlockTypesCounter.insert(make_pair(kBlockGray, 0));
}

void GameScene::createBackground()
{
    background = CCSprite::create(kImgBackground);
    background->setPosition(ccp(winSize.width / 2, winSize.height / 2));
    addChild(background, kZOrderBackground, kTagBackground);    
}

void GameScene::createBlock()
{
    int tag = kTagBaseBlock;
    for (int x = 0; x < kMaxBlockLeft; x++)
    {
        vector<BlockSprite*> blocks;
        for (int y = 0; y < kMaxBlockTop; y++)
        {
            kBlock blockType = (kBlock)(rand() % kBlockCount);
            BlockSprite* block = BlockSprite::create(tag, blockType, kStatusNormal);
            block->setPosition(getPosition(x, y));
            background->addChild(block, kZOrderBlock, tag);
            blocks.push_back(block);
            
            tag++;
        }
        blockFields.push_back(blocks);
    }
    
    blockAreaStartPoint = getPosition(0, 0);
    blockAreaEndPoint   = getPosition(kMaxBlockLeft-1, kMaxBlockTop-1);
}

void GameScene::createHighScoreLabel()
{
    CCSize bgSize = background->getContentSize();
    int highScore = CCUserDefault::sharedUserDefault()->getIntegerForKey(kHighScore, 0);
    const char* highScoreStr = ccsf("%d", highScore);
    CCLabelBMFont* highScoreLabel = (CCLabelBMFont*)background->getChildByTag(kTagHighScoreLabel);
    if(!highScoreLabel)
    {
        highScoreLabel = CCLabelBMFont::create(highScoreStr, kFontWhite);
        highScoreLabel->setPosition(ccp(bgSize.width * 0.78, bgSize.height * 0.87));
        background->addChild(highScoreLabel, kZOrderLabel, kTagHighScoreLabel);
    }
    else
    {
        highScoreLabel->setString(highScoreStr);
    }
}

void GameScene::createResetButton()
{
    CCSize bgSize = background->getContentSize();
    
    CCMenuItemImage* resetButton = CCMenuItemImage::create(kImgResetOut, kImgResetOn, this, menu_selector(GameScene::menuResetCallback));
    resetButton->setPosition(ccp(bgSize.width * 0.78, bgSize.height * 0.1));
    
    CCMenu* menu = CCMenu::create(resetButton, NULL);
    menu->setPosition(CCPointZero);
    background->addChild(menu);
}

CCPoint GameScene::getPosition(int posIndexX, int posIndexY)
{
    float offsetX = background->getContentSize().width  * 0.168;
    float offsetY = background->getContentSize().height * 0.029;
    
    return CCPoint((posIndexX + 0.5) * blockSize + offsetX, (posIndexY + 0.5) * blockSize + offsetY);
}

GameScene::TouchedBlock GameScene::getTouchedBlock(CCPoint touchPoint)
{
    for (std::vector<std::vector<BlockSprite*> >::const_iterator it = blockFields.begin(); it != blockFields.end(); ++it)
    {
        for (vector<BlockSprite*>::const_iterator block = (*it).begin(); block != (*it).end(); ++block)
        {
            if (currentTag != 0 && (*block)->getTag() == currentTag)
            {
                continue;
            }

            if ((*block)->boundingBox().containsPoint(touchPoint))
            {
                return TouchedBlock((*block)->getTag(), (*block)->getType());
            }
        }
    }
    
    return TouchedBlock(0, (new BlockSprite())->getType());
}

cocos2d::CCNode* GameScene::createParticle(const char *plistFile, const cocos2d::CCPoint &tagPoint)
{
    CCParticleSystemQuad* pParticle = CCParticleSystemQuad::create(plistFile);
    pParticle->setPosition(tagPoint);
    background->addChild(pParticle, kZOrderPartical);
    
    return pParticle;
}

void GameScene::removingParticle(cocos2d::CCNode *particle)
{
    particle->removeFromParentAndCleanup(true);
}

void GameScene::saveHighScore()
{
    CCUserDefault* userDefault = CCUserDefault::sharedUserDefault();
    
    int oldHighScore = userDefault->getIntegerForKey(kHighScore, 0);
    if (oldHighScore < score)
    {
        userDefault->setIntegerForKey(kHighScore, score);
        userDefault->flush();
        
        createHighScoreLabel();
    }
}

void GameScene::menuResetCallback(cocos2d::CCObject* pSender)
{
    GameScene* scene = GameScene::create();
    CCDirector::sharedDirector()->replaceScene((CCScene*)scene);
}

GameScene::BlockFiledsPositionIndex GameScene::getBlockFieldsPositionIndex(int tag)
{    
    for (int x = 0; x < kMaxBlockLeft; x++)
    {
        for (int y = 0; y < kMaxBlockTop; y++)
        {
            if (blockFields[x][y]->getTag() == tag)
            {
                return BlockFiledsPositionIndex(x, y);
            }
        }
    }
    
    return BlockFiledsPositionIndex(0, 0);
}

void GameScene::removeBlocksActionAnimation(CCNode* node)
{
    BlockSprite* blockSprite = (BlockSprite*)node;
    
    blockSprite->removeFromParentAndCleanup(true);
    blockSprite->release();
    blockSprite = BlockSprite::create(blockSprite->getTag(), blockSprite->getType(), kStatusSwept);
}

void GameScene::removeBlocksAction()
{
    bool isFirst = true;
    for (vector<BlockSprite*>::const_iterator it = removedBlocks.begin(); it != removedBlocks.end(); ++it)
    {
        int x = (*it)->getNexPositionX();
        int y = (*it)->getNexPositionY();
        
        CCScaleTo* shrinkBlock    = CCScaleTo::create(0.2f, 0);
        CCCallFuncN* removeBlocksActionAnimationFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::removeBlocksActionAnimation));
        CCFiniteTimeAction* shrinkBlockSequence = CCSequence::create(shrinkBlock, removeBlocksActionAnimationFunction, NULL);
        
        CCScaleTo* burst   = CCScaleTo::create(0.3f, 2.5f);
        CCScaleTo* clear   = CCScaleTo::create(0.3f, 1);
        CCCallFuncN* removingParticleFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::removingParticle));
        CCFiniteTimeAction* shrinkParticalSequence = CCSequence::create(burst, clear, removingParticleFunction, NULL);
        CCNode* partial = createParticle("lizi.plist", blockFields[x][y]->getPosition());
        
        if (isFirst) {
            CCPlaySE* playSe = CCPlaySE::create(kSERemoveBlock);
            shrinkBlockSequence = CCSpawn::create(shrinkBlockSequence, playSe, NULL);
            isFirst = false;
        }
        
        blockFields[x][y]->runAction(shrinkBlockSequence);
        partial->runAction(shrinkParticalSequence);
    }
}

void GameScene::removeBlocks()
{
    for (int x = 0; x < kMaxBlockLeft; ++x)
    {
        for (int y = 0; y < kMaxBlockTop; ++y)
        {
            if (blockFields[x][y]->getStatus() == kStatusMarked)
            {
                BlockSprite* removedBlock = BlockSprite::create(blockFields[x][y]->getTag(), blockFields[x][y]->getType(), blockFields[x][y]->getStatus());
                removedBlock->setNexPositionX(x);
                removedBlock->setNexPositionY(y);
                removedBlocks.push_back(removedBlock);
                removedBlockTypesCounter[blockFields[x][y]->getType()] += 1;
            }
        }
    }
}

void GameScene::moveBlocks()
{
    map<int, BlockSprite*> moveBlockList;
    for (vector<BlockSprite*>::const_iterator it = removedBlocks.begin(); it != removedBlocks.end(); ++it)
    {
        BlockFiledsPositionIndex to = getBlockFieldsPositionIndex((*it)->getTag());
        int x = to.x;
        for (int y = to.y; y < kMaxBlockTop-1; ++y) {
            blockFields[x][y] = blockFields[x][y+1];
            blockFields[x][y]->retain();
            if (blockFields[x][y]->getStatus() == kStatusNormal) {
                BlockSprite* blockSprite = blockFields[x][y];
                blockSprite->setNexPositionX(x);
                blockSprite->setNexPositionY(y);
                moveBlockList.insert(make_pair(blockSprite->getTag(), blockSprite));
            }
        }
        blockFields[x][kMaxBlockTop-1] = BlockSprite::create(kTagSweptBlock, kBlockRed, kStatusSwept);
    }
    
    for (map<int, BlockSprite*>::const_iterator it = moveBlockList.begin(); it != moveBlockList.end(); ++it)
    {
        CCDelayTime* delay = CCDelayTime::create(0.8f);
        CCFiniteTimeAction* move = CCMoveTo::create(0.4f, getPosition((*it).second->getNexPositionX(), (*it).second->getNexPositionY()));
        CCSequence* action = CCSequence::createWithTwoActions(delay, move);
        (*it).second->runAction(action);
    }
}

void GameScene::appendBlocks()
{
    for (int x = 0; x < kMaxBlockLeft; ++x)
    {
        for (int y = 0; y < kMaxBlockTop; ++y)
        {
            if (blockFields[x][y]->getStatus() == kStatusSwept)
            {
                int tag = removedBlocks.back()->getTag();
                removedBlocks.pop_back();
                
                kBlock blockType   = (kBlock)(rand() % kBlockCount);
                BlockSprite* block = BlockSprite::create(tag, blockType, kStatusNormal);
                block->setPosition(getPosition(x, kMaxBlockTop));
                background->addChild(block, kZOrderBlock, tag);
                
                CCActionInterval* move = CCMoveTo::create(0.4f, getPosition(x, y));
                CCActionInterval* easeInOut = CCEaseInOut::create(move, 1.25f);
                block->runAction(easeInOut);

                blockFields[x][y] = block;
            }
        }
    }
}

void GameScene::recursiveRemoveBlocks()
{
    bool isMarked = false;
    prependRemoveBlocks();
    for (int x = 0; x < kMaxBlockLeft; ++x)
    {
        for (int y = 0; y < kMaxBlockTop; ++y)
        {
            if (blockFields[x][y]->getStatus() == kStatusMarked)
            {
                isMarked = true;
            }
        }
    }
    
    if (isMarked) {
        scheduleOnce(schedule_selector(GameScene::removeBlockAnimation), 0.5f);
    }
}

void GameScene::removeBlockAnimation()
{
    CCDelayTime* delay = CCDelayTime::create(1.6f);
    CCCallFuncN* removeBlocksFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::removeBlocks));
    CCCallFuncN* removeBlocksActionFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::removeBlocksAction));
    CCCallFuncN* moveBlocksFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::moveBlocks));
    CCCallFuncN* appendBlocksFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::appendBlocks));
    CCCallFuncN* recursiveRemoveBlocksFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::recursiveRemoveBlocks));
    CCCallFuncN* animationStartFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::animationStart));
    CCCallFuncN* animationEndFunction = CCCallFuncN::create(this, callfuncN_selector(GameScene::animationEnd));
    CCFiniteTimeAction* blockActionSequence = CCSequence::create(animationStartFunction, removeBlocksFunction, removeBlocksActionFunction, moveBlocksFunction, delay, appendBlocksFunction, recursiveRemoveBlocksFunction, animationEndFunction, NULL);
    
    this->runAction(blockActionSequence);
}

void GameScene::animationStart()
{
    setTouchEnabled(false);
}

void GameScene::animationEnd()
{
    setTouchEnabled(true);
}

vector<GameScene::BlockFiledsPositionIndex> GameScene::horizontalBlockCounting(int left, int top, kBlock block_type) {
    vector<BlockFiledsPositionIndex> flushPoints;
    if (blockFields[left][top]->getStatus() == kStatusMarked) {
        flushPoints.push_back(BlockFiledsPositionIndex(left, top));
    }
    
    for (int i = left; i < kMaxBlockLeft; ++i)
    {
        if (blockFields[i][top]->getStatus() == kStatusNormal && blockFields[i][top]->getType() == block_type)
        {
            flushPoints.push_back(BlockFiledsPositionIndex(i, top));
        }
        else
        {
            break;
        }
    }

    for (int i = left-1; i >= 0; --i)
    {
        if (blockFields[i][top]->getStatus() == kStatusNormal && blockFields[i][top]->getType() == block_type)
        {
            flushPoints.push_back(BlockFiledsPositionIndex(i, top));
        }
        else
        {
            break;
        }
    }
    return flushPoints;
}

//垂直方向に進みながら水平方向のブロックの数を数える
vector<GameScene::BlockFiledsPositionIndex> GameScene::crossVerticalBlockCounting(int left, int top, kBlock block_type)
{
    vector<BlockFiledsPositionIndex> flushPoints;

    for (int i = top; i < kMaxBlockTop; ++i)
    {
        if (blockFields[left][i]->getType() == block_type)
        {
            vector<BlockFiledsPositionIndex> horizontalFlushPoints = horizontalBlockCounting(left, i, block_type);
            if (horizontalFlushPoints.size() >= kBlockPurgeThreshold)
            {
                flushPoints.insert(flushPoints.end(), horizontalFlushPoints.begin(), horizontalFlushPoints.end());
            }
        }
        else
        {
            break;
        }
    }

    for (int i = top-1; i >= 0; --i)
    {
        if (blockFields[left][i]->getType() == block_type)
        {
            vector<BlockFiledsPositionIndex> horizontalFlushPoints = horizontalBlockCounting(left, i, block_type);
            if (horizontalFlushPoints.size() >= kBlockPurgeThreshold)
            {
                flushPoints.insert(flushPoints.end(), horizontalFlushPoints.begin(), horizontalFlushPoints.end());
            }
        }
        else
        {
            break;
        }
    }
    return flushPoints;
}

vector<GameScene::BlockFiledsPositionIndex> GameScene::verticalBlockCounting(int left, int top, kBlock block_type)
{
    vector<BlockFiledsPositionIndex> flushPoints;
    if (blockFields[left][top]->getStatus() == kStatusMarked) {
        flushPoints.push_back(BlockFiledsPositionIndex(left, top));
    }
    
    for (int i = top; i < kMaxBlockTop; ++i)
    {
        if (blockFields[left][i]->getStatus() == kStatusNormal && blockFields[left][i]->getType() == block_type)
        {
            flushPoints.push_back(BlockFiledsPositionIndex(left, i));
        }
        else
        {
            break;
        }
    }

    for (int i = top-1; i >= 0; --i)
    {
        if (blockFields[left][i]->getStatus() == kStatusNormal && blockFields[left][i]->getType() == block_type)
        {
            flushPoints.push_back(BlockFiledsPositionIndex(left, i));
        }
        else
        {
            break;
        }
    }
    return flushPoints;
}

//水平方向に進みながら垂直方向のブロックの数を数える
vector<GameScene::BlockFiledsPositionIndex> GameScene::crossHorizontalBlockCounting(int left, int top, kBlock block_type)
{
    vector<BlockFiledsPositionIndex> flushPoints;
    for (int i = left; i < kMaxBlockLeft; ++i)
    {
        if (blockFields[i][top]->getType() == block_type)
        {
            vector<BlockFiledsPositionIndex> verticalFlushPoints = verticalBlockCounting(i, top, block_type);
            if (verticalFlushPoints.size() >= kBlockPurgeThreshold)
            {
                flushPoints.insert(flushPoints.end(), verticalFlushPoints.begin(), verticalFlushPoints.end());
            }
        }
        else
        {
            break;
        }
    }

    for (int i = left-1; i >= 0; --i)
    {
        if (blockFields[i][top]->getType() == block_type)
        {
            vector<BlockFiledsPositionIndex> verticalFlushPoints = verticalBlockCounting(i, top, block_type);
            if (verticalFlushPoints.size() >= kBlockPurgeThreshold)
            {
                flushPoints.insert(flushPoints.end(), verticalFlushPoints.begin(), verticalFlushPoints.end());
            }
        }
        else
        {
            break;
        }
    }
    return flushPoints;
}

int GameScene::crossBlockCounting(int left, int top, kBlock block_type)
{
    vector<BlockFiledsPositionIndex> crossHorizontalFlushPoints = crossHorizontalBlockCounting(left, top, block_type);
    vector<BlockFiledsPositionIndex> crossVerticalFlushPoints   = crossVerticalBlockCounting(left, top, block_type);
    if (crossHorizontalFlushPoints.size() >= kBlockPurgeThreshold)
    {
        for (vector<BlockFiledsPositionIndex>::const_iterator it = crossHorizontalFlushPoints.begin(); it != crossHorizontalFlushPoints.end(); ++it)
        {
            blockFields[it->x][it->y]->setStatus(kStatusMarked);
        }
    }

    if (crossVerticalFlushPoints.size() >= kBlockPurgeThreshold)
    {
        for (vector<BlockFiledsPositionIndex>::const_iterator it = crossVerticalFlushPoints.begin(); it != crossVerticalFlushPoints.end(); ++it)
        {
            blockFields[it->x][it->y]->setStatus(kStatusMarked);
        }
    }
    // kBlockPurgeThresholdを超えているかを知りたいだけなので、別に比較する意味はない・・・
    return max(crossHorizontalFlushPoints.size(), crossVerticalFlushPoints.size());
}

void GameScene::prependRemoveBlocks()
{
    for (int x = 0; x < kMaxBlockLeft; ++x)
    {
        for (int y = 0; y < kMaxBlockTop; ++y)
        {
            if (blockFields[x][y]->getTag() == 0) {
                continue;
            }
            
            int counter;
            counter = crossBlockCounting(x, y, blockFields[x][y]->getType());
            if (counter >= kBlockPurgeThreshold)
            {
                CCLog("prependRemoveBlocks x: %i, y: %i", x, y);
                prependRemoveBlocks();
            }
        }
    }
}

void GameScene::removeTempTag(CCNode* sender, void* target_tag)
{
    CCNode* target = background->getChildByTag((int)target_tag);
    background->removeChild(sender, true);
    target->setVisible(true);
}

void GameScene::testCreateBlock()
{
	int tag = kTagBaseBlock;
    
	CsvReader* csv = new CsvReader("/Users/NoguchiOsamu/Dropbox/ios_farm/Book1.csv");
	vector<vector<string> > testTags = csv->all();
    
    for (int x = 0; x < kMaxBlockLeft; x++) {
        vector<BlockSprite*> blocks;
        for (int y = 0; y < kMaxBlockTop; y++)
        {
			stringstream ss;
			int testBlockType;
			ss << testTags[y][x];
			ss >> testBlockType;
            kBlock blockType = (kBlock)testBlockType;
            
            BlockSprite* block = BlockSprite::create(tag, blockType, kStatusNormal);
            block->setPosition(getPosition(x, y));
            background->addChild(block, kZOrderBlock, tag);
            blocks.push_back(block);
            
            tag++;
        }
        blockFields.push_back(blocks);
    }
    blockAreaStartPoint = getPosition(0, 0);
    blockAreaEndPoint   = getPosition(kMaxBlockLeft-1, kMaxBlockTop-1);
}
