/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ntkn

//format = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S")
//date = format.format(new java.util.Date()).toString()
date = new java.sql.Timestamp(new java.util.Date().getTime())
randomName = "adaptor-" + (new java.util.Random().nextInt() % 5 + 5);
randomCounter = new java.util.Random().nextInt()%500 + 1000
data = "${date}, ${randomName}, ${randomCounter}, 0, 1".getBytes("ASCII")
addr = InetAddress.getByName("localhost")
port = 5000
packet = new DatagramPacket(data, data.length, addr, port)
socket = new DatagramSocket()
socket.send(packet)

socket.receive(packet)
println new String(packet.getData())