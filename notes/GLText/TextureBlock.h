#ifndef TEXTUREBLOCK_H
#define TEXTUREBLOCK_H

#include <QSize>
#include <QPoint>

class TexturePage;

class TextureBlock
{
public:
    TextureBlock(TexturePage *texturePage, QPoint origin, QSize size);

    const QPoint origin();
    TexturePage *texturePage();

private:
    TexturePage *texturePage_;
    QSize size_;
    QPoint origin_;
};

#endif // TEXTUREBLOCK_H
