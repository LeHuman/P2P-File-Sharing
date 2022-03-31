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

# P2P File Sharing - Consistency - Output

This is the output of two super-peers and two leaf-nodes running locally on a Windows machine.
Some of the hashes get cutoff in this document but they are unique enough that the first part of each hash is sufficient.

Notes:

- When peers stream data to each other, they both print information about it
- Files are deleted, added, or modified by interacting with them directly in their respective directory
- Propagated requests print their unique message id alongside the log message to help keep track of each request
- This is the output when running the same setup and commands as in the Manual document

## Super-peer 0

```
$ .\Client.exe -i 0 -s
[Indexer] INFO   : Running Server
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47900
[Indexer] INFO   : Server pinged! 0.137000ms
[Exchanger] DEBUG  : Listening to port: 48900
[Console]        : Console start!
 [File] DEBUG  : Hashing file: file0.txt
Client-0 > [Folder] DEBUG  : File created: file0.txt
[Registry] INFO   : New Origin
        ID: 0
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7

[Registry]        : New
        ID: 0
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7

[Peer] INFO   : Registered hash: file0.txt
[Registry] INFO   : New Origin
        ID: 2
        name: file2.txt
        hash: 3377870DFEAAA7ADF79A374D2702A3FDB13E5E5EA0DD8AA95A802AD39044A92F

[Registry]        : New
        ID: 2
        name: file2.txt
        hash: 3377870DFEAAA7ADF79A374D2702A3FDB13E5E5EA0DD8AA95A802AD39044A92F


 Client-0 > [Indexer] DEBUG  : Request Listing 1
[List]        : Listing all files entries
[Indexer] INFO   : TTL finished 797849837586022540
[Indexer] DEBUG  : Returning results
[Indexer] DEBUG  : Request File 1
[Request]        : Searching for hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Request] INFO   : Entry found
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger] DEBUG  : p2p conn: 127.0.0.1:58177
[Exchanger Server] DEBUG  : P2PFile
[Exchanger Server] DEBUG  : Sending ID
[Exchanger Server] DEBUG  : Hash size get: 64
[Exchanger Server] DEBUG  : Hash get: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger Server] DEBUG  : File found, sending size: 5
[Exchanger Server] DEBUG  : Waiting to stream
[Exchanger Server] DEBUG  : Streaming
[Exchanger Server] DEBUG  : Written 100%
[Exchanger Server] INFO   : Finished streaming
[Indexer] DEBUG  : Request Search -1
[Search]        : Searching for query: file
[Indexer] INFO   : Propagating query search 12328414912905561117 1
[Indexer] DEBUG  : Returning results
[File] DEBUG  : Hashing file: file0.txt
[Folder] DEBUG  : File modified: file0.txt : 15
[Deregister]        : Erase entry
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Deregister]        : Erase peer
        ID: 0
[Deregister]        : New
        ID: 0
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7

[Peer] INFO   : Deregistered hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Registry] INFO   : New Origin
        ID: 0
        name: file0.txt
        hash: 6AAA22848C60483715FA71B87BFB3DE4A9C2C14A6DD8CAECDE650D386AEBD051

[Registry]        : New
        ID: 0
        name: file0.txt
        hash: 6AAA22848C60483715FA71B87BFB3DE4A9C2C14A6DD8CAECDE650D386AEBD051

[Peer] INFO   : Registered hash: file0.txt
[originFolderListener] DEBUG  : Propagating invalidation
[Indexer] DEBUG  : Request Invalidation -1
[Invalidate] WARNING: Entry not found: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Invalidate] ERROR  : Entry was not found or is invalid
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Indexer] INFO   : Pushing invalidation: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Indexer] INFO   : Propagating query invalidate 7721487775885090099 1
[Peer] WARNING: Unable to invalidate hash or no peers to locally invalidate: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger] DEBUG  : p2p conn: 127.0.0.1:58202
[Exchanger Server] DEBUG  : P2PFileUpdate
[Exchanger Server] DEBUG  : Sending ID
[Exchanger Server] DEBUG  : Hash size get: 64
[Exchanger Server] DEBUG  : Hash get: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger Server] DEBUG  : Name get: file0.txt
[Exchanger Server] DEBUG  : File found, sending size: 15
[Exchanger Server] DEBUG  : Waiting to stream
[Exchanger Server] DEBUG  : Streaming
[Exchanger Server] DEBUG  : Written 100%
[Exchanger Server] INFO   : Finished streaming
[Indexer] DEBUG  : Request Search 1
[Search]        : Searching for query: file0
[Indexer] INFO   : TTL finished 14638757213955201442
[Indexer] DEBUG  : Returning results

 Client-0 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Super-peer 1

```
$ .\Client.exe -i 1 -s
[Indexer] INFO   : Running Server
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47901
[Indexer] INFO   : Server pinged! 0.129000ms
[Exchanger] DEBUG  : Listening to port: 48901
[Console]        : Console start!
 Client-1 > [File] DEBUG  : Hashing file: file1.txt
