#ifndef SEQUENCEVALIDATOR_H
#define SEQUENCEVALIDATOR_H

#include <QValidator>

class SequenceValidator : public QValidator
{
    Q_OBJECT
public:
    explicit SequenceValidator(QObject *parent = 0);

    virtual State validate(QString &value, int &position) const;
    virtual void fixup(QString &value) const;
};

#endif // SEQUENCEVALIDATOR_H
