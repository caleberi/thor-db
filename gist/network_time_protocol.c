#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

void error( char* msg )
{
    perror( msg ); // Print the error message to stderr.

    exit( 0 ); // Quit the process.
}

int main( int argc, char* argv[ ] )
{
  int sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.

  int portno = 123; // NTP UDP port number.

  char* host_name = "us.pool.ntp.org"; // NTP server host-name.

  // Structure that defines the 48 byte NTP packet protocol.


typedef struct{
  // li -> represents leap indicator having 2-bit size
  // vn -> represents version number of the protocol using  3-bit size
  // node -> represent Client will pick mode 3 for client
  // diagram => [li_f,li_f,vn_f,vn_f,vn_f,node_f,node_f,node_f] 
  // where _f suffix represent fragrament of each component that is
  // equivalent to uint8_t in total
  uint8_t li_vn_node;
  
  // representation of stratum level of the local clock
  uint8_t stratum; 
  // representation of maximum interval between successive messages
  uint8_t poll;
  // representation of precision of the local clock
  uint8_t precision;
  // representation of total round trip delay time
  uint32_t rootDelay;
  // representation of max error aloud from primary clock source
  uint32_t rootDispersion;
  // representation of reference clock identifier
  uint32_t refId;
  // representation of reference time-stamp seconds
  uint32_t refTm_s;
  // representation of reference time-stamp fraction of a second
  uint32_t refTm_f;
  // representation of originate time-stamp seconds
  uint32_t origTm_s;
  // representation of originate time-stamp fraction of a second.
  uint32_t origTm_f;
  // representation of received time-stamp seconds.
  uint32_t rxTm_s;
  // representation of received time-stamp fraction of a second
  uint32_t rxTm_f;
  // the most important field the client cares about. Transmit time-stamp seconds
  uint32_t txTm_s;
  // transmit time-stamp fraction of a second
  uint32_t txTm_f;
  
} ntp_packet; // Total : 348 bits (addition of all individual bit)


// Create and zero out the packet. All 48 bytes worth. (initialize packet)

ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// set the value of each individual component of the packet

memset(&packet, 0,sizeof(ntp_packet));

// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

*( ( char * ) &packet + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

// Create a UDP socket, convert the host-name to an IP address, set the port number,
// connect to the server, send the packet, and then read in the return packet.

struct sockaddr_in serv_addr; // Server address data structure

struct hostent *server;      // Server data structure.

sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.

if ( sockfd < 0 )
  error( "ERROR opening socket" );

server = gethostbyname( host_name ); // Convert URL to IP.

if ( server == NULL )
  error( "ERROR, no such host" );

// Zero out the server address structure.

bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

serv_addr.sin_family = AF_INET;

// Copy the server's IP address to the server address structure.

bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

// Convert the port number integer to network big-endian style and save it to the server address structure.

serv_addr.sin_port = htons( portno );

// Call up the server using its IP address and port number.

if ( connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
  error( "ERROR connecting" );

// Send it the NTP packet it wants. If n == -1, it failed.

n = write( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

if ( n < 0 )
  error( "ERROR writing to socket" );

// Wait and receive the packet back from the server. If n == -1, it failed.

n = read( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

if ( n < 0 )
  error( "ERROR reading from socket" );

// These two fields contain the time-stamp seconds as the packet left the NTP server.
// The number of seconds correspond to the seconds passed since 1900.
// ntohl() converts the bit/byte order from the network's to host's "endianness".

packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.

// Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
// Subtract 70 years worth of seconds from the seconds since 1900.
// This leaves the seconds since the UNIX epoch of 1970.
// (1900)------------------(1970)**************************************(Time Packet Left the Server)

time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

// Print the time we got from the server, accounting for local timezone and conversion from UTC time.

printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );
                     

}