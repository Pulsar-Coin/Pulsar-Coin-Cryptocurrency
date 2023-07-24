// Copyright (c) 2011-2014 The Bitcoin Core developers
// Maintained and Managed by The Pulsar Coin Team
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PULSAR_QT_PULSARADDRESSVALIDATOR_H
#define PULSAR_QT_PULSARADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class PulsarAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit PulsarAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Pulsar address widget validator, checks for a valid Pulsar address.
 */
class PulsarAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit PulsarAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // PULSAR_QT_PULSARADDRESSVALIDATOR_H
