
#ifndef STREAMTRANSMISSION_HPP
#define STREAMTRANSMISSION_HPP

#include "framework.hpp"
/* 
Sending bytes from buffer to a socket. Sending stops until exceed send limit.
### Parameters
- `fd`: File descriptor of the destination's socket.
- `buf`: Data that need to be send.
- `len`: Send limit.
### Returns
Return -1 if sending failed, otherwise return number of bytes sent.
*/
ssize_t send_all(int fd, char* buf, size_t len);

/* 
Read bytes from socket and store it in a buffer. Reading stops until receive `'\n'` or exceed read limit.
### Parameters
- `fd`: File descriptor of the source's socket.
- `buf`: Buffer to store data read from the source.
- `len`: Read limit.
### Returns
Return -1 if reading failed, otherwise return number of bytes gained.
*/
ssize_t recv_line(int fd, char* buf, size_t len);

#endif // STREAMTRANSMISSION_HPP

