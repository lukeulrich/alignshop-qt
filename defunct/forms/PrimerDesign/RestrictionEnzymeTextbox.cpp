#include "RestrictionEnzymeTextbox.h"

using namespace PrimerDesign;

RestrictionEnzymeTextbox::RestrictionEnzymeTextbox(QWidget *parent)
    : QLineEdit(parent)
{
    QObject::connect(this, SIGNAL(textEdited(QString)), this, SLOT(onTextChanged()));
}

void RestrictionEnzymeTextbox::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    QPalette foreColor = palette();
    foreColor.setColor(QPalette::Text, Qt::black);
    setPalette(foreColor);
    QFont f = font();
    f.setItalic(false);
    setFont(f);
    setText(enzyme_.value);
    setToolTip(enzyme_.name);
}

void RestrictionEnzymeTextbox::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    enzyme_.value = text();
    if (!enzyme_.name.isEmpty())
    {
        showName();
    }
}

void RestrictionEnzymeTextbox::onTextChanged()
{
    enzyme_.name = "";
}

RestrictionEnzyme RestrictionEnzymeTextbox::restrictionEnzyme() const
{
    return enzyme_;
}

void RestrictionEnzymeTextbox::setRestrictionEnzyme(const RestrictionEnzyme &enzyme)
{
    enzyme_ = enzyme;
    if (hasFocus())
    {
        setText(enzyme_.value);
    }
    else
    {
        showName();
    }
}

void RestrictionEnzymeTextbox::showName()
{
    QPalette foreColor = palette();
    foreColor.setColor(QPalette::Text, QColor("#333"));
    setPalette(foreColor);
    QFont f = font();
    f.setItalic(true);
    setFont(f);
    setText(enzyme_.name + " - " + enzyme_.value);
    setToolTip(enzyme_.value);
}
