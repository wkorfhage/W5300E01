
package com.ntkn
import groovy.sql.Sql;
import com.mysql.jdbc.jdbc2.optional.MysqlConnectionPoolDataSource;

def log = GroovyLog.netInstance(Server.class)

def dataSource = new MysqlConnectionPoolDataSource();
dataSource.setUser("lqu")
dataSource.setPassword("lqu")
dataSource.setURL("jdbc:mysql://localhost/test")

//String createDB = "CREATE TABLE RECORD (id BIGINT NOT NULL auto_increment, counter INT NOT NULL, delta INT NOT NULL, name VARCHAR(255) NOT NULL, rank INT NOT NULL, time VARCHAR(255) NOT NULL, PRIMARY KEY (id)) ENGINE=MyISAM;";

//sql.eachRow("select * from RECORD order by delta", { println it } );

DatagramSocket socket = new DatagramSocket(5000)
buffer = ('\0' * 1024) as byte[]

while(true) {
	DatagramPacket incoming = new DatagramPacket(buffer, buffer.length)
	socket.receive(incoming)

	
	String s = new String(incoming.data, 0, incoming.length)

	if (s.startsWith("Tell me the time")) {
		println new Date()
		byte[] b = new java.sql.Timestamp(new java.util.Date().getTime()).toString().getBytes()
		DatagramPacket outgoing = new DatagramPacket(b, b.length, incoming.getAddress(), incoming.getPort())
		socket.send outgoing
	} else {
		s = s.replaceAll("\n", "")
		s = s.replaceAll("\000", "")
		String reply = "Client said: '$s'"
		println reply
		sa = s.split(",")
		println sa
		
		//def sql = Sql.newInstance("jdbc:mysql://localhost/test", "lqu", "lqu", "com.mysql.jdbc.Driver");
		//sql.execute("insert into stat values ('2010-12-12 22:22:22.222', 'name', 12345, 0, 1)")
		
		def sql = new Sql(dataSource)
		sql.execute("insert into ADAPTER (id, time, name, counter, delta, rank) values (null, ${sa[0]}, ${sa[1]}, ${sa[2]}, ${sa[3]}, ${sa[4]})")
		sql.close()
	}
}