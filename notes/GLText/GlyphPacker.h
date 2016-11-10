#ifndef GLYPHPACKER_H
#define GLYPHPACKER_H

#include <QList>
#include <QSize>

#include "TextureBlock.h"
#include "TextureLine.h"
#include "TexturePage.h"

class GlyphPacker
{
public:
    GlyphPacker();
    ~GlyphPacker();

    TextureBlock *getBlock(const QSize &size);
    void clear();

private:
    QSize texturePageSize_;
    qreal lineHeightThreshold_;
    qreal extendHeightFactor_;
    QList<TexturePage *> texturePages_;
};

#endif // GLYPHPACKER_H
