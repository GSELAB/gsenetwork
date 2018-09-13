# GSENetwork

Welcome to the c++ implementation of GSENetwork protocol. GSENetwork is a decentralized trust network for sharing economies that will disrupt the traditional model, eliminating the need for centralized intermediaries, through a three-pillar solution - DService, Incentive Model and Trust Network. GSENetwork envisions the future of the sharing economy as a global decentralized network which will seamlessly connect the interactions of every user, asset or service in the sharing economies anywhere in the world.

## Feature 
1. More flexible consensus mechanism
2. Support sub-chain network
3. Support D-Service Component
4. Conditional execution virtual machine environment
 
## Introduction
  * bundle: the protobuf class packaged by bundle.
  * chain: GSENetwork block chain control code.
  * config: network, storage, consensus synchronization configuration.
  * core: core common code.
  * database: GSENetwork data storage related code.
  * net: network code, including network discovery and data transfer
  * producer: GSENetwork node block logic code.
  * programs: executable program entry.
  * proto: data format definition.
  * rpc: GSENetwork remote procedure call protocol.
  * runtime: virtual machine operating environment.
  * utils: tool class directory.
  
## Setup
```
$ mkdir -p ~/GSE
$ cd ~/GSE
$ git clone https://github.com/GSELAB/gsenetwork.git
$ cd gsenetwork
```

```
$ ./compileproto.sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Run GSE Node
```
# Normal GSE Node.
$ cd programs
$ ./gsenode

# Producer
$ cd programs
$ ./gsenode -p
```