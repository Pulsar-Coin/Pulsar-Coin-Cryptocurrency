PulsarCoin Core version 1.1.2 is now available from:

  <https://github.com/PulsarCoin/Pulsar-Coin-Cryptocurrency/releases/latest>

This is a new minor version release, with a few bugfixes and new node listing.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/PulsarCoin/Pulsar-Coin-Cryptocurrency/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over `/Applications/Pulsar-Qt` (on Mac)
or `pulsard`/`pulsar-qt` (on Linux).

The first time you run any version of the wallet, your chainstate database will 
need to be synced, which will take anywhere from a few minutes to half an hour, 
depending on the speed of your machine and the quality of your network connection.

Downgrading warning
-------------------

Wallet versions below 1.1.2 are no longer supported and will not sync properly if you 
are not able to modify configuration files on your own. Please update to the latest 
release version to improve network connectivity.

Compatibility
==============

PulsarCoin Core is extensively tested on multiple operating systems using
the Linux kernel, macOS 10.8+, and Windows Vista and later. Windows XP is not supported.

PulsarCoin Core should also work on most other Unix-like systems but is not
frequently tested on them.

Notable changes
===============

1.1.2 change log
------------------

Networking improvements: new nodes added by default.
TestNet: node added, default port modified, connection requirement lowered.
Various Typo Fixes.
Proper release notes and readme files.
Bump to version v1.1.2

1.1.1 change log
------------------

New design for overview, top menu, transaction list and About page
Minor changes in color and texts' size
Typo fix in automatic configuration file
Fix on broken links
Mac host update
Qt version update
Aarch64 and arm-gnueabihf Qt compile fix
Bump to version v1.1.1

1.1.0 change log
------------------

New design for overview and modaloverlay
New progress bar design
Minor changes in rpc console (colors - texts)
New checkpoints to secure the chain
Fix on disconnecting peers' issue
Automatic configuration file
Bump to version v1.1.0

Credits
=======

Thanks to everyone who directly contributed to this release:

PulsarCoin Core Team: Pulsar Peter
PulsarCoin Modifier: PhoenixGaming

And to those who contributed to previous releases:

\_\_I TYLE I_/_/

As well as everyone that has contributed input and community resources on [Discord](https://discord.gg/kGBcBy5dFG).
