#ifndef TEXTURELINE_H
#define TEXTURELINE_H

#include <QList>
#include <QSize>
#include <QPoint>

#include "TextureBlock.h"

class TexturePage;

class TextureLine
{
public:
    TextureLine(TexturePage *texturePage, int yOffset, QSize size);
    ~TextureLine();

    const int height();
    const int width();
    const int x();

    TextureBlock *addTextureBlock(QSize size);

private:
    TexturePage *texturePage_;
    QSize size_;
    QPoint offset_;
    QList<TextureBlock *> textureBlocks_;
};

#endif // TEXTURELINE_H
