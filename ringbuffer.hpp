#ifndef __RINGBUFFER_HPP_
#define __RINGBUFFER_HPP_
class RingBuffer {
	public:
		RingBuffer();
		// give size. 
		RingBuffer(int bytes);
		~RingBuffer();

		// get last position
		//int get_last_pos();		

		// return how much buffer is used. 
		int num_stored();
		// return how much more is left to read 
		int num_unread();
		// return how much more space is left 
		int num_free();

		// get c-style string, this updates start_position.
		char *flush_str(int end_position);

		// add a character at the end
		// returns the updates position
		int add_char(char c);

		// get a character from the start_pos
		// and advances every time it's called.
		// returns current position
		int get_char(char &c);

	private:
		char *buffer, *buf;
		int size, stored, free;
		int	start_pos, read; 
};
#endif 
