// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Maintained and Managed by The Pulsar Coin Team
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PULSAR_SCRIPT_PULSARCONSENSUS_H
#define PULSAR_SCRIPT_PULSARCONSENSUS_H

#include <stdint.h>

#if defined(BUILD_PULSAR_INTERNAL) && defined(HAVE_CONFIG_H)
#include <config/pulsar-config.h>
  #if defined(_WIN32)
    #if defined(DLL_EXPORT)
      #if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT)
        #define EXPORT_SYMBOL __declspec(dllexport)
      #else
        #define EXPORT_SYMBOL
      #endif
    #endif
  #elif defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBPULSARCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PULSARCONSENSUS_API_VER 1

typedef enum pulsarconsensus_error_t
{
    pulsarconsensus_ERR_OK = 0,
    pulsarconsensus_ERR_TX_INDEX,
    pulsarconsensus_ERR_TX_SIZE_MISMATCH,
    pulsarconsensus_ERR_TX_DESERIALIZE,
    pulsarconsensus_ERR_AMOUNT_REQUIRED,
    pulsarconsensus_ERR_INVALID_FLAGS,
} pulsarconsensus_error;

/** Script verification flags */
enum
{
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4), // enforce NULLDUMMY (BIP147)
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11), // enable WITNESS (BIP141)
    pulsarconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = pulsarconsensus_SCRIPT_FLAGS_VERIFY_P2SH | pulsarconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               pulsarconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | pulsarconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               pulsarconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | pulsarconsensus_SCRIPT_FLAGS_VERIFY_WITNESS
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int pulsarconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, pulsarconsensus_error* err);

EXPORT_SYMBOL int pulsarconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, pulsarconsensus_error* err);

EXPORT_SYMBOL unsigned int pulsarconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // PULSAR_SCRIPT_PULSARCONSENSUS_H
