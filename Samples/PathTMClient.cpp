//{\footnotesize\begin{verbatim}
// -----------------------------------------------------------------------------
//  Columbus Control Center
// 
//  Data Services Subsystem DaSS
// 
//  This software is developed by VCS AG under contract for DLR, 
//  contract no 6-057-2272.
//  
//  The copyright in this file is vested VCS Aktiengesellschaft. 
//	It may only be reproduced in whole or in part, or stored in a retrieval 
//	system, or transmitted in any form, or by any means electronic, 
//  mechanical, photocopying or otherwise, either with the prior permission 
//	of VCS AG or in accordance with the terms of DLR Contract no 6-057-2272.
//------------------------------------------------------------------------------
/**
 *  @file
 *  @version 1.4
 *  @author VCS AG
 *
 *  @b ID: /spacecom/cvs_dass/DaSS/cpp/apis/dass/client/examples/PathTMClient.cpp,v 1.3 2005/01/24 14:29:48 mh Exp
 */ 
  
#include "PathTMInterface.h"
//#include "ServiceStatusInterface.h"
//#include "SpecificationException.h"
//#include "PathTM.h"
//#include "ServiceStatusResponse.h"
#include "StatusService.h"
//#include "PathTMResponse.h"
#include "PathTMClientConnection.h"
//#include "PathTMService.h"
//#include "RealtimePathTMSpecification.h"
//#include "PlaybackPathTMSpecification.h"
//#include "DaSS_C_API.h"
#include "ConnectionMonitor.h"
//#include "TimedEvent.h"
//#include "DaSSException.h"

/** Global variables */
int gapid, gvid, gpt, gph = 0;

/** 
* A simple client for the DaSS PATH TM service that
* implements the call back functions for data reception
*/
class PathTMClient:public dass::client::PathTMInterface,
		public dass::client::ConnectionMonitor,
		public dass::client::ServiceStatusInterface,
		public Util::TimedEvent
{
private:
  dass::client::RealtimePathTMSpecification m_TMSpec;
  /** To be used for playback requests */
  //dass::client::PlaybackPathTMSpecification m_TMPlaybackSpec;
  dass::client::PathTMClientConnection * m_Connection;
  dass::client::PathTMService * m_Svc;
  dass::client::StatusService * m_status_service;
  std::string m_hostname;
  int m_port;

  int numberofrequests;

public:
  PathTMClient (std::string hostname, int port, int timeout, int mode);
  virtual ~PathTMClient ();

  dass::client::PathTMService * getService ()
  {
    return m_Svc;
  }

  dass::client::StatusService * getStatusService ()
  {
    return m_status_service;
  };

  dass::client::PathTMClientConnection * getConn ()
  {
    return m_Connection;
  }

  void createConnection ();
  virtual void receivePathTMData (dass::client::PathTMService & service, dass::PathTM * data);

  /** Called when service response is received */
  virtual void receivePathTMResponse (dass::client::PathTMService & service,
			 dass::PathTMResponse & response);

  virtual void Triggered ();

  /** Call back functions from the connection monitor */
  virtual void connected (dass::client::ClientConnection & c)
  {
    printf ("Client is connected...\n");
  }

  virtual void disconnected (dass::client::ClientConnection & c)
  {
    printf ("Client is disconnected...\n");
  }

  virtual void reconnecting (dass::client::ClientConnection & c)
  {
    printf ("Reconnecting...\n");
  }

  virtual void reconnected (dass::client::ClientConnection & c)
  {
    printf ("Reconnected...\n");
  }

  /** 
    * Called by the connection if it has re-established the transport connection
    * and recovered all the services
    */
  virtual void recovered (dass::client::ClientConnection & c)
  {
    printf ("Recovered...\n");
  }

  /** Called by the connection if it cannot re-establish the connection. All services are deleted */
  virtual void closed (dass::client::ClientConnection & c)
  {
    m_Svc = NULL;
    printf ("Closed...\n");
  }

  virtual void illegalPacket (dass::client::ClientConnection & c)
  {
    printf ("IllegalPacket...\n");
  }

  /** Called when ServiceStatus is started and response is received */
  virtual void receiveServiceStatus (dass::client::StatusService & service,
			dass::ServiceStatusResponse & response)
  {
    m_status_service = &service;
    std::cout << "\nReceived " << service.toString ().c_str () << std::endl;
  }

  /** Called after positive response is received and the data connection is established */
  virtual void serviceStarted (dass::client::Service & service)
  {
    std::cout << "\nStarted " << service.toString ().c_str () << std::endl;
  }

  /** Called by the service if it is being stopped */
  virtual void serviceStopped (dass::client::Service & service)
  {
    std::cout << "\nStopped " << service.toString ().c_str () << std::endl;
  }

  /** Called if the service encounters an error from the DaSS Kernel */
  virtual void serviceError (dass::client::Service & service,
				dass::client::ServiceError & error)
  {
    std::cout << "\nError on service " << service.toString ().c_str () << std::endl;
    std::cout << "Error message " << error.getErrorMessage ().c_str () << std::endl;
    std::cout << "Error code " << error.getErrorCode () << std::endl;
  }
};

