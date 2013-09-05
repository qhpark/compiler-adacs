#include <iostream>
#include "ringbuffer.hpp"
#define REMOVE_DEBUG
#include "debug.hpp"

using namespace std;
static const int default_buffer = 2048;
RingBuffer::RingBuffer()
{
	buffer = new char[default_buffer];
	buf = new char[default_buffer + 1];
	size = default_buffer;
	stored = 0;
	free = size;

	start_pos = 0;
	read = 0;
	stored = 0;
}

RingBuffer::RingBuffer(int bytes)
{
	buffer = new char[bytes];
	buf = new char[bytes + 1];
	size = bytes; 
	stored = 0;
	free = size;

	start_pos = 0;
	read = 0;
	stored = 0;
}

RingBuffer::~RingBuffer()
{
	delete [] buffer;
	delete [] buf;
}
int RingBuffer::num_stored()
{
	return stored;
}

int RingBuffer::num_unread()
{
	return stored - read;
}

int RingBuffer::num_free()
{
	return free;
}


char* RingBuffer::flush_str(int bytes)
{
	if (debugOpt.inDebug("ring"))
		cout << "static buff addy :" << (void *) buf << endl;
	if (bytes > stored) 
		return NULL;
	int splice = start_pos + bytes > size ? 
			     (bytes + start_pos) % size : 0; 
			     //bytes - (size - start_pos) : 0; 

	if (splice) {
		// all the way to the end of the buffer
		memcpy(buf, &buffer[start_pos], size - start_pos);
		// and the rest from the beginning 
		memcpy(&buf[size - start_pos], buffer, splice);
	} else {
		memcpy(buf, &buffer[start_pos], bytes);
	}

	buf[bytes] = '\0';
	// update pointers
	start_pos = (start_pos + bytes) % size;
	free += bytes;
	stored -= bytes;
	read = 0;

	
	if (debugOpt.inDebug("ring2"))
		cout << "flush ["<<buf<< "] free: " 
				<< free <<" stored: "<< stored << " bytes: "
				<< bytes<< " start_pos: " << start_pos 
				<< " read: " << read<< endl;
	return buf;
}

int RingBuffer::add_char(char a)
{
	if (!free) {
		if (debugOpt.inDebug("ring"))
			cerr << "buffer full" << endl;
		return -1;
	}
	buffer[(start_pos + stored) % size] = a;
	if (debugOpt.inDebug("ring"))
		cout << "add_char: added ("<< a<< ") at position ("
				<< (start_pos + stored) % size << ")" << endl;
	stored += 1;
	free -= 1;

}
int RingBuffer::get_char(char &c)
{
	if (read == stored) {
		if (debugOpt.inDebug("ring"))
			cerr << "no more data to read" << endl;
		return -1;
	}
	c = buffer[(start_pos + read) % size];	
	if (debugOpt.inDebug("ring"))
		cout << "read ("<< c << ") at "<< 
				(start_pos + read) % size << endl;
	read += 1;
	return read;
}



