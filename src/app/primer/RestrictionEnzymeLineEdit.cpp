#include <QtCore/QMetaType>

#include "RestrictionEnzymeLineEdit.h"
#include "DnaSequenceValidator.h"

RestrictionEnzymeLineEdit::RestrictionEnzymeLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    // ISSUE? Should this be done elsewhere in a consolidated fashion? (e.g. RestrictionEnzyme.cpp)?
    qRegisterMetaType<RestrictionEnzyme>("RestrictionEnzyme");

    connect(this, SIGNAL(textEdited(QString)), SLOT(onTextChanged()));

    setValidator(new DnaSequenceValidator(this));
}

void RestrictionEnzymeLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    QPalette foreColor = palette();
    foreColor.setColor(QPalette::Text, Qt::black);
    setPalette(foreColor);
    QFont f = font();
    f.setItalic(false);
    setFont(f);
    if (!enzyme_.name().isEmpty())
    {
        setText(enzyme_.recognitionSite().asByteArray());
        setToolTip(enzyme_.name());
    }
    else
    {
        setText(manualSite_);
        setToolTip(QString("Custom 5' addition"));
    }
}

void RestrictionEnzymeLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    if (!enzyme_.name().isEmpty())
        showName();
}

void RestrictionEnzymeLineEdit::onTextChanged()
{
    if (enzyme_.isEmpty())
    {
        manualSite_ = text();
    }
    else if (text().toAscii() != enzyme_.recognitionSite().asByteArray())
    {
        enzyme_ = RestrictionEnzyme();
        manualSite_ = text();
    }
}

RestrictionEnzyme RestrictionEnzymeLineEdit::restrictionEnzyme() const
{
    if (!enzyme_.name().isEmpty())
        return enzyme_;
    else
        return RestrictionEnzyme(QString(), BioString(manualSite_.toAscii(), eDnaGrammar), QVector<int>(), QVector<int>());
}

void RestrictionEnzymeLineEdit::setRestrictionEnzyme(const RestrictionEnzyme &enzyme)
{
    manualSite_.clear();
    enzyme_ = enzyme;
    if (enzyme_.isEmpty())
    {
        setText("");
        return;
    }

    if (hasFocus())
        setText(enzyme_.recognitionSite().asByteArray());
    else
        showName();
}

void RestrictionEnzymeLineEdit::showName()
{
    QPalette foreColor = palette();
    foreColor.setColor(QPalette::Text, QColor("#333"));
    setPalette(foreColor);
    QFont f = font();
    f.setItalic(true);
    setFont(f);
    setText(enzyme_.name() + " - " + enzyme_.recognitionSite().asByteArray());
    setToolTip(enzyme_.recognitionSite().asByteArray());
}
