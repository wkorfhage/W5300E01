package com.ntkn

class Adapter {
	String time
	String name
	int counter
	int delta
	int rank
	
	static mapping = {
		version false
	}
	
    static constraints = {
		name()
		counter()
		delta()
		rank()
		time()
    }
}
