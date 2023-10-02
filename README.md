# talk-DHKE
A chat app showing of what I learned in BWSI's Cyber Ops Selection Course.

## Usage
‼️ ***CURRENTLY ONLY WORKS ON WINDOWS*** ‼️

run the executable from the command line with an integer specifying port number to start a server.
```
talk.exe 800
```
run the executable from the command line with an ip address followed by a port number to connect to a server as a client.
```
talk.exe 127.0.0.1 800
```

once connected anyone can type
```
exit
```
to end the conversation and close the program.

## Structure

*Listener Mode:* Listen for any incoming connections
*Speaker Mode:* Connect to a listener and await begin messaging