[Folder] DEBUG  : File created: file1.txt
[Registry] INFO   : New Origin
        ID: 1
        name: file1.txt
        hash: C147EFCFC2D7EA666A9E4F5187B115C90903F0FC896A56DF9A6EF5D8F3FC9F31

[Registry]        : New
        ID: 1
        name: file1.txt
        hash: C147EFCFC2D7EA666A9E4F5187B115C90903F0FC896A56DF9A6EF5D8F3FC9F31

[Peer] INFO   : Registered hash: file1.txt
[Registry] INFO   : New Origin
        ID: 3
        name: file3.txt
        hash: 6F3FEF6DC51C7996A74992B70D0C35F328ED909A5E07646CF0BAB3383C95BB02

[Registry]        : New
        ID: 3
        name: file3.txt
        hash: 6F3FEF6DC51C7996A74992B70D0C35F328ED909A5E07646CF0BAB3383C95BB02


 Client-1 > [Indexer] DEBUG  : Request Listing -1
[List]        : Listing all files entries
[Indexer] INFO   : Propagating query list 797849837586022540 1
[Indexer] DEBUG  : Returning results
[Indexer] DEBUG  : Request File -1
[Request]        : Searching for hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Request] WARNING: No entries found with hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Indexer] INFO   : Propagating query request 3498333107492499847 1
[Registry]        : New
        ID: 3
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7

[Entry] DEBUG  : TTR Updated: file0.txt:1648694786
[Indexer] DEBUG  : Request Search 1
[Search]        : Searching for query: file
[Indexer] INFO   : TTL finished 12328414912905561117
[Indexer] DEBUG  : Returning results
[Indexer] DEBUG  : Request Invalidation 1
[Invalidate]        : Invalidate
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7

[Indexer] INFO   : Pushing invalidation: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger Invalidation] DEBUG  : Connecting to peer: 3
[Exchanger Invalidation] DEBUG  : Sending mode
[Exchanger Invalidation] DEBUG  : sending hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Indexer] INFO   : TTL finished 7721487775885090099
[Registry]        : New
        ID: 3
        name: file0.txt
        hash: 6AAA22848C60483715FA71B87BFB3DE4A9C2C14A6DD8CAECDE650D386AEBD051

[Entry] DEBUG  : TTR Updated: file0.txt:-1
[Deregister]        : Erase entry
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Deregister]        : New
        ID: 3
        name: file0.txt
        hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7

[Registry] WARNING: Peer and Entry already refrence each other
        ID: 3
        hash: 6AAA22848C60483715FA71B87BFB3DE4A9C2C14A6DD8CAECDE650D386AEBD051

 Client-1 > search file0
[indexRPCParse] INFO   : Searching server file index for: file0
[Indexer] DEBUG  : Request Search -1
[Search]        : Searching for query: file0
[Indexer] INFO   : Propagating query search 14638757213955201442 1
[Indexer] DEBUG  : Returning results
Hash: 6AAA22848C60483715FA71B87BFB3DE4A9C2C14A6DD8CAECDE650D386AEBD051
Added on 30/03/2022 21:50:29
        Name: file0.txt
        Peers: 2
 Client-1 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Leaf-node 2

