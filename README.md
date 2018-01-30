# network_lab_project
Editor for collaboration. 

## Task requirement:
* server only in C/C++ (C socket);
* gui for client.

## History 

### Release 0.03.x

#### How to launch:

### Release 0.01 and Release 0.02.x.x

#### How to launch:
* Before install python3-pyqt4 (Release 0.01 and Release 0.02.x.x).
1. Launch make to build.
2. Start server.
3. Start client.

#### Screenshot 1
![demo_screen](https://user-images.githubusercontent.com/32968460/34411660-63ba9440-ebd8-11e7-8215-6958b7eef73e.png)

#### Screenshot 2
![demo_second](https://user-images.githubusercontent.com/32968460/34445288-1004c298-ecd3-11e7-8870-4c83416571fd.png)

#### Screenshot 3
![demo_thrid](https://user-images.githubusercontent.com/32968460/34482638-8af9e4a4-efb9-11e7-8eb7-9f56e03d0914.png)

## TODO
* Servicing signals to linuxfd, descriptors and poll. (In signals use write and read with STDOUT_FILENO.)
* Poll - use reactor pattern or proactor pattern.
