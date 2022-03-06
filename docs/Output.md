---
geometry:

- margin=1in
- letterpaper

classoption:    table
documentclass:  extarticle
urlcolor:       blue
fontsize:       11pt
header-includes: |
    \usepackage{multicol}
    \usepackage{graphicx}
    \usepackage{float}
...

\normalsize
\pagenumbering{gobble}
\pagenumbering{arabic}

**CS 550 - Spring 2022**  
**Isaias Rivera**  
**A20442116**

# P2P File Sharing - Super-peers - Output

This is the output of two super-peers and two leaf-nodes running locally on a Windows machine.
Some of the hashes get cutoff in this document but they are unique enough that the first part of each hash is sufficient.

Notes:

- When peers stream data to each other, they both print information about it
- Files are deleted, added, or modified by interacting with them directly in their respective directory
- Propagated requests print their unique message id alongside the log message to help keep track of each request
- This is the output when running the same setup and commands as in the Manual document

## Super-peer 0

```
$ .\Client.exe -i 0 -c test_config_simple.json
[Indexer] INFO   : Running Server
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47900
[Indexer] INFO   : Server pinged! 0.083000ms
[Exchanger] DEBUG  : Listening to port: 48900
[Console]        : Console start!
 [File] DEBUG  : Hashing file: dup.txt
Client-0 > [Folder] DEBUG  : File created: dup.txt
[Registry]        : New
        ID: 0
        name: dup.txt
        hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F

[Peer] INFO   : Registered hash: dup.txt
[Registry]        : New
        ID: 2
        name: 2.txt
        hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D

[Indexer] DEBUG  : Request Listing 1
[List]        : Listing all files entries
[Indexer] DEBUG  : TTL finished 797849837586022540
[Indexer] DEBUG  : Returning results
[Indexer] DEBUG  : Request File 1
[Request]        : Searching for hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[Request] INFO   : Entry found
        name: dup.txt
        hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[Indexer] DEBUG  : Request Search 1
[Search]        : Searching for query: dup

 Client-0 > list
[indexRPCParse] INFO   : Listing server file index
[Indexer] DEBUG  : Request Listing -1
[List]        : Listing all files entries
[Indexer] DEBUG  : Propagating query list 7721487775885090099 1
[Indexer] DEBUG  : Returning results
Hash: 76BD6080BE28BDF33FA030A29A6D65849C2684F2F93DC3A41D3668801DF29FB1
Added on 05/03/2022 22:59:31
        Name: 3.txt
        Peers: 1
Hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
Added on 05/03/2022 22:59:28
        Name: dup.txt
        Peers: 3
Hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
Added on 05/03/2022 22:59:30
        Name: 2.txt
        Peers: 1
 Client-0 > [Indexer] DEBUG  : Request File 1
[Request]        : Searching for hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[Request] INFO   : Entry found
        name: 2.txt
        hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D

 Client-0 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Super-peer 1

```
$ .\Client.exe -i 1 -c test_config_simple.json
[Indexer] INFO   : Running Server
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47901
[Indexer] INFO   : Server pinged! 0.095000ms
[Exchanger] DEBUG  : Listening to port: 48901
[Console]        : Console start!
 [FileClient-1 > ] DEBUG  : Hashing file: dup.txt
[Folder] DEBUG  : File created: dup.txt
[Registry]        : New
        ID: 1
        name: dup.txt
        hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F

[Peer] INFO   : Registered hash: dup.txt
[Registry]        : New
        ID: 3
        name: 3.txt
        hash: 76BD6080BE28BDF33FA030A29A6D65849C2684F2F93DC3A41D3668801DF29FB1

[Indexer] DEBUG  : Request Listing -1
[List]        : Listing all files entries
[Indexer] DEBUG  : Propagating query list 797849837586022540 1
[Indexer] DEBUG  : Returning results
[Indexer] DEBUG  : Request File -1
[Request]        : Searching for hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[Request] INFO   : Entry found
        name: dup.txt
        hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[Indexer] DEBUG  : Propagating query request 3498333107492499847 1
[Exchanger] DEBUG  : p2p conn: 127.0.0.1:59862
[Exchanger Server] DEBUG  : Sending ID
[Exchanger Server] DEBUG  : Reading hash size
[Exchanger Server] DEBUG  : Hash get: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[Exchanger Server] DEBUG  : File found, sending size: 24
[Exchanger Server] DEBUG  : Waiting to stream
[Exchanger Server] DEBUG  : Streaming
[Exchanger Server] DEBUG  : Written 100%
[Exchanger Server] INFO   : Finished streaming
[Registry]        : New
        ID: 3
        name: dup.txt
        hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F

