#include "PrimerDesignInput.h"

using namespace PrimerDesign;

PrimerDesignInput::PrimerDesignInput(const QString &amplicon)
{
    this->amplicon = amplicon;
    ampliconBounds = Range(1, amplicon.length());
    ampliconSizeRange = Range(amplicon.length() - 20, amplicon.length());
    primerSizeRange = Range(20, 25);
    tmRange = RangeF(55, 85);
    sodiumConcentration = 1.0;
}

QString PrimerDesignInput::getErrorMessage() const
{
    if (ampliconBounds.min() >= ampliconBounds.max())
    {
        return "The maximum amplicon bound must be greater than the minimum.";
    }

    if (ampliconSizeRange.min() > ampliconSizeRange.max())
    {
        return "The maximum amplicon length may not be less than the minimum.";
    }

    if(ampliconSizeRange.max() > ampliconBounds.length() + 1)
    {
        return "The maximum amplicon length may not be larger than target sequence length.";
    }

    if (primerSizeRange.min() > primerSizeRange.max())
    {
        return "The maximum primer size may not be less than the minimum.";
    }

    if (tmRange.min() >= tmRange.max())
    {
        return "The maximum TM must be greater than the minimum.";
    }

    return "";
}

QString PrimerDesignInput::getBoundedAmplicon() const
{
    return amplicon.mid(ampliconBounds.min(), ampliconBounds.length() + 1);
}

bool PrimerDesignInput::isValid() const
{
    return getErrorMessage().isEmpty();
}

PrimerDesignInput PrimerDesignInput::toZeroBased() const
{
    PrimerDesignInput zeroBased = *this;
    zeroBased.ampliconBounds = Range(ampliconBounds.min() - 1, ampliconBounds.max() - 1);
    zeroBased.ampliconSizeRange = Range(ampliconSizeRange.min() - 1, ampliconSizeRange.max() - 1);
    return zeroBased;
}

PrimerDesignInput PrimerDesignInput::toOneBased() const
{
    PrimerDesignInput oneBased = *this;
    oneBased.ampliconBounds = Range(ampliconBounds.min() + 1, ampliconBounds.max() + 1);
    oneBased.ampliconSizeRange = Range(ampliconSizeRange.min() + 1, ampliconSizeRange.max() + 1);
    return oneBased;
}
