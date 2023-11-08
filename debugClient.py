import socket
import time
def makeAck(n):
    code = 4
    ret = b''
    ret = ret + code.to_bytes(2, "big")
    ret = ret + n.to_bytes(2,"big")
    return ret

# Specify the IP address and port to send data to
ip_address = "127.0.0.1"  # Replace with the target IP address
port = 69  # Replace with the target port number

# Create a UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Data to send
opcode = 1
filename = "file1.txt"  # Replace with the data you want to send
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
cnt = 1
while(1):
    data, client_address = udp_socket.recvfrom(1024) 
    print(client_address)
    print(data)
    sendB = makeAck(cnt)
    udp_socket.sendto(sendB, client_address)
    cnt = cnt+ 1
    if(len(data)<512):
        break
        
time.sleep(2)
udp_socket.close()