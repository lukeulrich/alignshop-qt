#include "GlyphPacker.h"

GlyphPacker::GlyphPacker()
{
	texturePageSize_ = QSize(128, 128);
    lineHeightThreshold_ = .7f;
    extendHeightFactor_ = 1.1f;
}

GlyphPacker::~GlyphPacker()
{
    clear();
}

TextureBlock *
GlyphPacker::getBlock(const QSize &size)
{
    qreal bestRatio = 0.f;
    TextureLine *bestLine = NULL;

    TexturePage *page;
    foreach (page, texturePages_)
    {
        TextureLine *line = page->getBestLine(size);
        if (!line)
            continue;

        qreal r = static_cast<qreal>(size.height()) / static_cast<qreal>(line->height());
        if (r > bestRatio)
        {
            bestRatio = r;
            bestLine = line;
        }
    }
    
    if (bestLine &&
        bestRatio >= lineHeightThreshold_)
        return bestLine->addTextureBlock(size);

    int desiredHeight = static_cast<int>(size.height() * extendHeightFactor_) + 1;
    foreach (page, texturePages_)
    {
        if (page->ySpace() < desiredHeight)
            continue;

        TextureLine *line = page->addTextureLine(desiredHeight);
        return line->addTextureBlock(size);
    }

    Q_ASSERT(size.height() <= texturePageSize_.height() &&
             size.width() <= texturePageSize_.width());

    page = new TexturePage(texturePageSize_);

//	printf("GlyphPacker::getBlock(...); Page: %d\n", page->textureId());

    Q_CHECK_PTR(page);
    texturePages_.append(page);
    TextureLine *line = page->addTextureLine(desiredHeight);
    return line->addTextureBlock(size);
}

void
GlyphPacker::clear()
{
    TexturePage *page;
    foreach (page, texturePages_)
        delete page;

    texturePages_.clear();
}
