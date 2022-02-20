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

# P2P File Sharing - Output

This is the output of the server and two clients running locally on a Windows machine.
Some of the hashes get cutoff in this document but they are unique enough that the first part of each hash is sufficient.

Note: when peers stream data to each other, they both print information about it.

Note 2: Files are deleted, added, or modified by interacting with them directly.

## Server

This is the output of the server when it was running. It shows whenever anything is happening on the server.
It also prints relevant information to a request such as a file's hash, file name or a peer's ID.

```
$ .\Server.exe
[Indexer] INFO   : Running Server
[Registry]        : New
        ID: 0
        name: Copy.txt
        hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86

[Registry]        : New
        ID: 1
        name: Copy.txt
        hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86

[Registry]        : New
        ID: 1
        name: New Text File.txt
        hash: D32E5192C14009C6BDBF44D866CF7C43255877DCDAA1739F43F80A7E0ABC1184

[Registry]        : New
        ID: 1
        name: Test.txt
        hash: 70D50811CAF34BC24F9FB2698B64B2FC6EF880537F3A27D5DE937DD065E8F5AB

[Registry]        : New
        ID: 1
        name: Test2.txt
        hash: A8776257F59232070E9930744B0AE890D40B8BB81D2E9237F8980E8C7409EDD0

[Deregister]        : Erase entry
        name: New Text File.txt
        hash: D32E5192C14009C6BDBF44D866CF7C43255877DCDAA1739F43F80A7E0ABC1184
[Deregister]        : New
        ID: 1
        name: New Text File.txt
        hash: D32E5192C14009C6BDBF44D866CF7C43255877DCDAA1739F43F80A7E0ABC1184

[Registry]        : New
        ID: 1
        name: New Text File.txt
        hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D

[Search]        : Searching for query: copy
[Request]        : Searching for hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[Request] INFO   : Entry found
        name: Copy.txt
        hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[Deregister]        : New
        ID: 1
        name: Copy.txt
        hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86

[Request]        : Searching for hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[Request] INFO   : Entry found
        name: Copy.txt
        hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[Registry]        : New
        ID: 1
        name: Copy.txt
        hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86

[List]        : Listing all files entries
[Search]        : Searching for query: i
[Request]        : Searching for hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
[Request] INFO   : Entry found
        name: New Text File.txt
        hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
[Registry]        : New
        ID: 0
        name: New Text File.txt
        hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D

```

## Client 1

This is the output of client with the id of 1. On startup we can see that all the files in the appropriate folder were hashed and then indexed on the server automatically. Any user input is wherever there is a `Client >` followed by a command. Sometimes the prompt is finicky as it might get covered. In these cases I ensured to refresh the prompt and then type my command to make it clear.

What exactly occurred on this client is as follows.

1. `search copy`
2. Show search results
3. `request EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86`
4. File already exists, show error
5. Manually delete file
6. `request EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86`
7. Connect to first available peer and begin downloading procedure
8. auto register newly downloaded file
9. `list`
10. Show every available file
11. Respond to Client 0's request
12. `q`
13. Quit

