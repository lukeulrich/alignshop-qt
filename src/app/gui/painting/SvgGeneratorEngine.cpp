/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtGui/QFont>

#include "SvgGeneratorEngine.h"
#include "../../graphics/AbstractCharPixelMetrics.h"
#include "../../graphics/AbstractTextRenderer.h"
#include "../../graphics/TextColorStyle.h"
#include "../../core/macros.h"
#include "../../core/misc.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
SvgGeneratorEngine::SvgGeneratorEngine(AbstractTextRenderer *abstractTextRenderer, QObject *parent)
    : AbstractRenderEngine(parent),
      abstractTextRenderer_(abstractTextRenderer),
      resolution_(72)
{
    ASSERT(abstractTextRenderer_ != nullptr);
}

/**
  */
SvgGeneratorEngine::~SvgGeneratorEngine()
{
    close();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void SvgGeneratorEngine::close()
{
    if (file_.isOpen())
    {
        writeSvgFooter();
        file_.close();
    }
}

/**
  * @returns QString
  */
QString SvgGeneratorEngine::description() const
{
    return description_;
}

/**
  * @returns QString
  */
QString SvgGeneratorEngine::fileName() const
{
    return file_.fileName();
}

/**
  * @returns bool
  */
bool SvgGeneratorEngine::isOpen() const
{
    return file_.isOpen();
}

/**
  * @param pointSize [const int]
  * @returns qreal
  */
qreal SvgGeneratorEngine::mapPointSize(const int pointSize) const
{
    return static_cast<qreal>(pointSize) * resolution_ / 72.;
}

/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool SvgGeneratorEngine::open(const QString &fileName)
{
    if (file_.isOpen())
        return false;

    file_.setFileName(fileName);
    if (!file_.open(QFile::WriteOnly))
        return false;

    writeSvgHeader();

    return true;
}

/**
  * @param description [const QString &]
  */
void SvgGeneratorEngine::setDescription(const QString &description)
{
    description_ = description;
}

/**
  * @param newResolution [const int]
  */
void SvgGeneratorEngine::setResolution(const int newResolution)
{
    resolution_ = newResolution;
}

/**
  * @param newSize [const QSizeF &]
  */
void SvgGeneratorEngine::setSize(const QSizeF &newSize)
{
    size_ = newSize;
}

/**
  * @param title [const QString &]
  */
void SvgGeneratorEngine::setTitle(const QString &title)
{
    title_ = title;
}

/**
  * @returns QSizeF
  */
QSizeF SvgGeneratorEngine::size() const
{
    return size_;
}

/**
  * @returns QString
  */
QString SvgGeneratorEngine::title() const
{
    return title_;
}

/**
  * @returns AbstractTextRenderer *
  */
AbstractTextRenderer *SvgGeneratorEngine::abstractTextRenderer() const
{
    return abstractTextRenderer_;
}

/**
  * @param pointF [const QPointF &]
  * @param ch [const char]
  * @param textColorStyle [const TextColorStyle &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::drawBlockChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter * /* painter */)
{
    if (!isOpen())
        return;

    writeAll(file_, "<g>");

    // Background rectangle
    if (textColorStyle.background_ != Qt::white)
    {
        QString filledRect = QString("  <rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" style=\"fill:%5\" />\n")
                             .arg(pointF.x())
                             .arg(pointF.y())
                             .arg(abstractTextRenderer_->width())
                             .arg(abstractTextRenderer_->height())
                             .arg(textColorStyle.background_.name());
        writeAll(file_, filledRect.toAscii());
    }

    // Text character
    const AbstractCharPixelMetrics *pixelMetrics = static_cast<const AbstractTextRenderer *>(abstractTextRenderer_)->charPixelMetrics();
    QPointF origin = pointF + pixelMetrics->blockOrigin(ch);
    QString text = QString("  <text class=\"ac\" x=\"%1\" y=\"%2\"")
                   .arg(origin.x())
                   .arg(origin.y());
    if (textColorStyle.foreground_ != Qt::black)
        text += " fill=\"" + textColorStyle.foreground_.name() + "\"";
    text += QString(">%1</text>\n</g>\n").arg(ch);
    writeAll(file_, text.toAscii());
}

/**
  * @param p1 [const QPointF &]
  * @param p2 [const QPointF &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::drawLine(const QPointF &p1, const QPointF &p2, const QColor &color, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}

/**
  * @param rect [const QRect &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::drawRect(const QRect &rect, const QColor &color, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}

/**
  * @param rect [const QRectF &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::drawRect(const QRectF &rect, const QColor &color, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}

/**
  * @param origin [const QPointF &]
  * @param string [const QString &]
  * @param font [const QFont &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::drawText(const QPointF &origin, const QString &string, const QFont &font, const QColor &color, QPainter * /* painter */)
{
    if (!isOpen())
        return;

    QString text = QString("<text x=\"%1\" y=\"%2\" style=\"font-family: %3; font-size: %4;\"")
                   .arg(origin.x())
                   .arg(origin.y())
                   .arg(font.family())
                   .arg(font.pointSize());
    if (color != Qt::black)
        text += " fill=\"" + color.name() + "\"";
    text += QString(">%1</text>\n").arg(string);
    writeAll(file_, text.toAscii());
}

/**
  * @param rect [const QRect &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::fillRect(const QRect &rect, const QBrush &brush, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}

/**
  * @param rect [const QRectF &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::fillRect(const QRectF &rect, const QBrush &brush, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}

/**
  * @param rect [const QRect &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::outlineRectInside(const QRect &rect, const QBrush &brush, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}

/**
  * @param rect [const QRect &]
  * @param side [const Side &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void SvgGeneratorEngine::outlineSideInside(const QRect &rect, const Side &side, const QBrush &brush, QPainter * /* painter */)
{
    if (!isOpen())
        return;
}


// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param pixels [const int]
  * @returns qreal
  */
qreal SvgGeneratorEngine::pixelsToInches(const int pixels) const
{
    return static_cast<qreal>(pixels) / static_cast<qreal>(resolution_);
}

/**
  */
void SvgGeneratorEngine::writeSvgHeader()
{
    ASSERT(isOpen());

    writeAll(file_, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
    writeAll(file_, "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.2\" baseProfile=\"tiny\"\n");
    if (size_.width() > 0 && size_.height() > 0)
    {
        writeAll(file_, QString("  width=\"%1\" height=\"%2\"")
                 .arg(size_.width())
                 .arg(size_.height()).toAscii());
    }
    writeAll(file_, ">\n");
    writeAll(file_,
             "<style type=\"text/css\">\n"
             "<![CDATA[\n"
             "rect {\n"
             "  stroke-width:0;\n"
             "}\n"
             "text.ac {\n"
             "  fill:#000;\n"
             "  stroke:none;\n"
             "  font-family:");
    writeAll(file_, abstractTextRenderer_->font().family().toAscii());
    writeAll(file_, ";\n  font-size:");
    writeAll(file_, QByteArray::number(mapPointSize(abstractTextRenderer_->font().pointSize())));
    writeAll(file_,
             ";\n  font-weight:400;\n"
             "  font-style:normal;\n"
             "}\n"
             "]]>\n"
             "</style>\n"
             "<title>");
    writeAll(file_, title_.toAscii());
    writeAll(file_,
             "</title>\n"
             "<desc>");
    writeAll(file_, description_.toAscii());
    writeAll(file_,
             "</desc>\n"
             "<g>");
}

/**
  */
void SvgGeneratorEngine::writeSvgFooter()
{
    ASSERT(isOpen());

    writeAll(file_,
             "</g>\n"
             "</svg>\n");
}
