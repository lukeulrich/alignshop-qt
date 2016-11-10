#ifndef TEXTUREPAGE_H
#define TEXTUREPAGE_H

#include <QList>
#include <QSize>
#include <QtOpenGL>

#include "TextureLine.h"

class TexturePage
{
public:
    TexturePage(QSize size);
    ~TexturePage();

    const int textureId();
    TextureLine *addTextureLine(int height);
    TextureLine *getBestLine(const QSize &size);
    const QSize &size();
    const int ySpace();

private:
    void createTexture();
    void deleteTexture();

    int textureId_;
    QSize size_;
    int yOffset_;
    QList<TextureLine *> textureLines_;
};

#endif // TEXTUREPAGE_H
