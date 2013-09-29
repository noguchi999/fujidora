#ifndef fujidora_Config_h
#define fujidora_Config_h

enum kBlock {
    kBlockRed,
    kBlockBlue,
    kBlockYellow,
    kBlockGreen,
    kBlockGray,
    kBlockCount
};

enum kStatus {
    kStatusNormal,
    kStatusMarked,
    kStatusSwept
};

#define ccsf(...) CCString::createWithFormat(__VA_ARGS__)->getCString()

#endif
