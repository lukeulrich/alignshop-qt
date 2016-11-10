#include "GlFont.h"

// --------------------------------------------------------
GlFont::GlFont(QString fontFamily, int pointSize, qreal scale) :
        pointSize_(pointSize), fontFamily_(fontFamily), font_(fontFamily_, pointSize_),
        fontMetrics_(font_)
{
    setScale(scale);
}

// --------------------------------------------------------
GlFont::~GlFont()
{
    clearGlyphs();
}

// --------------------------------------------------------
void
GlFont::clearGlyphs()
{
    glyphPacker_.clear();

    foreach (TextureGlyph *g, glyphMap_)
        delete g;

    glyphMap_.clear();
}

// --------------------------------------------------------
const qreal
GlFont::height()
{
    return height_;
}

// --------------------------------------------------------
const qreal
GlFont::width(QChar character)
{
    return fontMetrics_.width(character) * scale_;
}

// --------------------------------------------------------
const qreal
GlFont::width(QString string)
{
    return fontMetrics_.width(string) * scale_;
}

// --------------------------------------------------------
void
GlFont::renderText(QString string)
{
    QPointF position(0., 0.);
    int nChars = string.length();
    if (!nChars)
        return;


	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	for (int i=0; i< nChars; ++i)
	{
		if (string[i] == '\n')
		{
//			glTranslatef(-position.x(), height_, 0.);

            position.rx() = 0;
            position.ry() += height_;
            continue;
        }

        if (string[i] == ' ')
        {
//			glTranslatef(spaceWidth_, 0, 0);
            position.rx() += spaceWidth_;
            continue;
        }

//		qDebug() << "Drawing character: " << string[i];

		TextureGlyph *g = cachedGlyph(string[i]);

		g->render(position.x(), position.y());


//		glTranslatef(g->width_, 0., 0.);

		position.rx() += g->width_;
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}

// --------------------------------------------------------
void
GlFont::setFamily(QString fontFamily)
{
	if (fontFamily == fontFamily_)
		return;

	fontFamily_ = fontFamily;

	clearGlyphs();

	font_ = QFont(fontFamily_, pointSize_);
	fontMetrics_ = QFontMetrics(font_);
	computeFontMetrics();
}

// --------------------------------------------------------
void
GlFont::setPointSize(int pointSize)
{
    if (pointSize == pointSize_)
        return;

    pointSize_ = pointSize;

    clearGlyphs();

    font_ = QFont(fontFamily_, pointSize_);
    fontMetrics_ = QFontMetrics(font_);
    computeFontMetrics();
}

// --------------------------------------------------------
void
GlFont::setScale(qreal scale)
{
	if (scale_ == scale || scale < .001)
		return;

//	qDebug() << ">>> New scale: " << scale;
//	printf("%f\n", scale);

    scale_ = scale;

    clearGlyphs();
    computeFontMetrics();
}

// --------------------------------------------------------
TextureGlyph *
GlFont::cachedGlyph(QChar character)
{
    if (glyphMap_.contains(character))
        return glyphMap_[character];

    // Draw the scaled character on an image surface
    QImage image(QSize(static_cast<int>(width(character)), static_cast<int>(height_)), QImage::Format_RGB32);
    image.fill(Qt::black);
    QPainter painter(&image);
    painter.setFont(font_);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::white);
    painter.scale(scale_, scale_);
    // Drawing with unscaled parameters because it will scale automatically
    painter.drawText(0, fontMetrics_.ascent() + 1, character);
    painter.end();

    QRect bounds = imageBoundingRect(image);

    // Create unsigned char array of alpha mask using only the region indicated by bounds
    uchar *buffer = new uchar[ bounds.width() * bounds.height() ];
    int x1 = bounds.x();
    int y1 = bounds.y();
    int x2 = x1 + bounds.width();
    int y2 = y1 + bounds.height();
    int stride = bounds.width();

//	printf("(%d, %d) -> (%d, %d)\n", x1, y1, x2, y2);

    for (int y=y1, a=0; y< y2; ++y, ++a)
    {
        for (int x=x1, b=0; x< x2; ++x, ++b)
        {
//			*buffer++ = qRed(image.pixel(x, y));
            buffer[ a * stride + b ] = qRed(image.pixel(x, y));

//            printf("%3d ", buffer[a * stride + b]);
        }
//        printf("\n");
    }

//	qDebug() << "New sz: " << bounds.size();

    // Create the glyph and pack into a texture block
    TextureBlock *block = glyphPacker_.getBlock(bounds.size());
    TextureGlyph *g = new TextureGlyph(block->texturePage(), block->origin(), bounds.size(), buffer);

    g->width_ = width(character);
    g->bearing_.rx() = (g->width_ - static_cast<qreal>(bounds.width())) / 2.;
    g->bearing_.ry() = y1;

    delete[] buffer;
    buffer = NULL;

    glyphMap_.insert(character, g);

    return g;
}

// --------------------------------------------------------
void
GlFont::computeFontMetrics()
{
    spaceWidth_ = fontMetrics_.width(' ') * scale_;
    lineSpacing_ = fontMetrics_.lineSpacing() * scale_;
    height_ = fontMetrics_.height() * scale_;
    leading_ = fontMetrics_.leading() * scale_;
    ascent_ = fontMetrics_.ascent() * scale_;
    descent_ = fontMetrics_.descent() * scale_;
}

// --------------------------------------------------------
QRect
GlFont::imageBoundingRect(QImage &image)
{
    Q_ASSERT(image.width() && image.height());

    int x_min = 0;
    int x_max = image.width()-1;
    int y_min = 0;
    int y_max = image.height()-1;

    int h = image.height();
    int w = image.width();

    for (; x_min < w; ++x_min)
        for (int y=0; y< y_max; ++y)
            if (qRed(image.pixel(x_min, y)))
                goto DONE_LEFT;
 DONE_LEFT:

    for (; x_max > x_min; --x_max)
        for (int y=0; y< y_max; ++y)
            if (qRed(image.pixel(x_max, y)))
                goto DONE_RIGHT;
 DONE_RIGHT:

    for (; y_min < h; ++y_min)
        for (int x=0; x< w; ++x)
            if (qRed(image.pixel(x, y_min)))
                goto DONE_TOP;
 DONE_TOP:

    for (; y_max > y_min; --y_max)
        for (int x=0; x< w; ++x)
            if (qRed(image.pixel(x, y_max)))
                goto DONE_BOTTOM;
 DONE_BOTTOM:

    return QRect(x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);
}
