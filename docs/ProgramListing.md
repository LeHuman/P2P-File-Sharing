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

\graphicspath{ {img/} }

**CS 550 - Spring 2022**  
**Isaias Rivera**  
**A20442116**

# P2P File Sharing - Consistency - Program Listing

I did my best to include comments where the source files were modified from PA2 using

```c++
/*--------- start change ----------*/

/*--------- end change ----------*/
```

The following files were modified from PA2

- Client.cpp
- Parsers.cpp
- Exchanger.cpp
- Exchanger.h
- index.cpp
- index.h
- indexRPC.cpp
- indexRPC.h
- Peer.cpp
- Peer.h
- TestPeer.cpp
- Folder.cpp
- Folder.h
- Log.h
