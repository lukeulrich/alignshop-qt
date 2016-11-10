#ifndef TEXTUREGLYPH_H
#define TEXTUREGLYPH_H

#include <QPointF>
#include <QSize>
#include <QtOpenGL>

#include "TexturePage.h"

class TextureGlyph
{
public:
    TextureGlyph(TexturePage *texturePage, QPoint offset, QSize size, const uchar *alphaMask);

    const int textureId();
	void render(qreal x = 0., qreal y = 0.);

    qreal width_;
    QPointF bearing_;

private:
    void computeUV();
    void copyMaskToMemory(const uchar* alphaMask);

    TexturePage *texturePage_;
    QPoint textureOffset_;
    QSize textureSize_;
    QPointF uv_[2];
};

#endif // TEXTUREGLYPH_H
