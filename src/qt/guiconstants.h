// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_GUICONSTANTS_H
#define BITCOIN_QT_GUICONSTANTS_H

/* Milliseconds between model updates */
static const int MODEL_UPDATE_DELAY = 250;

/* AskPassphraseDialog -- Maximum passphrase length */
static const int MAX_PASSPHRASE_SIZE = 1024;

/* BitcoinGUI -- Size of icons in status bar */
static const int STATUSBAR_ICONSIZE = 14;

static const bool DEFAULT_SPLASHSCREEN = true;

/* Invalid field background style */
#define STYLE_INVALID "background:#FF8080"

/* Transaction list -- unconfirmed transaction */
#define COLOR_UNCONFIRMED QColor(140, 140, 140)
/* Transaction list -- negative amount */
#define COLOR_NEGATIVE QColor(63, 58, 255)
/* Table List -- negative amount */
#define COLOR_NEGATIVE_TABLE QColor(63, 58, 255)
/* Transaction list -- positive amount */
#define COLOR_POSITIVE QColor(0, 152, 206)
/* Transaction list -- bare address (without label) */
#define COLOR_BAREADDRESS QColor(121, 121, 121)
/* Transaction list -- TX status decoration - open until date */
#define COLOR_TX_STATUS_OPENUNTILDATE QColor(64, 64, 255)
/* Transaction list -- TX status decoration - danger, tx needs attention */
#define COLOR_TX_STATUS_DANGER QColor(63, 58, 255)
/* Transaction list -- TX status decoration - default color */
#define COLOR_BLACK QColor(0, 0, 0)

/* Tooltips longer than this (in characters) are converted into rich text,
   so that they can be word-wrapped.
 */
static const int TOOLTIP_WRAP_THRESHOLD = 80;

/* Maximum allowed URI length */
static const int MAX_URI_LENGTH = 255;

/* QRCodeDialog -- size of exported QR Code image */
#define QR_IMAGE_SIZE 300

/* Number of frames in spinner animation */
#define SPINNER_FRAMES 36

#define QAPP_ORG_NAME "Pulsar"
#define QAPP_ORG_DOMAIN "pulsar.net"
#define QAPP_APP_NAME_DEFAULT "Pulsar-Qt"
#define QAPP_APP_NAME_TESTNET "Pulsar-Qt-testnet"

/* Colors for staking tab for each coin age group */
#define COLOR_MINT_YOUNG QColor(255, 224, 226)
#define COLOR_MINT_MATURE QColor(204, 255, 207)
#define COLOR_MINT_OLD QColor(111, 252, 141)

#endif // BITCOIN_QT_GUICONSTANTS_H