PathTMClient::PathTMClient (std::string hostname, 
					int port, 
					int timeout, 
					int mode):
Util::TimedEvent (0), 
m_Connection (0), 
m_Svc (0), 
m_status_service (0),
m_hostname (hostname), 
m_port (port), 
numberofrequests(0)
{
  try
  {
    int Servicemode = dass::ServiceDescriptor::SERVICEMODE_REALTIME;
    /** Mission mode */
    int MissionMode = dass::ServiceDescriptor::MODE_TEST;

    m_TMSpec.setAPID (gapid);
    m_TMSpec.setVehicleID (gvid);
    m_TMSpec.setPacketType (gpt);
    m_TMSpec.setPrivateHeaderSource (gph);

    /* Create connection object */
    m_Connection = new dass::client::PathTMClientConnection (Servicemode, MissionMode);
    m_Connection->addConnectionMonitor (*this);
    /** After loosing the connection to the DaSS one retry will be performed after 30 seconds */
    m_Connection->setSessionRetrievalTime(30);

    if (mode == 2)
      m_Connection->setCredentials ("Kaspar.Hauser@vcs.de", "Hauser");
    if (mode == 1)
      m_Connection->setCredentials ("./usercert.pem");
    try
    {
      m_Connection->connect (hostname, port, timeout);
      if (m_Connection->connected ())
	{
	  /* Issue the requests by setting the events */
	  m_Svc = m_Connection->startPathTMService (*this, m_TMSpec);
	  Set (Util::CTimeSpan::Second () * 5);
	}
      else
	{
	  Clear ();
	  Set (Util::CTimeSpan::Second () * 10);
	}
    }
    catch (...)
    {
      Clear ();
      std::cout << "Exception while starting PT service!" << std::endl;
      Set (Util::CTimeSpan::Second () * 10);
    }
  }
  catch (dass::DaSSException & excp)
  {
    std::cout << "DaSSException:" << excp.getErrorMessage () << std::endl;
    throw;
  }
  catch (...)
  {
    /** Catch unknown exception */
    std::cout << "Unkown exception" << std::endl;
    throw;
  }
}

void PathTMClient::Triggered ()
{     
  if (m_Connection->connected ())
    m_status_service = m_Connection->startStatusService (*this);
  if (m_status_service)
    std::cout << "\nStarted Service Status..." << std::endl;
}

PathTMClient::~PathTMClient ()
{
  /** Stop the services if still active */
  if (m_Svc != NULL)
    m_Svc->stop ();
  if (m_Connection != NULL)
    delete m_Connection;
}

