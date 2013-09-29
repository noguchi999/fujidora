#include "BlockSprite.h"

using namespace cocos2d;

BlockSprite::BlockSprite()
:nextPositionX(-1)
,nextPositionY(-1)
{
}

BlockSprite::~BlockSprite() {
}

BlockSprite* BlockSprite::create(int _tag, kBlock _type, kStatus _status)
{
    BlockSprite *pRet = new BlockSprite();
    
    if (pRet && pRet->init(_tag, _type, _status))
    {
        return pRet;
        
    } else {
        CC_SAFE_DELETE(pRet);
        return NULL;
    }
}

bool BlockSprite::init(int _tag, kBlock _type, kStatus _status)
{
    if (!CCSprite::initWithFile(getBlockImageFileName(_type)))
    {
        return false;
    }
    
    tag    = _tag;
    type   = _type;
    status = _status;
    
    return true;
}

const char* BlockSprite::getBlockImageFileName(kBlock _type)
{
    switch (_type) {
        case kBlockRed:
            return "004_akikan.png";
            
        case kBlockBlue:
            return "blue.png";
            
        case kBlockYellow:
            return "003_tamago.png";
            
        case kBlockGreen:
            return "001_baikin.png";
            
        case kBlockGray:
            return "002_kuma.png";
            
        default:
            CCAssert(false, "invalid blockType");
            return "";
    }
}