#ifndef DNASEQUENCEVALIDATOR_H
#define DNASEQUENCEVALIDATOR_H

#include <QtGui/QValidator>

class DnaSequenceValidator : public QValidator
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit DnaSequenceValidator(QObject *parent = 0);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    State validate(QString &value, int &position) const;
    void fixup(QString &value) const;
};

#endif // DNASEQUENCEVALIDATOR_H