[Indexer] DEBUG  : Request Search -1
[Search]        : Searching for query: dup
[Indexer] DEBUG  : Propagating query search 14412114876339529566 1
[Indexer] DEBUG  : Request Listing 1
[List]        : Listing all files entries
[Indexer] DEBUG  : TTL finished 7721487775885090099
[Indexer] DEBUG  : Returning results

 Client-1 > request 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[indexRPCParse] INFO   : Searching server peer index for: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[Indexer] DEBUG  : Request File -1
[Request]        : Searching for hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[Request] WARNING: No entries found with hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[Indexer] DEBUG  : Propagating query request 14638757213955201442 1
[2] 127.0.0.1:48902
 Client-1 > [Exchanger] DEBUG  : Connecting to peer: 2
[Exchanger Client] DEBUG  : Receiving server ID
[Exchanger Client] DEBUG  : ID match, sending hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[Exchanger Client] DEBUG  : Receiving file size
[Exchanger Client] DEBUG  : Valid filesize, notifying server: 29
[Exchanger Client] DEBUG  : Streaming
[Exchanger Client] INFO   : Finished streaming
[File] DEBUG  : Hashing file: 2.txt
[Folder] DEBUG  : File created: 2.txt
[Registry]        : New
        ID: 1
        name: 2.txt
        hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D

[Peer] INFO   : Registered hash: 2.txt

 Client-1 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Leaf-node 2

```
$ .\Client.exe -i 2 -c test_config_simple.json
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47900
[Indexer] INFO   : Server pinged! 0.083000ms
[Exchanger] DEBUG  : Listening to port: 48902
[Console]        : Console start!
 [Client-2 > File] DEBUG  : Hashing file: 2.txt
[Folder] DEBUG  : File created: 2.txt
[Peer] INFO   : Registered hash: 2.txt
[Exchanger] DEBUG  : p2p conn: 127.0.0.1:59866
[Exchanger Server] DEBUG  : Sending ID
[Exchanger Server] DEBUG  : Reading hash size
[Exchanger Server] DEBUG  : Hash get: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
[Exchanger Server] DEBUG  : File found, sending size: 29
[Exchanger Server] DEBUG  : Waiting to stream
[Exchanger Server] DEBUG  : Streaming
[Exchanger Server] DEBUG  : Written 100%
[Exchanger Server] INFO   : Finished streaming

 Client-2 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Leaf-node 3

```
$ .\Client.exe -i 3 -c test_config_simple.json
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47901
[Indexer] INFO   : Server pinged! 0.089000ms
[Exchanger] DEBUG  : Listening to port: 48903
[Console]        : Console start!
[File] DEBUG  :  Client-3 > Hashing file: 3.txt
[Folder] DEBUG  : File created: 3.txt
[Peer] INFO   : Registered hash: 3.txt

 Client-3 > list
[indexRPCParse] INFO   : Listing server file index
Hash: 8C8C5354325CFD52914A7B36D8A28695F5DEDCBBABEE7C13BE2E02B0CAEF5A6D
Added on 05/03/2022 22:59:30
        Name: 2.txt
        Peers: 1
Hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
Added on 05/03/2022 22:59:28
        Name: dup.txt
        Peers: 2
Hash: 76BD6080BE28BDF33FA030A29A6D65849C2684F2F93DC3A41D3668801DF29FB1
Added on 05/03/2022 22:59:31
        Name: 3.txt
        Peers: 1
 Client-3 > request 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[indexRPCParse] INFO   : Searching server peer index for: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[1] 127.0.0.1:48901
[0] 127.0.0.1:48900
 Client-3 > [Exchanger] DEBUG  : Connecting to peer: 1
[Exchanger Client] DEBUG  : Receiving server ID
[Exchanger Client] DEBUG  : ID match, sending hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
[Exchanger Client] DEBUG  : Receiving file size
[Exchanger Client] DEBUG  : Valid filesize, notifying server: 24
[Exchanger Client] DEBUG  : Streaming
[Exchanger Client] INFO   : Finished streaming
[File] DEBUG  : Hashing file: dup.txt
[Folder] DEBUG  : File created: dup.txt
[Peer] INFO   : Registered hash: dup.txt

 Client-3 > search dup
[indexRPCParse] INFO   : Searching server file index for: dup
Hash: 395A71419DADACF6653548C449A207536831AF86ECF265E2067E0377A14A447F
Added on 05/03/2022 22:59:28
        Name: dup.txt
        Peers: 3
 Client-3 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```
