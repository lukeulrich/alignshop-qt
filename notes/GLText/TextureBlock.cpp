#include "TextureBlock.h"

TextureBlock::TextureBlock(TexturePage *texturePage, QPoint origin, QSize size) :
        texturePage_(texturePage), size_(size), origin_(origin)
{
}

const QPoint
TextureBlock::origin()
{
    return origin_;
}

TexturePage *
TextureBlock::texturePage()
{
    return texturePage_;
}
