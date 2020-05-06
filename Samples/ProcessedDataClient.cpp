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
 *  @b ID: /spacecom/cvs_dass/DaSS/cpp/apis/dass/client/examples/ProcessedDataClient.cpp,v 1.3 2005/01/24 14:29:48 mh Exp
 */

/** Import statements */
#include <stdio.h>
#include "ProcessedDataInterface.h"
#include "ProcessedDataClientConnection.h"
#include "StatusService.h"
#include "ServiceStatusResponse.h"
#include "ConnectionMonitor.h"

/**
* A simple client for the DaSS ProcessedData service that
* implements the call back functions for data reception
*/
class ProcessedDataClient:public dass::client::ProcessedDataInterface,
			public dass::client::ConnectionMonitor,
			public dass::client::ServiceStatusInterface,
			public Util::TimedEvent
{
private:
  dass::client::RealtimeProcessedDataSpecification m_spec;
  dass::client::ProcessedDataClientConnection * m_conn;
  dass::client::ProcessedDataService * m_svc;
  dass::client::StatusService * m_status_service;

public:
  ProcessedDataClient (std::string hostname, int port, int timeout, int mode, std::string um);

  dass::client::ProcessedDataService * getService ()
  {
    return m_svc;
  }

  dass::client::StatusService * getStatusService ()
  {
    return m_status_service;
  }

  dass::client::ProcessedDataClientConnection * getConn ()
  {
    return m_conn;
  }

  virtual ~ ProcessedDataClient ();

  /** Issues the request(s) */
  void sendRequest (int);

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
    m_svc = NULL;
    printf ("Closed...\n");
  }

  virtual void illegalPacket (dass::client::ClientConnection & c)
  {
    printf ("IllegalPacket...\n");
  }

  /** Call back functions from Interface */
  virtual void
  receiveProcessedData (dass::client::ProcessedDataService & service, dass::ProcessedDataItemSet * data);

  /** Called when service response is received */
  virtual void
  receiveProcessedDataResponse (dass::client::ProcessedDataService & service,
				dass::ProcessedDataResponse & response);

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
    std::cout << "Message " << error.getErrorMessage ().c_str () << std::endl;
  }
};

ProcessedDataClient::ProcessedDataClient (std::string hostname,
					int port,
					int timeout,
					int mode,
					std::string um):
