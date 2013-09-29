#ifndef __fujidora__BlockSprite__
#define __fujidora__BlockSprite__

#include "cocos2d.h"
#include "Config.h"

using namespace cocos2d;

class BlockSprite : public cocos2d::CCSprite {
public:
    CC_SYNTHESIZE(int, nextPositionX, NexPositionX);
    CC_SYNTHESIZE(int, nextPositionY, NexPositionY);
    CC_SYNTHESIZE(int, tag, Tag);
    CC_SYNTHESIZE(kBlock, type, Type);
    CC_SYNTHESIZE(kStatus, status, Status);
    
    BlockSprite();
    virtual ~BlockSprite();
    virtual bool init(int _tag, kBlock _type, kStatus _status);
    static BlockSprite* create(int _tag, kBlock _type, kStatus _status);
    
protected:
    const char* getBlockImageFileName(kBlock _type);
};

#endif /* defined(__fujidora__BlockSprite__) */
