# UDP-WebSocket
This project is the implemetation of the JSONSocket.v1 protocol, with the usage of UDP, on Linux operating system.

## Table of Contents

## General Information
The main goal of JSONSocket is to be deployed as the streaming transport protocol in application-layer service meshes, where the JSONSocket headers attached to each stream can be used to communicate rich connection metadata/context across micro-services. This makes it possible to move out-of-band control channels to in-band signaling using JSONSocket, even when the underlying transport layer does not provide such a feature (e.g., UDP or UNIX domain sockets). In the case of UDP-WebSocket I used UDP on Linux operating system for the implementation of the protocol.

## Technologies Used
 - C programming language
 - Socket programming
 - JSON-C library

## Features
 - Establish connection between server and client after the exchange of JSON request and response headers
 - Join multiple clients to the server t the same time
 - Build the library with the provided Makefile
 - Try out the demo chat program provided, to test the usability of the project

## Setup
If you do not have JSON-C installed, you should begin with doing so, using the following command:
```
sudo apt install libjson-c-dev
```
After you've installed it, you can generate the library with the provided Makefile using a simple ```make``` command in the project's root folder.

## Usage
You can start the server side of the projects simply by typing ```./Server/server``` in your console. Similarly you can start the client side by typing in ```./Client/client```.

## Project Status
The projects fully implemented the JSONSocket.v1 protocol for UDP, however as the specification progresses, so can the project.

## Room for Improvement
 - The project is not exactly optimized and could use some polishing.
 - It only implements the specification with the usage of UDP, however in the future it should work on all transport protocols.

## Acknowledgements
I would like to thank my consultant, Dr. Gábor Rétvári, for providing me with the opportunity to implement this project based on his specification.
