
package com.ntkn

data_in = ('\0' * 256) as byte[]
data_out = "Tell me the time".getBytes("ASCII")
addr = InetAddress.getByName("localhost")
port = 5000
packet_out = new DatagramPacket(data_out, data_out.length, addr, port)
packet_in = new DatagramPacket(data_in, data_in.length)
socket = new DatagramSocket()
socket.send(packet_out)

socket.receive(packet_in)
println new String(packet_in.getData())