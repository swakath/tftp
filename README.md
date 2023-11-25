# tftp
Trivial File Transport Protocol Implementation as per RFC 1350. This project was initiated as part of [Software Systems Lab](https://www.cse.iitd.ac.in/~srsarangi/courses/2023/cop_701_2023/index.html) at IIT Delhi during my MTech program.

## Dependencies
- C++ (g++ compiler 7.5.0 or above)
- [GTest](https://github.com/google/googletest), used for unit testing.
- CMake
- [easyloggingpp](https://github.com/abumq/easyloggingpp) for generating logs.


## Building and Usage
~~~
# Create a build directory inside tftp dir
mkdir build
cd build
cmake ..

# Server Usage
./tftpServer <SERVER_IP>

# Client Usage
./tftpClient <TFTP_OPERATION> <FILE_NAME> <SERVER_IP>
~~~

## Summary
The repositry contains the source code for TFTP Server and client as per [RFC1350](https://datatracker.ietf.org/doc/html/rfc1350). The implementating only works in "octet" mode specified in the RFC, "netascii" mode is not supported in the current implementation. Server operates in default TFTP port 69. Due to this, running the server may require root prelivages. 

### File Delete Operation
A custom DELETE operation is included appart from the five default operations mentioned in the RFC. A client can initiate a DELETE operation to delete a file in the server. DELETE uses an 2 byte opcode of value 06. The DELETE packet format similar to RRQ/WRQ as specified below.

    Opcode   string   1 byte    string   1 byte
    ----------------------------------------------
    | 06 |  Filename |   0  |    Mode    |   0  |
    ----------------------------------------------

All the other packet size specification mentioned in RFC1350 apply for DELETE packet. 

If the server receives a DELETE packet from a client, it checks the availability and privilage of the file requested to deleted. If the availability and privilages are satisfied, it deletes the specified file from the server directory and acknowledges the client by send a ACK packet with block Number 1. If any other condition occurs, it is considered as an error. In such conditions a corresponding ERROR packet is sent to the client and the server terminates the DELETE operation. 

DELETE operation is a connection initiation operation like RRQ and WRQ operations.

### Debug Logs
As mentioned above, the implementation uses [easyloggingpp](https://github.com/abumq/easyloggingpp) for generating logs. To enable Debug level logs, set the macro DEBUG to 1 , else set it to 0. To enable print of the logs to standard output set the macro TOSTDOUT to 1, else set it to 0. Disable debug while normal usage.

### Unit testing
As mentioned above, the implementation uses [GTest](https://github.com/google/googletest) for unit testing. Test are organized into suites, each test suit is contained in a seperate CPP file. All the unit test CPP files are organized into gtest directory. Test execution is as follows
~~~
# Create a build directory inside gtest dir
mkdir build
cd build
cmake ..

# Execution of test
./unitTest
~~~

The test suit is very basic. **Contributions to developing the testsuit and mock sockets will be much appretiated**.

# Contributers
[Manish Kumar](https://github.com/MKumar-12)
