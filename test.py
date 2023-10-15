import socket

# Define the server's IP address and port
server_ip = '127.0.13.1'  # Replace with the server's IP address
server_port = 389  # Replace with the server's port

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the server
client_socket.connect((server_ip, server_port))

# Binary data to send
hex_data = "30 84 00 00 00 0F 02 83 00 00 02 01 01 60 0A 02 01 03 04 03 31 32 33 80 00"

# Convert hexadecimal data to bytes
binary_data = bytes.fromhex(hex_data)

# Send the binary data
client_socket.send(binary_data)

# Close the connection
client_socket.close()
