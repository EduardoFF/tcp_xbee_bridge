#include <iostream>
#include <stdlib.h>
#include "xbee_interface.h"
#include <glog/logging.h>
#include <GetPot.hpp>
#include "tcpconn.h"
#include <signal.h>
using namespace std;

#ifndef IT
#define IT(c) __typeof((c).begin())
#endif

#ifndef FOREACH
#define FOREACH(i,c) for(__typeof((c).begin()) i=(c).begin();i!=(c).end();++i)
#endif


XbeeInterface *g_xbee;
char g_outBuf[120];
bool g_abort;

/// Mutex used to send one packet at a time
pthread_mutex_t g_sendMutex;
int g_nPacketsSent;
std::map<uint8_t, int> g_nPacketsRcv;
std::map<uint8_t, uint16_t>  g_lastSeqnRcv;

using namespace std;


TCPConn *g_tcpConn;

void
signalHandler( int signum )
{
    g_abort = true;
    printf("ending app...\n");
    LOG(INFO) << "Ending app";
    /// stop timers

    delete g_xbee;
    delete g_tcpConn;
    LOG(INFO) << "Clean exit";

    exit(signum);
}



void xbeeSend(uint8_t dst, size_t buflen)
{
    
  pthread_mutex_lock(&g_sendMutex);
  XbeeInterface::TxInfo txInfo;
  txInfo.reqAck = true;
  txInfo.readCCA = false;
  
  int retval = g_xbee->send(dst, txInfo, g_outBuf, buflen);
  if( retval == XbeeInterface::NO_ACK )
    {
      DLOG(INFO) << "send failed NOACK";
    }
  else if( retval == XbeeInterface::TX_MAC_BUSY )
    {
      DLOG(INFO) << "send failed MACBUSY";
    }
  else
    {
      DLOG(INFO) << "send OK";
      g_nPacketsSent++;
    }
  pthread_mutex_unlock(&g_sendMutex);
}

/// Function that receives the messages and handles them per type
void
receiveData(uint16_t addr,
	    void *data,
	    char rssi, timespec timestamp, size_t len)
{
  //   printf("got msg of len %u\n", len);
  g_tcpConn->send(static_cast<const char*>(data), len);
}
 

void
receiveTCP(void *data, size_t len)
{
  DLOG(INFO) << "Got TCP: len: " << len;
  memcpy(g_outBuf, data, len);
  DLOG(INFO) << "Forwarding to Xbee";
  xbeeSend(1, len);
}

/// Function to print Help if need be
void print_help(const string Application)
{
    exit(0);
}

/*--------------------------------------------------------------------
 * main()
 * Main function to set up ROS node.
 *------------------------------------------------------------------*/
int main(int argc, char **argv)
{

  /// register signal
  signal(SIGINT, signalHandler);


  /// Simple Command line parser
  GetPot   cl(argc, argv);
  if(cl.search(2, "--help", "-h") ) print_help(cl[0]);
  cl.init_multiple_occurrence();
  cl.enable_loop();
  const string  xbeeDev   = cl.follow("/dev/ttyUSB0", "--dev");
  const int     baudrate  = cl.follow(57600, "--baud");
  const uint8_t nodeId    = cl.follow(0, "--nodeid");
  const string  xbeeMode   = cl.follow("xbee1", "--mode");
  const string  ipaddr   = cl.follow("127.0.0.1", "--ip");
  const int     port  = cl.follow(12345, "--port");
  const string  logDir  = cl.follow("/tmp/", "--logdir");

  /// Initialize Log
  google::InitGoogleLogging(argv[0]);
  FLAGS_logbufsecs = 0;
  FLAGS_log_dir = logDir;
  LOG(INFO) << "Logging initialized in " << logDir;
    
  
  LOG(INFO) << "Logging initialized";

  
  g_abort = false;
  g_nPacketsSent=0;
    
  /// Xbee PARAMETERS
  XbeeInterfaceParam xbeePar;
  xbeePar.SourceAddress = nodeId;
  xbeePar.brate = baudrate;
  xbeePar.mode  = xbeeMode;
  xbeePar.Device = xbeeDev;
  xbeePar.writeParams = false;

  /// create mutexes
  if (pthread_mutex_init(&g_sendMutex, NULL) != 0)
    {
      fprintf(stderr, "mutex init failed\n");
      fflush(stderr);
      exit(-1);
    }

    
  g_xbee = new XbeeInterface(xbeePar);
  if( g_xbee->isOK() )
    {
      /// Listen for messages
      g_xbee->registerReceive(&receiveData);
    }
  else
    {
      fprintf(stderr, "xbee init failed\n");
      exit(-1);
    }
  

  g_tcpConn = new TCPConn(ipaddr, port, true);
  g_tcpConn->registerReceive(&receiveTCP);
			

  
  // Main loop.
  for(;;)
  {
    // Publish the message.
    //    node_example->publishMessage(&pub_message);

    //pthread_mutex_lock(&g_sendMutex);
    //    if( !g_xbee->checkAlive() )
    //printf("Xbee is not alive!!\n");
    //pthread_mutex_unlock(&g_sendMutex);
    sleep(5);
      
  }

  return 0;
} // end main()
