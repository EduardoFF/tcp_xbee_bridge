#ifndef _TCPCONN_H_
#define _TCPCONN_H_
// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * gps_driver.{cc,hh} -- interface to GPS data
 *
 * Author:		        Eduardo Feo Flushing
                    Dalle Molle Institute for Artificial Intelligence
                IDSIA - Manno - Lugano - Switzerland
                (eduardo <at> idsia.ch)

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <set>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>


#include "tcpacceptor.h"
#include "tcpstream.h"


class TCPConn 
{
public:
  /// constructor for using gpsd client
  TCPConn(string ip, int port, bool autorun);
  void send(const char *, size_t);
  bool run();
  ~TCPConn();
  typedef void (*ReceiveCB)(void *data,
			    size_t len);
  void registerReceive(ReceiveCB);
      
  
private:
    static uint64_t getTime();
    static std::string getTimeStr();
    TCPStream* m_stream;
    TCPAcceptor* m_acceptor;
    string m_ip;
    int m_port;
    bool m_abort;
    bool m_running;
    ReceiveCB m_receiveCB;
    pthread_mutex_t m_mutex; /** Mutex to control the access to member variables **/
    pthread_t m_thread;     /** Thread **/

    bool isRunning(){ return m_running;}
    void initSocket();
    bool stop(){ m_abort=true;}
    static void * internalThreadEntryFunc(void * ptr)
    {
        (( TCPConn *) ptr)->internalThreadEntry();
        return NULL;
    }
    void internalThreadEntry();
};

#endif






