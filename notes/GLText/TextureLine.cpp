#include "TextureLine.h"

TextureLine::TextureLine(TexturePage *texturePage, int yOffset, QSize size) :
        texturePage_(texturePage), size_(size)
{
    Q_CHECK_PTR(texturePage);

    offset_.ry() = yOffset;
}

TextureLine::~TextureLine()
{
    TextureBlock *tb;
    foreach (tb, textureBlocks_)
        delete tb;

    textureBlocks_.clear();
}

const int
TextureLine::height()
{
    return size_.height();
}

const int
TextureLine::width()
{
    return size_.width();
}

const int
TextureLine::x()
{
    return offset_.x();
}

TextureBlock *
TextureLine::addTextureBlock(QSize size)
{
    Q_ASSERT(size_.width() - offset_.x() >= size.width());

    TextureBlock *newBlock = new TextureBlock(texturePage_, offset_, size);
    Q_CHECK_PTR(newBlock);
    textureBlocks_.append(newBlock);

    offset_.rx() += size.width();

    return newBlock;
}
