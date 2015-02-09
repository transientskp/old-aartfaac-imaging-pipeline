#!/usr/bin/python
import socket;
import sys;
import time;

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print 'Usage: sendtcp.py host:port tx_filename';

	_host = sys.argv[1].split(':')[0];
	_port = int (sys.argv[1].split(':')[1]); 
	print '--> Transmitting to host %s, port %d' % (_host, _port);
	_fid = socket.socket (socket.AF_INET, socket.SOCK_STREAM);
	# _fid.bind( (_host, _port) );
	try:
		_fid.connect( (_host,_port) );
	except:
		print '### Error trying to connect!'
	
	print 'Sending file ', sys.argv[2];
	with open (sys.argv[2]) as fp:
		for line in fp:
			_fid.send(line);
			time.sleep(0.1);

	print 'Done.';
			
