# Image Resizer Server

# Overview

This program is a simple `c++` web server that resize image to desired dimension.

## Dependencies:

1. [Boost 1.75.0](https://www.boost.org/users/history/version_1_75_0.html) : Web service framework
2. [Base64](https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/) : Image encoding
3. OpenCV2 : Image resizing

1 and 2 available on `include` folder.

## Installation

```bash
cd build
cmake ..
make
```

## Usage

Test image resizer : 

```bash
# ./resizer_test <image_directory> <new_width> <new_height>
./resizer_test ../testdata/gundamcat.png 100 100
```

Server :

```bash
# ./server <IP address> <port>
./server 0.0.0.0 8080
```

Client :

```bash
# ./client <image_directory> <new_width> <new_height> <IP address> <port> <endpoint> 
./client ../testdata/gundamcat.png 100 100 0.0.0.0 8080 /resize_image
```

## Checklists

- [x]  Cmake
- [x]  Resize image
- [x]  Encode image
- [x]  Working web service
- [x]  Send POST
- [x]  Receive POST
- [ ]  Read body from POST