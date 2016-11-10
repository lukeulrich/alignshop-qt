/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "GlMsaView.h"

GlMsaView::GlMsaView()
{
}


void
MsaView::resizeGL(int width, int height)
{
    printf("ResieGL: %d, %d\n", width, height);
    fflush(stdout);

    if (!msa_)
        return;

    curCols_ = qMin(static_cast<int>(static_cast<qreal>(width) / static_cast<qreal>(w_) + 1.), msa_->length());
    curRows_ = qMin(static_cast<int>(static_cast<qreal>(height) / static_cast<qreal>(h_) + 1.), msa_->subseqCount());

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);

	// In windows, when the window is maximized, the original visible porition
	// is no longer updated - only the new previously invisible area is updated.
	// Seems to be do to an issue with the OpenGL drivers/hardware (e.g. Intel GMA950)
	// The code below sets the proper window region and enables updates to the entire
	// area when maximized.
	// Source: http://labs.trolltech.com/forums/topic/658
#ifdef Q_WS_WIN
	HWND hwnd = WindowFromDC(wglGetCurrentDC());
	HRGN hrgn = CreateRectRgn(0, 0, width-1, height-1);
	SetWindowRgn(hwnd, hrgn, false);
#endif
}

void
MsaView::initializeGL()
{
    printf("Initializing GL\n");
    fflush(stdout);

    glClearColor(1., 1., 1., 1.);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);       /* Enable Texture Mapping */

    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_LOGIC_OP);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);

    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    glPixelTransferi(GL_RED_SCALE, 1);
    glPixelTransferi(GL_RED_BIAS, 0);
    glPixelTransferi(GL_GREEN_SCALE, 1);
    glPixelTransferi(GL_GREEN_BIAS, 0);
    glPixelTransferi(GL_BLUE_SCALE, 1);
    glPixelTransferi(GL_BLUE_BIAS, 0);
    glPixelTransferi(GL_ALPHA_SCALE, 1);
    glPixelTransferi(GL_ALPHA_BIAS, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const GLubyte* extString = glGetString(GL_EXTENSIONS);

    if (extString != NULL)
    {
        if (strstr((char*) extString, "GL_EXT_convolution") != NULL)
        {
            glDisable(GL_CONVOLUTION_1D_EXT);
            glDisable(GL_CONVOLUTION_2D_EXT);
            glDisable(GL_SEPARABLE_2D_EXT);
        }
        if (strstr((char*) extString, "GL_EXT_histogram") != NULL)
        {
            glDisable(GL_HISTOGRAM_EXT);
            glDisable(GL_MINMAX_EXT);
        }
        if (strstr((char*) extString, "GL_EXT_texture3D") != NULL)
            glDisable(GL_TEXTURE_3D_EXT);
    }

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glShadeModel(GL_FLAT);

    w_ = font_.width('M');
    h_ = font_.height();

    createDL();
}

void
MsaView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    for (int i=0; i< curRows_; ++i)
    {
        glPushMatrix();
        glTranslatef(0, i * h_, 0.);
        QByteArray characters = msa_->at(i+1)->bioString().sequence().toAscii();
        glCallList(dl_offset_ + i);

		glColor3ubv(gl_black);
		font_.renderText(msa_->at(i+1)->bioString().sequence().mid(0, curCols_));
		glPopMatrix();
	}

    /*
    // Render new rows at cols
    for (int i=0; i<curRows_; ++i)
    {
        glPushMatrix();
        glTranslatef(0, i * h_, 0.);
        QByteArray characters = msa_->at(i+1)->bioString().sequence().toAscii();
        for (int j=0; j<curCols_; ++j)
        {
            QChar ch = characters.at(j);
            if (BioString::isGap(ch))
                continue;

			TextColorStyle colors = positionalColorProvider_->color(*msa_, i, j);
			uchar r = colors.background_.red();
			uchar g = colors.background_.green();
			uchar b = colors.background_.blue();

			glPushMatrix();
			glTranslatef(j * w_, 0., 0.);
			glColor3ub(r, g, b);
//			glColor3ubv(gl_black);
			glBegin(GL_QUADS);
			glVertex2f(0., 0.);
			glVertex2f(0., h_);
			glVertex2f(w_, h_);
			glVertex2f(w_, 0.);
			glEnd();
			glPopMatrix();
		}

		glColor3ubv(gl_black);
		font_.renderText(msa_->at(i+1)->bioString().sequence().mid(0, curCols_));
		glPopMatrix();
	}
	*/
}

void MsaView::createDL()
{
	if (!msa_)
		return;

	dl_offset_ = glGenLists(msa_->subseqCount());

	for (int y=0; y< msa_->subseqCount(); ++y)
	{
		QByteArray characters = msa_->at(y+1)->bioString().sequence().toAscii();

		glNewList(dl_offset_ + y, GL_COMPILE);
		glBegin(GL_QUADS);
		for (int j=0; j< msa_->length(); ++j)
		{
			QChar ch = characters.at(j);
			if (BioString::isGap(ch))
				continue;

			TextColorStyle colors = positionalColorProvider_->color(*msa_, y, j);
			uchar r = colors.background_.red();
			uchar g = colors.background_.green();
			uchar b = colors.background_.blue();

			glColor3ub(r, g, b);
			glVertex2f(j * w_, 0);
			glVertex2f(j * w_, h_);
			glVertex2f(j * w_ + w_, h_);
			glVertex2f(j * w_ + w_, 0);
		}
		glEnd();
		glEndList();
	}
}
