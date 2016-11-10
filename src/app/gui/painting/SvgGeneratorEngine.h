/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SVGGENERATORENGINE_H
#define SVGGENERATORENGINE_H

#include <QtCore/QSizeF>

#include "AbstractRenderEngine.h"

class QFile;
class QFont;

/**
  * SvgGeneratorEngine provides for generating SVG output that adheres to the IRenderEngine interface.
  *
  * Before any operation is successful, it must be opened with a valid filename. This will open the file for writing
  * and output the SVG header information. All subsequent painting calls will write the corresponding SVG output
  * to fileName until close is called.
  *
  * If the generator is not open, the methods do nothing.
  *
  * This class is not as generic as it name suggests. Rather it contains alignment-specific instructions.
  */
class SvgGeneratorEngine : public AbstractRenderEngine
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    SvgGeneratorEngine(AbstractTextRenderer *abstractTextRenderer, QObject *parent = nullptr);
    ~SvgGeneratorEngine();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void close();                                                   //!< Close any open output file
    QString description() const;                                    //!< Returns the description
    QString fileName() const;                                       //!< Returns the filename
    bool isOpen() const;                                            //!< Returns true if a file is open for writing; false otherwise
    qreal mapPointSize(const int pointSize) const;                  //!< Returns pointSize multiplied by resolution() divided by 72; useful for mapping a device font size to its equivalent font size in SVG space
    bool open(const QString &fileName);                             //!< Opens fileName for writing, outputs the SVG header information and returns true on success; false otherwise
    int resolution() const;                                         //!< Returns the current document resolution
    void setDescription(const QString &description);                //!< Sets the description
    void setResolution(const int newResolution);                    //!< Sets the resolution to newResolution
    void setSize(const QSizeF &newSize);                            //!< Sets the size in pixels to newSize
    void setTitle(const QString &title);                            //!< Sets the title
    QSizeF size() const;                                            //!< Returns the current size
    QString title() const;                                          //!< Returns the title


    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    virtual AbstractTextRenderer *abstractTextRenderer() const;
    virtual void drawBlockChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter);
    virtual void drawLine(const QPointF &p1, const QPointF &p2, const QColor &color, QPainter *painter);
    // Draws outlined rectangle within and up to the rect boundaries. Not outside the boundaries
    virtual void drawRect(const QRect &rect, const QColor &color, QPainter *painter);
    virtual void drawRect(const QRectF &rect, const QColor &color, QPainter *painter);
    // Draws text using the specified font
    virtual void drawText(const QPointF &origin, const QString &string, const QFont &font, const QColor &color, QPainter *painter);
    // Fills the rect without any outline
    virtual void fillRect(const QRect &rect, const QBrush &brush, QPainter *painter);
    virtual void fillRect(const QRectF &rect, const QBrush &brush, QPainter *painter);
    virtual void outlineRectInside(const QRect &rect, const QBrush &brush, QPainter *painter);
    virtual void outlineSideInside(const QRect &rect, const Side &side, const QBrush &brush, QPainter *painter);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Converts pixels to inches
    qreal pixelsToInches(const int pixels) const;
    void writeSvgHeader();                  //!< Writes the SVG header
    void writeSvgFooter();                  //!< Writes the SVG footer


    // ------------------------------------------------------------------------------------------------
    // Private members
    AbstractTextRenderer *abstractTextRenderer_;
    QFile file_;
    int resolution_;
    QSizeF size_;                // In user units
    QString title_;
    QString description_;
};

#endif // SVGGENERATORENGINE_H
