# net-web-server

To compile:
	make

To run:
	make run

To clean compiled files:
	make clean

To run client:
	./client (-I) hostname:port/filepath

	- If you choose to include the command line argument -I, only the head will be returned
	- If you choose to omit the -I, only the file contents will ben displayed
	- :port it optional, default is 80
	- Must include a file path
