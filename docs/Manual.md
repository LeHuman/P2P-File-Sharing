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

# P2P File Sharing - Manual

## Note

This manual only covers how to use the program. Terms that will be not explained in this manual are `ip address`, `ports`, `port forwarding`, `directories`, and `file hashes`.
If this program is only needed to run locally, the *Simple* examples should work just fine. However, this manual is also mainly for Windows, meaning, the exact syntax of some commands may depend on your system.

## Procedure

The typical procedure for using this program is as follows

1. Create valid folders for each client
2. Move files to folders for each client that want to be shared
3. Create the static connection config (or use the one provided)
4. Start at least two clients, using the folders previously created, and where at least one of the clients is a super-peer
5. On one client, search for file
6. Request file, given the file hash (The long string of characters) from searching
7. Wait for file download or repeat steps 5-6
8. Close all clients

## Program arguments

A \<int\> just means it should be a number.

### Client

Usage:

   Client  [-ah] [--version] [-c \<filePath\>] [-f \<directory\>] [-i \<int\>]

Where:

   -i \<int\>,  --identity \<int\>
     Unique ID identifying this client

   -c \<filePath\>,  --configFile \<filePath\>
     The config file to use

   -f \<directory\>,  --downloadFolder \<directory\>
     The local folder files should be uploaded and downloaded to

   -a,  --all2all
     enable all2all mode

#### Example

##### *Simple* - Local Only

`.\Client.exe -i 0 -c "test_config.json" -f "watchFolder"`

## Using the Interactive Console

The main interface with this program involves using the terminal of your device when using the client.

The following options are allowed when using the client.

- `ping`
  - Ping the server to check it's response time, meaning, how long the server takes to respond
- `list`
  - List all the files on the indexing server, meaning, show all the files we can either download or already have
- `search [query]`
  - Search for the query as a substring the in the name of all the files on the indexing server, meaning, search for a file where `query` is what you want to look for
  - Example: `search am`
    - Search for files with an `am` in it's name
- `request [hash]`
  - Request a specific file, given the hash, meaning, copy the `hash` (The long string of characters) from a single file when you did either a `search` or `list` and paste it where `hash` is.
  - Example: `request E2D0FE1585A63EC6009C8016FF8DDA8B17719A637405A4E23C0FF81339148249`
- `q` or `quit` or `exit`
  - Stop the interactive console, this will close the client

To stop the server, simply close the terminal or press `Ctrl + C`

Remember, to add or remove files for sharing. Simply delete or add them to your folder that you initially created for the client.

## Overall Example

This example will demonstrate following the procedures for a *simple* local case.

### Initial setup

Here I have layed everything out onto one screen, simply take note of what everything is. Note that your terminal may be separate windows instead of just one.

\includegraphics[width=\textwidth]{TestInital.png}

\newpage

Here we can also see the file contents for my test file.

\includegraphics[width=\textwidth]{TestFileCont0.png}

And before that we can see where the directories are relative to the program.

\includegraphics[width=\textwidth]{TestdirLoc.png}

And just as a tip, you can shift right click on Windows in the same directory as the program to more easily open a terminal.

\newpage

### Start Programs

Here we can see the commands are setup to be run.

\includegraphics[width=\textwidth]{TestCmds.png}

`.\Server.exe` to start the server

`.\Client.exe -i 0 -f 0` to start the client with id `0` and watching the folder named `0`

`.\Client.exe -i 1 -f 1` to start the client with id `1` and watching the folder named `1`

\includegraphics[width=\textwidth]{TestFirstRun.png}

After running them all, you might notice the prompt `Client >` becomes a bit garbled, simply pressing `enter` once should fix it

\includegraphics[width=\textwidth]{TextPrompt.png}

### Running a search

Here we are searching for the file we want, just looking for the letter `t` is enough. Note that we are using the client that is not watching the folder with the test file already in it.

\includegraphics[width=\textwidth]{TestSearch.png}

Make sure to copy the hash, in this case it would be `E2D0FE1585A63EC6009C8016FF8DDA8B17719A6`
`37405A4E23C0FF81339148249`

### Requesting a file

Now we need to actually request this file by running the following command on the same client.

`request E2D0FE1585A63EC6009C8016FF8DDA8B17719A637405A4E23C0FF81339148249`

After this runs, we can now see that the file is in the second directory.

\includegraphics[width=\textwidth]{TestDownload.png}

Just to make sure, we can see that the file has the same contents as it's copy.

\includegraphics[width=\textwidth]{TestSame.png}

### Closing

We now can close both clients with `q` and the server with `Ctrl+C`.

\includegraphics[width=\textwidth]{TestClosing.png}

Here everything has now closed and we are done.

\includegraphics[width=\textwidth]{TestClosed.png}
