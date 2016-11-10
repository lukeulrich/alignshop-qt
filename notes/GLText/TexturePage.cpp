#include "TexturePage.h"

TexturePage::TexturePage(QSize size) :
        textureId_(0), size_(size), yOffset_(0)
{
    createTexture();
}

TexturePage::~TexturePage()
{
    TextureLine *tl;
    foreach (tl, textureLines_)
        delete tl;

    textureLines_.clear();

    deleteTexture();
}

const int
TexturePage::textureId()
{
    return textureId_;
}

TextureLine *
TexturePage::addTextureLine(int height)
{
    Q_ASSERT(ySpace() >= height);

    TextureLine *newLine = new TextureLine(this, yOffset_, QSize(size_.width(), height));
    Q_CHECK_PTR(newLine);
    textureLines_.append(newLine);

    yOffset_ += height;

    return newLine;
}

TextureLine *
TexturePage::getBestLine(const QSize &size)
{
    qreal bestRatio = 0.f;
    TextureLine *bestLine = NULL;
    TextureLine *tl;
    foreach (tl, textureLines_)
    {
        if (tl->height() < size.height())
            continue;

        if (tl->width() - tl->x() < size.width())
            continue;

        qreal r = static_cast<qreal>(size.height()) / static_cast<qreal>(tl->height());

        if (r > bestRatio)
        {
            bestRatio = r;
            bestLine = tl;
        }
    }

    return bestLine;
}

const QSize &
TexturePage::size()
{
    return size_;
}

const int
TexturePage::ySpace()
{
    return size_.height() - yOffset_;
}

void
TexturePage::createTexture()
{
    // Make 8-bit alpha mask
    int textureSize = size_.width() * size_.height();
    unsigned char *tbuf = new unsigned char[textureSize];
    memset(tbuf, 0, textureSize);

	// Unnecessary because we call this during the OpenGL initialization
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, reinterpret_cast<GLuint *>(&textureId_));
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, size_.width(), size_.height(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, tbuf);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    delete[] tbuf;
    tbuf = NULL;
}

void
TexturePage::deleteTexture()
{
    Q_ASSERT(textureId_);

    glDeleteTextures(1, reinterpret_cast<GLuint *>(&textureId_));
    textureId_ = 0;
}
