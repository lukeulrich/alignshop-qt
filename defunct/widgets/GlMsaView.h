/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef GLMSAVIEW_H
#define GLMSAVIEW_H

#include "AbstractMsaView.h"

class QGLWidget;

class GlMsaView : public AbstractMsaView
{
public:
    GlMsaView(QWidget *parent = 0) : MsaView(parent)
    {
        glWidget_ = new QGLWidget();
        setViewport(glWidget_);

        intialized_ = false;
    }

    // setupViewport is useless for initializing the GL canvas because it cannot be made be guaranteed to be made
    // current; however, in the paintEvent the opposite is true

protected:

    virtual void drawBackground(QPainter *painter) const;           //!< Draw the background using painter; empty stub defined for inherited classes to change the default behavior
    //! Draw the relevant Msa characters contained in msaRect at origin using painter
    virtual void drawMsa(const QPointF &origin, const QRect &msaRect, QPainter *painter) const;
    virtual void drawSelection(QPainter *painter) const;            //!<
    virtual void drawEditCursor(QPainter *painter) const;
    virtual void drawMouseCursor(QPainter *painter) const;
    virtual void setViewport(QWidget *widget);


    virtual void paintEvent(QPaintEvent *paintEvent)
    {
        if (!glWidget_->isValid())
            return;

        glWidget_->makeCurrent();
        if (!initialized_)
        {
            initializeGL();
            resizeGL(glWidget_->width(), glWidget_->height());
            initialized_ = true;
        }

        // Call parent class master method that calls all other virtual methods in their proper order, we don't need
        // a painter here though - thus the 0 for the drawAll argument :)
        drawAll(0);

        // Update the screen
        if (glWidget_->doubleBuffer())
            glWidget_->swapBuffers();
        else
            glFlush();
    }

    virtual void resizeEvent(QResizeEvent *resizeEvent)            //!< Reimplemented from QAbstractScrollArea::resizeEvent()
    {
        resizeGL(resizeEvent->size().width(), resizeEvent->size().height());
    }


private:
    void initializeGL()
    {
        // gl context is already current
        glWidget_->qglClearColor(QPalette::Base);
    }

    void resizeGL(int w, int h);

    QGLWidget *glWidget_;
    bool initialized_;
};

#endif // GLMSAVIEW_H
