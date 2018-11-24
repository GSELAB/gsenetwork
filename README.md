# GSENetwork

Welcome to the C++ implementation of GSENetwork protocol. GSENetwork is a decentralized trust network for sharing economies that will disrupt the traditional model, eliminating the need for centralized intermediaries, through a three-pillar solution - DService, Incentive Model and Trust Network. GSENetwork envisions the future of the sharing economy as a global decentralized network which will seamlessly connect the interactions of every user, asset or service in the sharing economies anywhere in the world.
 
## Introduction
  * chain: GSENetwork block chain module.
  * config: Configuration module.
  * core: Basic module.
  * database: Data storage implementation.
  * net: Network module.
  * producer: Block producer module.
  * programs: GSENetwork node program entry.
  * rpc: Remote procedure call interface.
  * runtime: Virtual machine operating environment.
  
## Prerequisites
```
cmake
```

## Installation
```
$ mkdir -p ~/GSE
$ cd ~/GSE
$ git clone https://github.com/GSELAB/gsenetwork.git
$ cd gsenetwork
$ mkdir build
$ cd build
$ cmake ..
$ make
```


## Deployment
GSE node can run on testnet or mainnet. On testnet, six block producing nodes has been deployed, which means any node deployed on testnet will receive blocks from these six producers. You may use the [gsenetwork-client](https://github.com/GSELAB/gsenetwork-cli) to make transaction, vote and get blockchain information, etc.

Currently, there are no blocks being produced on mainnet.

### Setup GSE Testnet Node
Build testnet directory and copy the testnet configuration file [testnet_config](testnet_config) into it.
```
$ mkdir -p ~/GSE/testnet_node
$ cp ~/GSE/gsenetwork/testnet_config ~/GSE/testnet_node
$ cp ~/GSE/gsenetwork/build/programs/gsenode ~/GSE/testnet_node
$ cd ~/GSE/testnet_node
```

Below are the six predefined nodes' addresses and ports. Please feel free to add ip addresses to the testnet configuration file if needed. 
```
    "node_list": [
        "132.232.52.144:60606",
        "132.232.52.133:60606",
        "132.232.52.156:60606",
        "54.255.241.130:60606",
        "54.169.253.99:60606",
        "13.229.53.152:60606"
    ],
```

You may run multiple nodes on the same computer. Repeat the instructions above and make sure NO nodes are sharing the same port for rpc service. For example, if the first node is using port 50505, you may choose port 50506 for the second node.
```
    "rpc": {
        "ip_port": "0.0.0.0:50505"
    },

```

### Setup GSE Mainnet Node
Build mainnet directory and copy the mainnet configuration file [mainnet_config](mainnet_config) into it. 
```
$ mkdir -p ~/GSE/mainnet_node
$ cp ~/GSE/gsenetwork/mainnet_config ~/GSE/mainnet_node
$ cp ~/GSE/gsenetwork/programs/gsenode ~/GSE/mainnet_node
$ cd ~/GSE/mainnet_node
```

## Running
Before running, you have to set your private key in the "key" section of the configuration file.
```
     "key": {
         "secret": "0000000000000000000000000000000000000000000000000000000000000000"
     },
```

The first time you run GSE node, it will try to synchronize data from other nodes, which may take hours or even days.
```
Usage:
    -c arg                Specify testnet_config or main_net_config (required)
    -r                    Enable rpc service (optinoal)
    -p                    Enable producer mode (optional)
```
For example, to run node on testnet while having rpc service and producer mode enabled:
```
$ ./gsenode -c testnet_config -r -p
[2018-11-25 20:47:51.105234] [0x000000010979d5c0] [info]    Config:testnet_config

GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG                 GGGSSS                 SSSEEE                 EEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGGGGGG   GGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGGSSS                 SSSEEE                 EEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGG       GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGGGGGG   GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG   GGGGGGGGGGG   GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGG                 GGGSSS                 SSSEEE                 EEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
[2018-11-25 20:47:51.116640] [0x000000010979d5c0] [info]    Turn on producer service
[2018-11-25 20:47:51.116654] [0x000000010979d5c0] [info]    Turn on rpc service
```

To run node on maintnet while having rpc service enabled and producer mode disabled:
```
$ ./gsenode -c mainnet_config -r
```

## License
This project is licensed under GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
