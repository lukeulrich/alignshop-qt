#ifndef GLFONT_H
#define GLFONT_H

#include <QChar>
#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QString>
#include <QtOpenGL>

#include "TextureGlyph.h"
#include "GlyphPacker.h"

class GlFont
{
public:
    GlFont(QString fontFamily = "monospace", int pointSize = 12, qreal scale = 1.f);
    ~GlFont();

    void clearGlyphs();
    const qreal height();
    const qreal width(QChar character);
    const qreal width(QString string);
    void renderText(QString string);
	void setFamily(QString fontFamily);
	void setPointSize(int pointSize);
    void setScale(qreal scale);

protected:
    TextureGlyph *cachedGlyph(QChar character);
    void computeFontMetrics();

private:
    QRect imageBoundingRect(QImage &image);
    
    int pointSize_;
    QString fontFamily_;
    QFont font_;
    QFontMetrics fontMetrics_;
    qreal lineSpacing_;
    qreal spaceWidth_;
    qreal height_;
    qreal leading_;
    qreal ascent_;
    qreal descent_;

    qreal scale_;
    GlyphPacker glyphPacker_;
    QHash<QChar, TextureGlyph *> glyphMap_;
};

#endif // GLFONT_H
