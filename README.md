HOW TO BUILD:
- Ensure that the DLLs inside of $(SolutionDir)Extern\DDLs are placed in the correct locations (x64\Configuration\)
- Ensure that the following libs are inside of $(SolutionDir)Extern\lib\Configuration\ folders:
	- cryptlib.lib (https://cryptopp.com/)
	- libcrypto.lib (https://dev.mysql.com/downloads/connector/cpp/)
	- mysqlcppconn.lib (https://dev.mysql.com/downloads/connector/cpp/)
	- mysqlcppconn8.lib (https://dev.mysql.com/downloads/connector/cpp/)
	- NetworkUtils.lib (Built in this project)
	- libprotobuf.lib (https://github.com/protocolbuffers/protobuf)
	
EXECUTION:
- Ensure that the projects are executed in the following order:
	- AuthServer
	- Server
	- Client
	
Demo Video: https://www.youtube.com/watch?v=mtrzBen490M

Github: https://github.com/Kihz17/Networking-Project2