```
$ .\Client.exe -i 1 -f 1
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: localhost:55555
[Indexer] INFO   : Server pinged! 0.092000ms
[Exchanger] DEBUG  : Listening to port: 44001
[Console]        : Console start!
 Client > [File] DEBUG  : Hashing file: Copy.txt
[Folder] DEBUG  : File created: Copy.txt
[Peer] INFO   : Registered hash: Copy.txt
[File] DEBUG  : Hashing file: New Text File.txt
[Folder] DEBUG  : File created: New Text File.txt
[Peer] INFO   : Registered hash: New Text File.txt
[File] DEBUG  : Hashing file: Test.txt
[Folder] DEBUG  : File created: Test.txt
[Peer] INFO   : Registered hash: Test.txt
[File] DEBUG  : Hashing file: Test2.txt
[Folder] DEBUG  : File created: Test2.txt
[Peer] INFO   : Registered hash: Test2.txt
[File] DEBUG  : Hashing file: New Text File.txt
[Folder] DEBUG  : File modified: New Text File.txt
[Peer] INFO   : Deregistered hash: D32E5192C14009C6BDBF44D866CF7C43255877DCDAA1739F43F80A7E0ABC1184
[Peer] INFO   : Registered hash: New Text File.txt

 Client > search copy
[indexRPCParse] INFO   : Searching server file index for: copy
Hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
Added on 20/02/2022 01:24:49
        Name: Copy.txt
        Peers: 2
 Client > request EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[indexRPCParse] INFO   : Searching server peer index for: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[0] 127.0.0.1:44000
[1] 127.0.0.1:44001
[Exchanger] ERROR  : File already exists locally, not downloading: Copy.txt
 Client > [Folder] DEBUG  : File deleted: Copy.txt
[Peer] INFO   : Deregistered hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86

 Client > request EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[indexRPCParse] INFO   : Searching server peer index for: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[0] 127.0.0.1:44000
 Client > [Exchanger] DEBUG  : Connecting to peer: 0
[Exchanger Client] DEBUG  : Receiving server ID
[Exchanger Client] DEBUG  : ID match, sending hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[Exchanger Client] DEBUG  : Receiving file size
[Exchanger Client] DEBUG  : Valid filesize, notifying server: 9
[Exchanger Client] DEBUG  : Streaming
[Exchanger Client] INFO   : Finished streaming
[File] DEBUG  : Hashing file: Copy.txt
[Folder] DEBUG  : File created: Copy.txt
[Peer] INFO   : Registered hash: Copy.txt

 Client > list
[indexRPCParse] INFO   : Listing server file index
Hash: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
Added on 20/02/2022 01:24:49
        Name: Copy.txt
        Peers: 2
Hash: A8776257F59232070E9930744B0AE890D40B8BB81D2E9237F8980E8C7409EDD0
Added on 20/02/2022 01:24:50
        Name: Test2.txt
        Peers: 1
Hash: 70D50811CAF34BC24F9FB2698B64B2FC6EF880537F3A27D5DE937DD065E8F5AB
Added on 20/02/2022 01:24:50
        Name: Test.txt
        Peers: 1
Hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
Added on 20/02/2022 01:25:02
        Name: New Text File.txt
        Peers: 1
 Client > [Exchanger] DEBUG  : p2p conn: 127.0.0.1:57101
[Exchanger Server] DEBUG  : Sending ID
[Exchanger Server] DEBUG  : Reading hash size
[Exchanger Server] DEBUG  : Hash get: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
[Exchanger Server] DEBUG  : File found, sending size: 47
[Exchanger Server] DEBUG  : Waiting to stream
[Exchanger Server] DEBUG  : Streaming
[Exchanger Server] DEBUG  : Written 100%
[Exchanger Server] INFO   : Finished streaming

 Client > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```

## Client 0

This is the output of client with the id of 0. It is the same program as Client 1.

What exactly occurred on this client is as follows.

1. Respond to Client 1's request
2. `search i`
3. Show search results
4. `request 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D`
5. Connect to first available peer and begin downloading procedure
6. auto register newly downloaded file
7. `q`
8. Quit

```
 .\Client.exe -i 0 -f 0
[Indexer] INFO   : Running Client
[Indexer] DEBUG  : Connecting to index server at: localhost:55555
[Indexer] INFO   : Server pinged! 0.112000ms
[Exchanger] DEBUG  : Listening to port: 44000
[Console]        : Console start!
 Client > [File] DEBUG  : Hashing file: Copy.txt
[Folder] DEBUG  : File created: Copy.txt
[Peer] INFO   : Registered hash: Copy.txt
[Exchanger] DEBUG  : p2p conn: 127.0.0.1:57100
[Exchanger Server] DEBUG  : Sending ID
[Exchanger Server] DEBUG  : Reading hash size
[Exchanger Server] DEBUG  : Hash get: EE0F2CE65776190342FE013D46032A9D3BF7A23466CA808B42CCEA908084BC86
[Exchanger Server] DEBUG  : File found, sending size: 9
[Exchanger Server] DEBUG  : Waiting to stream
[Exchanger Server] DEBUG  : Streaming
[Exchanger Server] DEBUG  : Written 100%
[Exchanger Server] INFO   : Finished streaming

 Client > search i
[indexRPCParse] INFO   : Searching server file index for: i
Hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
Added on 20/02/2022 01:25:02
        Name: New Text File.txt
        Peers: 1
 Client > request 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
[indexRPCParse] INFO   : Searching server peer index for: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
[1] 127.0.0.1:44001
 Client > [Exchanger] DEBUG  : Connecting to peer: 1
[Exchanger Client] DEBUG  : Receiving server ID
[Exchanger Client] DEBUG  : ID match, sending hash: 25349FB263E07C0D107837A2CB661E79D18F0F88B9DEB7C67254C9C96C52450D
[Exchanger Client] DEBUG  : Receiving file size
[Exchanger Client] DEBUG  : Valid filesize, notifying server: 47
[Exchanger Client] DEBUG  : Streaming
[Exchanger Client] INFO   : Finished streaming
[File] DEBUG  : Hashing file: New Text File.txt
[Folder] DEBUG  : File created: New Text File.txt
[Peer] INFO   : Registered hash: New Text File.txt

 Client > q
[Console] INFO   : Exiting
[Folder] DEBUG  : Stopped watching path for changes
[Exchanger] DEBUG  : stopped receiving peer requests
[Indexer] INFO   : Stopping Client
```
