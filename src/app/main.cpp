#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtGui/QFontDatabase>

#include "gui/BasicApplication.h"
#include "gui/forms/MainWindow.h"
#include "core/metatypes.h"

#include "core/Services/LicenseValidator.h"
#include "core/misc.h"
#include "gui/wizards/LicenseWizard.h"

int main(int argc, char *argv[])
{
    QTextStream cerr(stderr);

    try
    {
        BasicApplication application(argc, argv);

        QCoreApplication::setOrganizationName("Agile Genomics, LLC");
        QCoreApplication::setOrganizationDomain("agilegenomics.com");
        QCoreApplication::setApplicationName("AlignShop");

        QSettings settings;
        settings.beginGroup("License");
        QString name = settings.value("name").toString();
        QString licenseKey = settings.value("key").toString();
        settings.endGroup();
        if (!LicenseValidator::isValidLicense(name, licenseKey))
        {
            LicenseWizard licenseWizard;
            licenseWizard.setLicenseText(::readBetaLicense());
            if (!licenseWizard.exec())
                return 0;

            // License is valid, persist to the settings
            settings.beginGroup("License");
            settings.setValue("name", licenseWizard.field("name").toString());
            settings.setValue("key", licenseWizard.field("licenseKey").toString());
            settings.endGroup();
        }

        QFontDatabase::addApplicationFont("aliases/fonts/VeraMono");
        QFontDatabase::addApplicationFont("aliases/fonts/Cousine-Regular-Latin");
        QFontDatabase::addApplicationFont("aliases/fonts/Inconsolata");
        QFontDatabase::addApplicationFont("aliases/fonts/Anonymous Pro");
        QFontDatabase::addApplicationFont("aliases/fonts/Cabin-Regular");

        QFontDatabase::addApplicationFont(":/resources/fonts/DejaVuSans.ttf");
        QFontDatabase::addApplicationFont("aliases/fonts/DejaVuSansMono");
        QFontDatabase::addApplicationFont("aliases/fonts/DejaVuSansCondensed");

        MainWindow w;
        w.show();

        return application.exec();
    }
    catch (QString &error)
    {
        cerr << "Unhandled exception: " << error << "\n";
        return 1;
    }
    catch (...)
    {
        cerr << "Unhandled exception: unknown\n";
        return 1;
    }
}
