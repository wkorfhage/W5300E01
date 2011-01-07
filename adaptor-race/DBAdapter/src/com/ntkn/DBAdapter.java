/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.ntkn;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.Timestamp;

/**
 *
 * @author lqu
 */
public class DBAdapter {

    static private String createDB = "CREATE table stat ("
            + "time timestamp,"
            + "name VARCHAR(30),"
            + "counter integer,"
            + "delta integer,"
            + "rank integer)";

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws Exception {
        Class.forName("org.apache.derby.jdbc.EmbeddedDriver");
        System.out.println("Driver OK");

        Connection conn = null;
        String strUrl = "jdbc:derby:stat;create=true";

        conn = DriverManager.getConnection(strUrl);

        Statement stmt = conn.createStatement();
        stmt.execute("drop table stat");
        stmt.execute(createDB);
        stmt.execute("insert into stat values ('2010-12-6 12:01:37.333', '123', 1234, 0, 1)");
        stmt.execute("insert into stat values ('2010-12-6 12:01:37.333', 'aaa', 1234, 3, 1)");
        stmt.execute("insert into stat values ('2010-12-6 12:01:37.333', 'bbc', 1234, 2, 1)");

        ResultSet rs = stmt.executeQuery("select * from stat order by delta");

        while (rs.next()) {
            Timestamp time = rs.getTimestamp("time");
            String name = rs.getString("name");
            int counter = rs.getInt("counter");
            int delta = rs.getInt("delta");
            int rank = rs.getInt("rank");
            System.out.println(time + "\t" + name + "\t" + counter + "\t" + delta + "\t" + rank);
        }
    }
}
