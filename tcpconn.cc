#include "tcpconn.h"
#include <glog/logging.h>
#include <sys/time.h>
#define IT(c) __typeof((c).begin())
#define FOREACH(i,c) for(__typeof((c).begin()) i=(c).begin();i!=(c).end();++i)

using namespace std;



TCPConn::TCPConn(string  ip,
		 int port,
		 bool autorun):
  m_acceptor(NULL),
  m_stream(NULL),
  m_ip(ip),
  m_port(port)
{
  m_abort = false;
  pthread_mutex_init(&m_mutex, NULL);
  initSocket();
  if( autorun )
    run();
  
}


void
TCPConn::initSocket()
{
  m_acceptor = new TCPAcceptor(m_port, m_ip.c_str());  
}

bool
TCPConn::run()
{
    int status = pthread_create(&m_thread, NULL, internalThreadEntryFunc, this);
    m_running=true;
    return (status == 0);
}


/// returns time in milliseconds
uint64_t
TCPConn::getTime()
{
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);

    uint64_t ms1 = (uint64_t) timestamp.tv_sec;
    ms1*=1000;

    uint64_t ms2 = (uint64_t) timestamp.tv_usec;
    ms2/=1000;

    return (ms1+ms2);
}

std::string
TCPConn::getTimeStr()
{
#ifndef FOOTBOT_LQL_SIM
    char buffer [80];
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    char currentTime[84] = "";
    sprintf(currentTime, "%s:%d", buffer, milli);
    std::string ctime_str(currentTime);
    return ctime_str;
#else
    return "mytime";
#endif
}

void
TCPConn::internalThreadEntry()
{

  if (m_acceptor->start() == 0)
    {
      while (!m_abort)
	{
	  DLOG(INFO) << "calling accept";
	  TCPStream *_stream = m_acceptor->accept();
	  DLOG(INFO) << "accept returned";
	  DLOG(INFO) << "trying to lock for accept";
	  pthread_mutex_lock(&m_mutex);
	  DLOG(INFO) << "lock for accept";
	  m_stream = _stream;
	  pthread_mutex_unlock(&m_mutex);
	  DLOG(INFO) << "unlock for accept";
	  if (m_stream != NULL)
	    {
	      DLOG(INFO) << "stream not null";
	      ssize_t len;
	      char line[95];
	      for(;;)
		{
		  // DLOG(INFO) << "trying to lock (receive)";
		  //pthread_mutex_lock(&m_mutex);
		  //DLOG(INFO) << "lock receive";
		  len = m_stream->receive(line, sizeof(line)-1);
		  //pthread_mutex_unlock(&m_mutex);
		  //DLOG(INFO) << "unlock receive";
		  if(len > 0)
		    {
		      line[len] = 0;
		      DLOG(INFO) << "TCP Received " << len;
		      //printf("received - %s\n", line);
		      if( m_receiveCB )
			{
			  m_receiveCB(line,len);
			}
		    }
		  else if( len <= 0)
		    break;
		}
	      DLOG(INFO) << "receive <= 0";
	      DLOG(INFO) << "trying to lock for delete";
	      pthread_mutex_lock(&m_mutex);
	      DLOG(INFO) << "lock for delete";
	      delete m_stream;
	      m_stream = NULL;
	      pthread_mutex_unlock(&m_mutex);
	      DLOG(INFO) << "unlock for delete";
	    }
	  else
	    {
	      DLOG(INFO) << "stream NULL";
	    }
	}
    }
}

void
TCPConn::registerReceive(TCPConn::ReceiveCB cb)
{
  m_receiveCB = cb;
}

void
TCPConn::send(const char *data, size_t len)
{
  DLOG(INFO) << "trying to lock (send)";
  pthread_mutex_lock(&m_mutex);
  DLOG(INFO) << "lock (send)";
  if( m_stream != NULL )
    {
      m_stream->send(data, len);
    }
  DLOG(INFO) << "unlock (send)";
  pthread_mutex_unlock(&m_mutex);
}

TCPConn::~TCPConn()
{
  if( isRunning() )
    {
      stop();
      pthread_join(m_thread,NULL);
    }
  if( m_acceptor )
    delete m_acceptor;
  if( m_stream )
    delete m_stream;

}
