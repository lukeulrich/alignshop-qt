#include <QRegExpValidator>
#include <QRegExp>
#include <QDebug>

#include "PrimerFinderDialog.h"
#include "ui_PrimerFinderDialog.h"
#include "global.h"


PrimerFinderDialog::PrimerFinderDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    //
    restrictionEnzymeDialog_ = 0;
    QObject::connect(addForwardRestrictionSiteButton, SIGNAL(clicked()), this, SLOT(onSelectRestrictionSite_clicked()));
    //
    QRegExp validBases_(QString("[%1]").arg(constants::kDnaCharacters));
    forward5Addition->setValidator(new QRegExpValidator(validBases_, this));
    reverse5Addition->setValidator(new QRegExpValidator(validBases_, this));
}

/**
  * If the user desires to add a restriction site to the terminus of a primer to model, open
  * a RestrictionEnzymeDialog dialog to select the appropriate enzyme recognition site
  */
void PrimerFinderDialog::onSelectRestrictionSite_clicked()
{
    if(!restrictionEnzymeDialog_){restrictionEnzymeDialog_ = new RestrictionEnzymeDialog(this);}
    restrictionEnzymeDialog_->exec();
    forward5Addition->setText(restrictionEnzymeDialog_->selectedRestrictionSite());
}
