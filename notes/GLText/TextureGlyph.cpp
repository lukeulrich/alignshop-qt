#include "TextureGlyph.h"

TextureGlyph::TextureGlyph(TexturePage *texturePage, QPoint offset, QSize size, const uchar *alphaMask) :
        texturePage_(texturePage), textureOffset_(offset), textureSize_(size)
{
    computeUV();
    copyMaskToMemory(alphaMask);

//	printf("TextureGlyph::TextureGlyph(); I am on page: %d\n", texturePage_->textureId());

	/*
	qDebug() << "TextureOffset: " << textureOffset_;
	qDebug() << "TextureSize: " << textureSize_;
	qDebug() << "UV[0]: " << uv_[0];
	qDebug() << "UV[1]: " << uv_[1];
	*/
}

const int
TextureGlyph::textureId()
{
	return texturePage_->textureId();
}

void
TextureGlyph::render(qreal x, qreal y)
{
    static int texId = -1;
//	printf("TextureGlyph::render(); Binding page: %d\n", textureId());
//	qDebug() << "\tLocation: " << textureOffset_ << "  size: " << textureSize_;

    // ASSUME that gl is in the proper state
    // Specifically, that TEXTURE_2D, BLEND, and QUADS have been started
    // and that No of QUADS rendered % 4 == 0
    int actualTextureId = textureId();
    if (actualTextureId != texId)
    {
        glBindTexture(GL_TEXTURE_2D, actualTextureId);
        texId = actualTextureId;
    }

	glBegin(GL_QUADS);
	glTexCoord2f(uv_[0].x(), uv_[0].y());
	glVertex2f(x + bearing_.x(),
			   y + bearing_.y());

	glTexCoord2f(uv_[0].x(), uv_[1].y());
	glVertex2f(x + bearing_.x(),
			   y + bearing_.y() + textureSize_.height());

	glTexCoord2f(uv_[1].x(), uv_[1].y());
	glVertex2f(x + bearing_.x() + textureSize_.width(),
			   y + bearing_.y() + textureSize_.height());

	glTexCoord2f(uv_[1].x(), uv_[0].y());
	glVertex2f(x + bearing_.x() + textureSize_.width(),
			   y + bearing_.y());
	glEnd();
}

void
TextureGlyph::computeUV()
{
    QSize tsize = texturePage_->size();
    qreal shift = 1. / static_cast<qreal>(tsize.width()) * .0078125;

    uv_[0].rx() = static_cast<qreal>(textureOffset_.x()) / static_cast<qreal>(tsize.width()) + shift;
    uv_[0].ry() = static_cast<qreal>(textureOffset_.y()) / static_cast<qreal>(tsize.height()) + shift;

    uv_[1].rx() = (static_cast<qreal>(textureOffset_.x()) + textureSize_.width()) / static_cast<qreal>(tsize.width()) + shift;
    uv_[1].ry() = (static_cast<qreal>(textureOffset_.y()) + textureSize_.height()) / static_cast<qreal>(tsize.height()) + shift;
}

void
TextureGlyph::copyMaskToMemory(const uchar *alphaMask)
{
    Q_CHECK_PTR(alphaMask);

    glBindTexture(GL_TEXTURE_2D, textureId());
    glTexSubImage2D(GL_TEXTURE_2D, 0, textureOffset_.x(), textureOffset_.y(), textureSize_.width(), textureSize_.height(), GL_ALPHA, GL_UNSIGNED_BYTE, alphaMask);
}

