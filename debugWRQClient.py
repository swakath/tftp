import socket
import time

def madeDataPacket(dataB, blockNum):
    opcode = 3
    ret = b''
    ret = ret + opcode.to_bytes(2, "big")
    ret = ret + blockNum.to_bytes(2, "big")
    ret = ret + dataB
    return ret

def checkACK(buff, blockNum):
    opcode = int.from_bytes(buff[0:2], "big")
    if(opcode != 4):
        return False
    recvBlock = int.from_bytes(buff[2:4], "big")
    if(recvBlock != blockNum):
        return False
    return True
    
# Specify the IP address and port to send data to
ip_address = "127.0.0.1"  # Replace with the target IP address
port = 69  # Replace with the target port number

# Create a UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Data to send
opcode = 2
filename = "file5.txt"  # Replace with the data you want to send
mode = "octet"
# Convert the data to bytes
data_bytes = b''
data_bytes = data_bytes + opcode.to_bytes(2, "big")
data_bytes = data_bytes + filename.encode('utf-8')
data_bytes = data_bytes + b'\x00'
data_bytes = data_bytes + mode.encode('utf-8')
data_bytes = data_bytes + b'\x00'
print(data_bytes)
print(len(data_bytes))

# Send the data to the specified IP and port
udp_socket.sendto(data_bytes, (ip_address, port))
cnt = 0

file_path = "./CMakeLists.txt"
with open(file_path, 'rb') as file:
    while (True):
        ack, client_address = udp_socket.recvfrom(1024)
        print(ack, client_address)
        if(not checkACK(ack, cnt)):
            break
        # Read 512 bytes from the file
        data_chunk = file.read(512)
        print(data_chunk)
        sendB = madeDataPacket(data_chunk, cnt+1)
        udp_socket.sendto(sendB, client_address)
        cnt = cnt +1
        if not data_chunk:
            break

time.sleep(2)
udp_socket.close()