void PathTMClient::receivePathTMData (dass::client::PathTMService & service, dass::PathTM * data)
{
  try
  {
    int apid = data->getCCSDSPacket()->getAPID();
    int pt   = data->getCCSDSPacket()->getPacketType();
    int vid  = data->getCCSDSPacket()->getVehicleID();
    int phs  = data->getCCSDSPacket()->getPrivateHeaderSource();
    std::cout << "Received PathTMData - APID/PT/VID/PHS: " << std::dec << apid << "/" << pt << "/" << vid << "/" << phs << std::endl;
    
    const unsigned char *buf = data->getCCSDSPacket ()->getRawPacket ();
    int len = data->getCCSDSPacket ()->length ();
    std::cout << "  Packet length: " << std::hex << len << std::endl;
    for (int i = 0; i < len + 7; i++)
    {
      std::cout.width (2);
      std::cout << "  " << std::hex << (unsigned int) buf[i] << " ";
      if ((i & 15) == 15)
	std::cout << std::endl;
      }
    std::cout << std::endl;
    /** The user is not responsible for deleting the PathTM data packets received */
  }
  catch (dass::DaSSException & excp)
  {
    std::cout << excp.getErrorMessage () << std::endl;
    throw;
  }
  catch (...)
  {				
    /** Catch unknown exception */
    std::cout << "Unkown exception..." << std::endl;
    throw;
  }
}

void PathTMClient::receivePathTMResponse (dass::client::PathTMService & service,
				     dass::PathTMResponse & response)
{
  try
  {
    int ret = response.getResponseCode ();
    printf ("Received %s response, code: %d\n", service.toString ().c_str (),
	    ret);
    std::string msg = response.getResponseMessage (ret);
    std::cout << "Response message: " << msg << std::endl;
  }
  catch (dass::DaSSException & excp)
  {
    std::cout << excp.getErrorMessage () << std::endl;
    throw;
  }
  catch (...)
  {				// catch unknown exception
    std::cout << "Unkown exception..." << std::endl;
    throw;
  }
}

int main (int argc, char *argv[])
{
  if ((argc <= 10))
    {
      std::
	cout <<
	"\nError! The address, port, mode, apid, vid, pt and ph are not specified!"
	<<
	" Usage: -h <host> -p <port> -m <secureMode> ( 1 for secure port and 2 for insecure port) apid vid pt ph"
	<< std::endl;
      return 1;
    }
  try
  {
    int port = 0, mode = 0;
    /** Timeout to get connected */ 
    int timeout = 10;
    std::ostringstream hostname;

    if (argv[1] == std::string ("-p"))
      port = atoi (argv[2]);
    else if (argv[1] == std::string ("-h"))
      hostname << argv[2];
    else if (argv[1] == std::string ("-m"))
      mode = atoi (argv[2]);

    if (argv[3] == std::string ("-p"))
      port = atoi (argv[4]);
    else if (argv[3] == std::string ("-h"))
      hostname << argv[4];
    else if (argv[3] == std::string ("-m"))
      mode = atoi (argv[4]);

    if (argv[5] == std::string ("-p"))
      port = atoi (argv[6]);
    else if (argv[5] == std::string ("-h"))
      hostname << argv[6];
    else if (argv[5] == std::string ("-m"))
      mode = atoi (argv[6]);

    gapid = atoi (argv[7]);
    gvid = atoi (argv[8]);
    gpt = atoi (argv[9]);
    gph = atoi (argv[10]);

    PathTMClient *pclient = new PathTMClient (hostname.str (), port, timeout, mode);

    dass::client::PathTMService * srv = pclient->getService ();
    dass::client::ClientConnection::StartService (120);
    dass::client::PathTMClientConnection * conn = pclient->getConn ();

    if (srv != NULL)
      {
	srv->stop ();
	std::cout << "\nStopping request!" << std::endl;
      }
    /** Stop here for 5 seconds */
    dass::client::ClientConnection::StartService (5);

    if (pclient->getStatusService () != NULL)
      {
	pclient->getStatusService ()->stop ();
	std::cout << "\nStopping status request!" << std::endl;
      }
    /** Stop here for 5 seconds */
    dass::client::ClientConnection::StartService (5);

    if (conn != NULL && conn->connected ())
      {
	conn->disconnect ();
	std::cout << "\nClosing connection!" << std::endl;
      }
    /** Stop here for 5 seconds */
    dass::client::ClientConnection::StartService (5);

    if (pclient != NULL)
      delete pclient;

    return 0;
  }
  catch (dass::DaSSException & exp)
  {
    std::cout << exp.getErrorMessage () << std::endl;
  }
  catch (...)
  {
    /** Catch unknown exception */
    std::cout << "Unkown exception..." << std::endl;
  }
}
//\end{verbatim}}