Util::TimedEvent (0),
m_conn (0),
m_svc (0),
m_status_service (0)
{
  try
  {
    int Servicemode = dass::ServiceDescriptor::SERVICEMODE_REALTIME;
    int MissionMode = dass::ServiceDescriptor::MODE_TEST;

    dass::UMISet u;
    u.addUMI (dass::UMI (um));

    m_spec = dass::client::RealtimeProcessedDataSpecification (1, u);

    /** Create connection object */
    m_conn = new dass::client::ProcessedDataClientConnection (Servicemode, MissionMode);
    m_conn->addConnectionMonitor (*this);
    /** After loosing the connection to the DaSS one retry will be performed after 30 seconds */
    m_conn->setSessionRetrievalTime(30);

    if (mode == 2)
      m_conn->setCredentials ("Kaspar.Hauser@vcs.de", "Hauser");
    else if (mode == 1)
      m_conn->setCredentials ("./usercert.pem");
    try
    {
      m_conn->connect (hostname, port, timeout);
      if (m_conn->connected ())
      {
					m_svc = m_conn->startProcessedDataService (*this, m_spec);
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
      std::cout << "Exception while starting PD service!" << std::endl;
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

void ProcessedDataClient::Triggered ()
{
  if (m_conn->connected ())
    m_status_service = m_conn->startStatusService (*this);
  if (m_status_service)
    std::cout << "\nStarted Service Status..." << std::endl;
}

ProcessedDataClient::~ProcessedDataClient ()
{
  /** Stop the services if still active */
  if (m_svc != NULL)
    m_svc->stop ();
  if (m_conn != NULL)
    delete m_conn;
}

void ProcessedDataClient::receiveProcessedData (dass::client::ProcessedDataService & service,
						dass::ProcessedDataItemSet * data)
{
  try
  {
    /** Print out the received data */
    int count = data->count ();
    for (int i = 0; i < count; i++)
      {
	dass::ProcessedDataItem * item = data->getItem (i);
	std::cout << "\nReceived ProcessedData" << std::endl;
	std::cout << " UMI: " << item->getUMI ().toString ().c_str ()
	  << "\n Generated time: " << item->getGeneratedTime ().toString ().
	  c_str () << "\n StatusBitfield: " << item->
	  getStatusBitfield () << "\n ValueType: " << item->
	  getDataType () << "\n UnitIndex: " << item->
	  getUnitsIndex () << "\n Valuestate: " << (int) (item->
							getValueState ()) <<
	  "\n Unit: " << item->getUnits ().c_str () << std::endl;

	std::cout << " Raw data content: ";
	const unsigned char *buf = item->getRawValue ();
	for (int unsigned i = 0; i < item->getRawValueLength (); i++)
	  {
	    std::cout.width (2);
	    std::cout << "  " << std::hex << (unsigned int) buf[i] << " ";
	    if ((i & 15) == 15)
	      std::cout << std::endl;
	  }

      std::cout << std::endl;
      // Print int values
      if(1 == item->getDataType()) {
            std::cout << "Int32 Value: " << std::dec << item->intValue() << std::endl;
      }


      }
      /** The user is responsible for deleting the data received */
      delete data;
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

void ProcessedDataClient::receiveProcessedDataResponse (dass::client::ProcessedDataService &service,
							dass::ProcessedDataResponse &response)
{
  try
  {
    int ret = response.getResponseCode ();
    std::cout << "\nReceived ProcessedDataResponse containing the ResponseCode: " << ret << std::endl;
    std::string msg = response.getResponseMessage (ret);
    /** Log response */
    std::cout << "Response message: " << msg << std::endl;

    if( ret != 0 )
    {
       m_svc = 0;

    }

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

int main (int argc, char *argv[])
{
  if ((argc <= 8))
    {
      std::cout <<"\nError! \nThe address or port of remote host, sercurity mode or UMI to request are not specified!"
	<< "\nUsage: -h <host> -p <port> -m <secureMode> -u <UMI>" << std::endl;
      return 1;
    }
  try
  {
    int port = 0, mode = 0;
    /** Timeout to get connected */
    int timeout = 10;
    std::string um;
    std::ostringstream hostname;

    if (argv[1] == std::string ("-p"))
      port = atoi (argv[2]);
    else if (argv[1] == std::string ("-h"))
      hostname << argv[2];
    else if (argv[1] == std::string ("-m"))
      mode = atoi (argv[2]);
    else if (argv[1] == std::string ("-u"))
      um = (argv[2]);

    if (argv[3] == std::string ("-p"))
      port = atoi (argv[4]);
    else if (argv[3] == std::string ("-h"))
      hostname << argv[4];
    else if (argv[3] == std::string ("-m"))
      mode = atoi (argv[4]);
    else if (argv[3] == std::string ("-u"))
      um = (argv[4]);

    if (argv[5] == std::string ("-p"))
      port = atoi (argv[6]);
    else if (argv[5] == std::string ("-h"))
      hostname << argv[6];
    else if (argv[5] == std::string ("-m"))
      mode = atoi (argv[6]);
    else if (argv[5] == std::string ("-u"))
      um = (argv[6]);

    if (argv[7] == std::string ("-p"))
      port = atoi (argv[8]);
    else if (argv[7] == std::string ("-h"))
      hostname << argv[8];
    else if (argv[7] == std::string ("-m"))
      mode = atoi (argv[8]);
    else if (argv[7] == std::string ("-u"))
      um = (argv[8]);

    ProcessedDataClient *pclient = new ProcessedDataClient (hostname.str (), port, timeout, mode, um);

    /** Stop here for 30 seconds */
    dass::client::ClientConnection::StartService (30);

    dass::client::ProcessedDataService * svc = pclient->getService ();
    dass::client::ProcessedDataClientConnection * conn = pclient->getConn ();

    if (svc != NULL)
    {
			svc->stop ();
			std::cout << "\nStopping request!" << std::endl;
    }
    /** Stop here for 5 seconds */
    dass::client::ClientConnection::StartService (5);

    if (pclient->getStatusService () != NULL)
    {
			pclient->getStatusService ()->stop ();
			std::cout << "\nStopping status service request!" << std::endl;
    }
    /** Stop here for 5 seconds */
    dass::client::ClientConnection::StartService (5);

    if (conn != NULL && conn->connected ())
    {
			conn->disconnect ();
			std::cout << "Closing connection!" << std::endl;
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
