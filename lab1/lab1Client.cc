#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <cstring>

#include <netdb.h>
#include <unistd.h>

#include "MyPacket.h"   // defined by us
#include "lab1Client.h" // some supporting functions.
#include "TicTacToe.h"

using namespace std;



int main(int argc, char *argv[]) {

	MyPacket	sendpacket;
	MyPacket	recvpacket;
	char 		typeName[typeNameLen];

    	int         bytesReceived;
    	int         bytesSent; 
	

	sockaddr_in tcpServerAddr;	
	char                 	*serverNameStr = 0;
	unsigned short int	tcpServerPort;
	int 			tcpConnectionFd;

	sockaddr_in udpServerAddr;
	//char        		*udpServerNameStr = 0;
    	unsigned short int 	udpServerPort;
    	int         		udpSocketFd;
	
	stringstream ss;
	char assignment[16];
	

    // prase the argvs, obtain server_name and tcpServerPort
	parseArgv(argc, argv, &serverNameStr, tcpServerPort);

	struct hostent *hostEnd = gethostbyname(serverNameStr);

	std::cout << "[TCP] Tic Tac Toe client started..." << std::endl;
	std::cout << "[TCP] Connecting to server: " << serverNameStr << ":" << tcpServerPort << std::endl;

        tcpConnectionFd = socket(AF_INET, SOCK_STREAM, 0);

       	if(tcpConnectionFd < 0) {
       		cerr << "[ERR] Unable to create TCP socket." << endl;
       		exit(1);
       	}


	memset(&tcpServerAddr, 0, sizeof(tcpServerAddr));
	
	tcpServerAddr.sin_family = AF_INET;
        tcpServerAddr.sin_port   = htons(tcpServerPort);

	memcpy(&tcpServerAddr.sin_addr, hostEnd -> h_addr, hostEnd -> h_length);
	
	if(connect(tcpConnectionFd, (sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) < 0) {
            cerr << "[ERR] Unable to connect to server." << endl;
            if(close(tcpConnectionFd) < 0) {
                cerr << "[ERR] Error when closing TCP socket" << endl;
            }
            exit(1);
        }

	
	//Sends join packet

	sendpacket.type = JOIN;	
		
	bytesSent = send(tcpConnectionFd, &sendpacket, sizeof(sendpacket),0);
	
	if(bytesSent < 0) {
        	cerr << "[ERR] Error sending message to server." << endl;
        	exit(1);
        } 
	else {
		getTypeName(sendpacket.type, typeName);
	        cout << "[TCP] Sent: " << typeName << endl;
        }


	
	
	//Recieved JOIN_GRANT () packet
	bytesReceived = recv(tcpConnectionFd, &recvpacket, sizeof(recvpacket), 0);
	
	assignment[0] = recvpacket.buffer[0];	
	
	if(bytesReceived < 0) {
        	cerr << "[ERR] Error receiving message from server." << endl;
        	exit(1);
        } 
	else {
		getTypeName(recvpacket.type, typeName);		
		cout << "[TCP] Rcvd: " << typeName << " " << assignment[0] << endl;
        }
	
	

	//Sends GET_UDP_PORT packet	
	sendpacket.type = GET_UDP_PORT;
	bytesSent = send(tcpConnectionFd, &sendpacket, sizeof(sendpacket),0);	
	if(bytesSent < 0) {
        	cerr << "[ERR] Error sending message to server." << endl;
        	exit(1);
        } 
	else {
		getTypeName(sendpacket.type, typeName);
	        cout << "[TCP] Sent: " << typeName << endl;
	}

	
	//Recieves UDP_PORT PACKET	
	bytesReceived = recv(tcpConnectionFd, &recvpacket, sizeof(recvpacket), 0);
	if(bytesReceived < 0) {
        	cerr << "[ERR] Error receiving message from server." << endl;
        	exit(1);
        } 
	else {
		getTypeName(recvpacket.type, typeName);		
		cout << "[TCP] Rcvd: " << typeName << " " << " " << recvpacket.buffer << endl;
	}
	
	
	//Initialize UDP socket
	ss << recvpacket.buffer;
	ss >> udpServerPort;
	//hostEnd = gethostbyname(serverNameStr);
	
	udpSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(udpSocketFd < 0) {
        	std::cerr << "[ERR] Unable to create UDP socket." << std::endl;
        	exit(1);
    	}
	
	memset(&udpServerAddr, 0, sizeof(udpServerAddr));
	
	udpServerAddr.sin_family = AF_INET;
    	udpServerAddr.sin_port   = htons(udpServerPort);

    	memcpy(&udpServerAddr.sin_addr, hostEnd -> h_addr, hostEnd -> h_length);
	
	if(connect(udpSocketFd, (sockaddr *)&udpServerAddr, sizeof(udpServerAddr)) < 0) {
            cerr << "[ERR] Unable to connect to server." << endl;
            if(close(udpSocketFd) < 0) {
                cerr << "[ERR] Error when closing TCP socket" << endl;
            }
            exit(1);
        }



	//Sends GET_BOARD packet
	memset(&sendpacket, 0, sizeof(sendpacket));
	sendpacket.type = GET_BOARD;
	sendpacket.buffer[0] = assignment[0];
	bytesSent = sendto(udpSocketFd, &sendpacket, sizeof(sendpacket), 0, (sockaddr *) &udpServerAddr, sizeof(udpServerAddr));
	
	if(bytesSent < 0) {
        	std::cerr << "[ERR] Error sending message to server." << std::endl;
        	exit(1);
        } 
	else {	
		getTypeName(sendpacket.type, typeName);
        	std::cout << "[UDP] Sent: " << typeName << std::endl;
        }


	while(1){
		
	//Recieve YOUR_TURN packet
		memset(&recvpacket, 0, sizeof(recvpacket));
		
		bytesReceived = recv(udpSocketFd, &recvpacket, sizeof(recvpacket), 0);
		
		if(bytesReceived < 0) {
        		std::cerr << "[ERR] Error receiving message from server."<< std::endl;
            		exit(1);
        	} 
		else {
			getTypeName(recvpacket.type, typeName);
            		std::cout << "[UDP] Rcvd: " << typeName << " " << recvpacket.buffer << std::endl;
		}
		
	
		TicTacToe Game (recvpacket.buffer);
		Game.printBoard();
		
		if (recvpacket.type == YOUR_TURN){
			memset(&sendpacket, 0, sizeof(sendpacket));
			sendpacket.type = PLAYER_MARK;
			
			char test;				
			getCommand(sendpacket, Game, test);
			
			bytesSent = sendto(udpSocketFd, &sendpacket, sizeof(sendpacket), 0, (sockaddr *) &udpServerAddr, sizeof(udpServerAddr));	
		
		
			if(bytesSent < 0) {
        			std::cerr << "[ERR] Error sending message to server." << std::endl;
        			exit(1);
        		} 
			else {	
				getTypeName(sendpacket.type, typeName);
        			std::cout << "[UDP] Sent: " << typeName << std::endl;
        		}		
			
	
		}
		
		
		if (recvpacket.type == UPDATE_BOARD){
		
			memset(&sendpacket, 0, sizeof(sendpacket));
			sendpacket.type = GET_BOARD;
			sendpacket.buffer[0] = assignment[0];
			bytesSent = sendto(udpSocketFd, &sendpacket, sizeof(sendpacket), 0, (sockaddr *) &udpServerAddr, sizeof(udpServerAddr));
	
			if(bytesSent < 0) {
        			std::cerr << "[ERR] Error sending message to server." << std::endl;
        			exit(1);
        		} 
			else {	
				getTypeName(sendpacket.type, typeName);
        			std::cout << "[UDP] Sent: " << typeName << std::endl;
        		}
			
		
			
		}


		if ( (recvpacket.type == TIE) || (recvpacket.type == YOU_LOSE) || (recvpacket.type == YOU_WON)  ){
			
			cout << "GAME OVER" << endl;

			
			break;		
			
			
		}



		if ( recvpacket.type == EXIT_GRANT ){
			
			break;
		
		}
		
			
	}
		
	if(close(udpSocketFd) < 0) {
        	std::cerr << "[ERR] Error when closing connection socket."<< std::endl;
        	exit(1);
	}
	if(close(tcpConnectionFd) < 0) {
		std::cerr << "[ERR] Error when closing connection socket."<< std::endl;
        	exit(1);	
	}	
		
	

	return 0;	


}


