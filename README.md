# network_lab_project
Editor for collaboration. 
Proactor pattern
Task requirement:
* server only in C/C++ (C socket);
* gui for client.

How to launch:
* before install python3-pyqt4
* launch make_bash file
* start server: ./server/server
* start client: python3 client/textedit.py

## Demo 
### Screenshot 1
![demo_screen](https://user-images.githubusercontent.com/32968460/34411660-63ba9440-ebd8-11e7-8215-6958b7eef73e.png)

### Screenshot 2
![demo_second](https://user-images.githubusercontent.com/32968460/34445288-1004c298-ecd3-11e7-8870-4c83416571fd.png)

### Screenshot 3
![demo_thrid](https://user-images.githubusercontent.com/32968460/34482638-8af9e4a4-efb9-11e7-8eb7-9f56e03d0914.png)

## TODO
* Develop own GUI interface (programming library).
* Change static char array to vector.
* Servicing signals to linuxfd, descriptors and poll (remove std::cout).
* (C socket) poll, reactor pattern or proactor pattern.
