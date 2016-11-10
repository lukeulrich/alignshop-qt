#include "PrimerDesignWizard.h"
#include "ui_PrimerDesignWizard.h"

#include <PrimerDesign/PrimerDesignFile.h>
#include <PrimerDesign/Range.h>

const char * PrimerDesignWizard::WORKING_FILE = "qtworkingfile.xml";

PrimerDesignWizard::PrimerDesignWizard(QWidget *parent, DnaSequence *sequence, const PrimerDesignInput *params) :
    QWizard(parent),
    ui(new Ui::PrimerDesignWizard)
{
    ui->setupUi(this);
    if (!params)
    {
        input_ = PrimerDesignInputRef(new PrimerDesignInput(sequence->sequence()));
    }
    else
    {
        input_ = PrimerDesignInputRef(new PrimerDesignInput(*params));
        input_->amplicon = sequence->sequence();
    }

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

PrimerDesignWizard::~PrimerDesignWizard()
{
    delete ui;
}

PrimerDesignInput *PrimerDesignWizard::getInput()
{
    return input_.data();
}

PrimerPairFinderResult *PrimerDesignWizard::getFinderResult()
{
    return &finderResult_;
}

void PrimerDesignWizard::setFinderResult(const PrimerPairFinderResult &result)
{
    finderResult_ = result;
}
