#include "PrimerDesignBasePage.h"

using namespace PrimerDesign;

PrimerDesignBasePage::PrimerDesignBasePage(PrimerDesignWizard *parent)
    : QWizardPage(parent)
{
}

PrimerDesignWizard *PrimerDesignBasePage::baseWizard()
{
    return ((PrimerDesignWizard*)wizard());
}
