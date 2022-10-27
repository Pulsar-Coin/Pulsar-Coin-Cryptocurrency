Pulsar 1.1.2

Setup
---------------------
Pulsar Core is the original Pulsar client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Pulsar transactions (which is currently around 500Mbs); depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

To download Pulsar Core, visit [GitHub Releases](https://github.com/PulsarCoin/Pulsar-Coin-Cryptocurrency/releases/latest).

Running
---------------------
The following are some helpful notes on how to run Pulsar on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/pulsar-qt` (GUI) or
- `bin/pulsard` (headless)

Data Directory Location:
- Unix: ~/.pulsar

### Windows

Unpack the files into a directory, and then run pulsar-qt.exe.

Data Directory Locations

- Windows < Vista: C:\Documents and Settings\Username\Application Data\Pulsar
- Windows >= Vista: C:\Users\Username\AppData\Roaming\Pulsar

### OS X

Drag Pulsar-Core to your applications folder, and then run Pulsar-Core.

Data Directory Location:
- Mac: ~/Library/Application Support/Pulsar

### Need Help?

* See the documentation at the [Bitcoin Wiki](https://en.bitcoin.it/wiki/Main_Page)
for help and more information. Pulsar is very similar to bitcoin, so you can use their wiki.
* Ask for help on [#general](https://discord.gg/ycV7abbF7M) in the Official Pulsar Coin Discord Channel.

Building
---------------------
The following are developer notes on how to build Pulsar on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The Pulsar repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](none-yet)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Travis CI](travis-ci.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Resources
* TODO: add some pulsar resources

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
