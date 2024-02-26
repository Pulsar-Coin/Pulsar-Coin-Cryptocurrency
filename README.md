# Pulsar Coin - Update 1.1.6
***
<a href="https://imgur.com/EmoVsgC"><img src="https://i.imgur.com/EmoVsgC.png" title="source: imgur.com" /></a>

## Coin Specifications:
#### Total Supply: 500,000,000 (500 Million PLSR)
#### Pre-mined Supply: 50,000,000 (50 Million PLSR | 10%)
#### Launch Supply: 450,000,000 (450 Million PLSR | 90%)
#### Block Maturity: 41
#### Block Time: 75 Seconds
#### Difficulty Time: 2,064 Blocks
#### Block Rewards (Proof of Stake): 45 PLSR
#### P2P Port: 5995
#### RPC Port: 5996
#### Total POS & POW rewards phase: 202.2 years
## Installation:
1. Navigate to [Downloads](https://github.com/Pulsar-Coin/Pulsar-Coin-Cryptocurrency/releases/latest)
    1. Download and extract the folder designated to your Operating System in the location of your choice
      * The act of downloading this software and receiving errors from Windows, Mac, or protection software is normal.
      * If a firewall pop-up appears, allow Pulsar - Wallet to communicate on both Private and Public Networks
2. Once the file is extracted, run "pulsar-qt" as administrator
    1. The application will attempt to connect to peer nodes and locally index the blockchain, this will take some time
      * Do not attempt any actions or transactions during this process, user interaction may negatively impact indexing
      * Update to the local index will take place every time you open "pulsar-qt". The amount of time this process takes depends on how long the program has been closed / not connected to public nodes.
3. Once indexing has completed, we advice users to navigate to: Settings (top left) -> Options (bottom of drop-down menu) -> Open Configuration File (button towards the bottom of options pop-up menu).
    1. A Configuration Options pop-up menu will appear with an override warning, click Ok
    2. A simple text file will open and should be named "pulsar.conf". This is where the configuration settings for your desktop wallet are kept. Settings should be pre-configured but you can edit this file to your liking. Navigate below this section to find "Config File" and that will contain all options and their functionality.
      * Editing parameters not defined below in "Config File" is not recommended and may cause serious damage the local wallet application and it's files
    3. Save "pulsar.conf" without altering the filename or filepath.
      * Please close the pulsar-qt application
        * This allows for your configuration file changes to be taken into effect.
4. Upon re-opening "pulsar-qt", it's recommended that each user navigate to: Settings (top left) -> Encrypt Wallet (top option of dropdown menu)
  * Encrypting your wallet enables staking and also adds an extra level of security to your pulsar coins.
    1. Choose a passcode that is unique to you (We recommend at least 12 characters with 1+ Capital letters, 1+ Symbols, and 1+ Numbers)
      * Record your passcode, preferably on physical paper and not in a digital location. This will be needed in order to Send & Stake pulsar coins on your local wallet
5. Lastly, open File -> Receiving addresses. Click the + New button in the bottom left, add a Label and click Ok.
  * This will create your Pulsar Address designation and can be used from 3rd party applications and other users to send your account pulsar coins.
6. You're all ready to go!
## Config File:
#### addnode= : Insert IP address for peer node (Ex. addnode=85.215.71.19:5995)
* Note: Please remove all entires of _addnode=_ in _pulsar.conf_ as appropriate values are already hard-coded in the wallet.
* Visit (link coming soon) for approved team & community nodes
* The 5995 is the RPC port, this must be present at the end of every added node address
#### banscore= : Please remove all entires of _banscore=_ in _pulsar.conf_ as appropriate values are already hard-coded in the wallet.
#### rpcuser= : This will set the username for secure JSON-RPC api interaction
#### rpcpassword= : This will set the password for secure JSON-RPC api interaction
 * Just like pulsar-qt encryption, record rpcuser & rpcpassword where you can access it. These do not need to be included if you're not dealing with apps other than pulsar-qt
#### server= : server=1 tells Pulsar to accept JSON-RPC commands.
#### listen= : Pulsar can send and receive commands on the specified TCP port (Default: listen=1)
#### daemon= : The daemon is PLSR software running in the background or in command prompts. RPC applications use this configuration setting (daemon=1)
* This setting should either be set to (daemon=1) or not included in config file
#### staking= : This setting should always be defined (Default: staking=1)
* This setting must be defined in order to participate in staking rewards.
* staking=0 means that your local wallet won't receive staking rewards whether your wallet is encrypted or not 
#### port= : This setting should always be defined (Default: port=5995)
#### rpcport= : This setting is only needed if an RPC connection is needed (Ex. Explorer, Pool, Custom Application)
#### maxconnections= : This setting defines how many public nodes can be connected to your wallet in a given time.
* The more connections you have, the more reliable your index becomes, but this comes at the cost of using more network data
* The default for this parmeter would be around 20 - 50. Unless your local system is running a blockchain application (Ex. Explorer or Pool), changing this won't affect your wallet much.
#### rpcallowip= : This is usually 127.0.0.1 by default, this is not needed unless an RPC connection is in-use
## How to mine Pulsar:
1. You can use 3rd party mining software to mine PLSR. Navigate to your chosen miner. We recommend [SRBMiner](https://github.com/doktor83/SRBMiner-Multi)
  * Please *read carefully* through your chosen coin-miner documentation and their defined fees.
    * The Pulsar Team does not own, manage, or control any coin-mining entity and *can not be at fault by any wrong doings 3rd party software engages in.* We can, however, **help you be safe and secure when operating this software.** Team supported software, such as SRBMiner, should take priority over other 3rd party software if you're new to the *cryptocurrency ecosystem*
2. Follow your miner's guide on setting up. Usually batch file (.bat) can be made along with your specifications
  * You can find mining software that offers mining different algorithms. PLSR can be mined on the CurveHash algorithm (CPU + GPU) and MinotaurX algorithm (CPU Only)
    * Depending on your physical hardware and specifications, the hash rate of your mining effort will vary.
3. A correct mining application (and correct mining configuration file) will open a terminal window and confirmation prompts will continue to accumulate.

## Explorer
<br> Pulsar Coin Explorer: [PLSR Explorer](https://explorer.pulsarcoin.info/)

## Links
<br> Wallet Downloads: [PLSR Downloads](https://github.com/Pulsar-Coin/Pulsar-Coin-Cryptocurrency/releases/latest)
<br> Website: [https://pulsarcoin.info](https://pulsarcoin.info)
<br> Whitepaper: [Whitepaper](https://drive.google.com/file/d/1PNd9VrxJqK3qUKYHVzck79Aw82E-CLt-/view?usp=sharing)
<br> 2.0 Proposal: [Proposal](https://drive.google.com/file/d/1kRlu-YnWpx8ye7JhOPY83268fL01D30F/view?usp=sharing)
<br> BitcoinTalk: [Welcome to the Pulsar Network](https://bitcointalk.org/index.php?topic=5383700)

### Social Media
<br> X: [Twitter](https://twitter.com/TeamPulsarCoin).
<br> Discord: [Click Here](https://discord.com/invite/VuDakSctNX)
<br> YouTube: [@TeamPulsarCoin](https://www.youtube.com/@TeamPulsarCoin)
<br> Instagram: [teampulsarcoin](https://www.instagram.com/teampulsarcoin/)
<br> LinkTree: [Linktr.ee](https://linktr.ee/teampulsarcoin)

## Exchanges
### Options below may have **more pairs** than just PLSR/BTC
#### XeggeX: https://xeggex.com/market/PLSR_BTC
#### Exbitron: https://exbitron.com/trade?market=plsr-btc

## Supported Wallets
#### Desktop Wallet: [Windows/Linux](https://github.com/Pulsar-Coin/Pulsar-Coin-Cryptocurrency/releases/latest)
#### Discord Wallet: [Discord](https://discord.com/invite/VuDakSctNX)

## Coin Trackers
#### CoinPaprika: https://coinpaprika.com/coin/plsr-pulsar-coin/
#### CoinGecko: https://www.coingecko.com/en/coins/pulsar-coin
#### CoinMarketCap: ***Coming Soon***
#### BlockSpot: https://blockspot.io/coin/pulsar-coin/

## Support
Please e-mail us any with any concerns you may come across 
<br> [Email Me](mailto:team@pulsarcoin.info)

## Support the Team, Developers, and the Project
**Development Funds MultiSig Wallets**
<br>PLSR: PuxaWK9Bizsrmk3N4EfxFsvk2k5C6DbikR
<br>LTC: ltc1q5vn626f996lx3lgc7s7rfyjzwkq6jm6fh9g2ly9c528w5jgsrr0szsfd9e
