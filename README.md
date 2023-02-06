# Introduction 
In this project we will be creating efficient software. Legacy code has been provided but needs to be improved. Better usage of memory, cpu, and disk will be included in this scope as well as faster timing. The software itself is a client server architecture. The client reads teleletry data from a file and sends it to the server. The server averages those data values with all recieved and sends back an average.

# Getting Started

To get started you first need to install the software which can be found here % fill this in %

Server
     - Once downloaded extract the folder
     - The system should have at minimum a quad core cpu (non-virtualized) with a base clock of 2600MHz or more, 4GB of ram
     - Open the configuration.conf file and under the wan = x.x.x.x replace x.x.x.x with your wan IP and replace port = 27001 with the port you wish to use.
     - Run the server.exe
     
Client
     - Once downloaded extract the folder
     - The system should have at minimum a dual core cpu, 2GB of ram, HDD 7200RPM disk with at least 2GB of storage remaining.
     - Open the configuration.conf file and replace the wan = x.x.x.x with the server you would like to connect to and replace port = 27001 with the port you wish to use
     - Run the client.exe

# Build and Test
TODO: Describe and show how to build your code and run the tests. 

# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

If you want to learn more about creating good readme files then refer the following [guidelines](https://docs.microsoft.com/en-us/azure/devops/repos/git/create-a-readme?view=azure-devops). You can also seek inspiration from the below readme files:
- [ASP.NET Core](https://github.com/aspnet/Home)
- [Visual Studio Code](https://github.com/Microsoft/vscode)
- [Chakra Core](https://github.com/Microsoft/ChakraCore)