```
$ .\Client.exe -i 2 -s
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47900
[Indexer] INFO   : Server pinged! 0.156000ms
[Exchanger] DEBUG  : Listening to port: 48902
[Console]        : Console start!
 Client-2[File > ] DEBUG  : Hashing file: file2.txt
[Folder] DEBUG  : File created: file2.txt
[Peer] INFO   : Registered hash: file2.txt

 Client-2 > search file
[indexRPCParse] INFO   : Searching server file index for: file
Hash: 6F3FEF6DC51C7996A74992B70D0C35F328ED909A5E07646CF0BAB3383C95BB02
Added on 30/03/2022 21:43:41
        Name: file3.txt
        Peers: 1
Hash: C147EFCFC2D7EA666A9E4F5187B115C90903F0FC896A56DF9A6EF5D8F3FC9F31
Added on 30/03/2022 21:43:40
        Name: file1.txt
        Peers: 1
Hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
Added on 30/03/2022 21:43:40
        Name: file0.txt
        Peers: 2
Hash: 3377870DFEAAA7ADF79A374D2702A3FDB13E5E5EA0DD8AA95A802AD39044A92F
Added on 30/03/2022 21:43:40
        Name: file2.txt
        Peers: 1
 Client-2 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Leaf-node 3

```
$ .\Client.exe -i 3 -s
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: 127.0.0.1:47901
[Indexer] INFO   : Server pinged! 0.136000ms
[Exchanger] DEBUG  : Listening to port: 48903
[Console]        : Console start!
 Client-3[ > File] DEBUG  : Hashing file: file3.txt
[Folder] DEBUG  : File created: file3.txt
[Peer] INFO   : Registered hash: file3.txt

 Client-3 > list
[indexRPCParse] INFO   : Listing server file index
Hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
Added on 30/03/2022 21:43:40
        Name: file0.txt
        Peers: 1
Hash: 3377870DFEAAA7ADF79A374D2702A3FDB13E5E5EA0DD8AA95A802AD39044A92F
Added on 30/03/2022 21:43:40
        Name: file2.txt
        Peers: 1
Hash: 6F3FEF6DC51C7996A74992B70D0C35F328ED909A5E07646CF0BAB3383C95BB02
Added on 30/03/2022 21:43:41
        Name: file3.txt
        Peers: 1
Hash: C147EFCFC2D7EA666A9E4F5187B115C90903F0FC896A56DF9A6EF5D8F3FC9F31
Added on 30/03/2022 21:43:40
        Name: file1.txt
        Peers: 1
 Client-3 > request 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[indexRPCParse] INFO   : Searching server peer index for: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[0] 127.0.0.1:48900
 Client-3 > [Exchanger] DEBUG  : Connecting to peer: 0:127.0.0.1:48900
[Exchanger Client] DEBUG  : Sending mode
[Exchanger Client] DEBUG  : Receiving server ID
[Exchanger Client] DEBUG  : ID match, sending hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger Client] DEBUG  : Receiving file size and origin
[Exchanger Client] DEBUG  : Origin get: 0:127.0.0.1:48900:0
[Exchanger Client] DEBUG  : Valid filesize, notifying server: 5
[Exchanger Client] DEBUG  : Streaming
[Exchanger Client] INFO   : Finished streaming, storing local file
[File] DEBUG  : Hashing file: file0.txt
[Exchanger Client] DEBUG  : Running finished listener for hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Peer] INFO   : Registered hash: file0.txt
[File] DEBUG  : Hashing file: file0.txt
[Folder] DEBUG  : File created: file0.txt
[Exchanger] DEBUG  : p2p conn: 127.0.0.1:58201
[Exchanger Server] DEBUG  : InvalidateFile
[Exchanger Server] DEBUG  : Hash size get: 64
[Exchanger Server] DEBUG  : Hash get: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger Server] DEBUG  : Running invalidation listener for hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Invalidator] DEBUG  : Origin server to connect: 127.0.0.1:48900
[Exchanger] DEBUG  : Connecting to peer: 0:127.0.0.1:48900
[Exchanger Client] DEBUG  : Sending mode
[Exchanger Client] DEBUG  : Receiving server ID
[Exchanger Client] DEBUG  : ID match, sending hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Exchanger Client] DEBUG  : Receiving file size and origin
[Exchanger Client] DEBUG  : Origin get: 0:127.0.0.1:48900:0
[Exchanger Client] DEBUG  : Valid filesize, notifying server: 15
[Exchanger Client] DEBUG  : Streaming
[Exchanger Client] INFO   : Finished streaming, storing local file
[File] DEBUG  : Hashing file: file0.txt
[Exchanger Client] DEBUG  : Running finished listener for hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Peer] INFO   : Registered hash: file0.txt
[File] DEBUG  : Hashing file: file0.txt
[Folder] DEBUG  : File modified: file0.txt : 15
[Peer] INFO   : Deregistered hash: 56F3FD843F7AE959A8409E0AE7C067A0E862A6FAA7A22BAD147EE90EE5992BD7
[Peer] INFO   : Registered hash: file0.txt

 Client-3 > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```
