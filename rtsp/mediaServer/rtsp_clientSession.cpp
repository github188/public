

/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2015, Live Networks, Inc.  All rights reserved
// A demo application, showing how to create and run a RTSP client (that can potentially receive multiple streams concurrently).
//
// NOTE: This code - although it builds a running application - is intended only to illustrate how to develop your own RTSP
// client application.  For a full-featured RTSP client application - with much more functionality, and many options - see
// "openRTSP": http://www.live555.com/openRTSP/
/*
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
*/
#include "rtsp_clientSession.h"
// Forward function definitions:
#include "math.h"
// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
  // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
//void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, Rtsp_OutRegister_t* registerInfo);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName) {
  env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
  env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

char eventLoopWatchVariable = 0;
#if 0
int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  // We need at least one "rtsp://" URL argument:
 // if (argc < 2)
  {
    usage(*env, argv[0]);
    //return 1;
  }
	openURL(*env, argv[0], "rtsp://192.168.28.45:554");
  // There are argc-1 URLs: argv[1] through argv[argc-1].  Open and start streaming each one:
  //for (int i = 1; i <= argc-1; ++i)
  //{
 //   openURL(*env, argv[0], argv[i]);
  //}

  // All subsequent activity takes place within the event loop:
  env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
    // This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.

  return 0;

  // If you choose to continue the application past this point (i.e., if you comment out the "return 0;" statement above),
  // and if you don't intend to do anything more with the "TaskScheduler" and "UsageEnvironment" objects,
  // then you can also reclaim the (small) memory used by these objects by uncommenting the following code:
  /*
    env->reclaim(); env = NULL;
    delete scheduler; scheduler = NULL;
  */
}
#endif
// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
  static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0, Rtsp_OutRegister_t *registerInfo= NULL);

protected:
  ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum, Rtsp_OutRegister_t *registerInfo);
    // called only by createNew();
  virtual ~ourRTSPClient();

public:
  Rtsp_OutRegister_t s_registerInfo;
  StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
  static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			     Rtsp_OutRegister_t ourRegiserInfo, char const* streamId = NULL); // identifies the stream itself (optional)

private:
  DummySink(UsageEnvironment& env, MediaSubsession& subsession, Rtsp_OutRegister_t ourRegiserInfo, char const* streamId);
    // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* fReceiveBuffer;
  MediaSubsession& fSubsession;
  char* fStreamId;

  Rtsp_OutRegister_t m_ourRegiserInfo;
  Rtsp_ClientStream_State_t m_ClientState;
  Fream_Info_t m_frameInfo;
  Fream_Info_t m_audioframeInfo;
  struct timeval m_prevTime;
  bool m_getVideoInfo;

public:
	bool m_showdown;


};

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL,  Rtsp_OutRegister_t* registerInfo)
{
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName, 0, registerInfo);
  if (rtspClient == NULL)
  {
    env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
    return;
  }

  ++rtspClientCount;

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  rtspClient->sendDescribeCommand(continueAfterDESCRIBE); //第二个值默认为空
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

	 //((ourRTSPClient*)rtspClient)->m_ClientState.status = RTSPCLIENT_FINISH;
   // ((ourRTSPClient*)rtspClient)->m_ourRegiserInfo.rtspStateCallback((&((ourRTSPClient*)rtspClient)->m_ClientState),
   // 		(ourRTSPClient*)rtspClient);
	
    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
	
    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription, 
    ((ourRTSPClient*)rtspClient)->s_registerInfo.control_parm.turnAudio);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
	env << "client port " << scs.subsession->clientPortNum();
      } else {
	env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, ((ourRTSPClient*)rtspClient)->s_registerInfo);
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession 
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;

  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
  }
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

  if (--rtspClientCount == 0) {
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you might want to comment this out,
    // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
    exit(exitCode);
  }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum, Rtsp_OutRegister_t* registerInfo) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, registerInfo);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum, Rtsp_OutRegister_t* registerInfo)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
	if (registerInfo == NULL)
	{
		printf("\033[33m""ourRTSPClient is fail\n""\033[0m");
	}
	else
	{
		memcpy(&s_registerInfo, registerInfo, sizeof(Rtsp_OutRegister_t));
	}

}

ourRTSPClient::~ourRTSPClient() {
}


// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 10000000
#define DUMMY_SINK_AUDFIO_BUFFER_SIZE 100000

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, Rtsp_OutRegister_t ourRegiserInfo, char const* streamId)
{
  return new DummySink(env, subsession,ourRegiserInfo, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, Rtsp_OutRegister_t ourRegiserInfo, char const* streamId)
  : MediaSink(env),
    fSubsession(subsession) {
  fStreamId = strDup(streamId);
  fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
  m_frameInfo.data = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
  m_audioframeInfo.data = new u_int8_t[DUMMY_SINK_AUDFIO_BUFFER_SIZE];
  m_ourRegiserInfo = ourRegiserInfo;
  m_frameInfo.param = ourRegiserInfo.param;
  m_frameInfo.height = subsession.height;
  m_frameInfo.width = subsession.width;
  m_frameInfo.fps = subsession.videoFPS();
  m_frameInfo.frameSize = 0;
  m_audioframeInfo.param = ourRegiserInfo.param;
  m_ClientState.param = ourRegiserInfo.param;
  m_ClientState.status = RTSPCLIENT_START;
  //m_ourRegiserInfo.stateCallback(&m_ClientState);
  m_getVideoInfo = 0;
  m_prevTime.tv_sec = m_prevTime.tv_usec = 0;
  m_showdown = 0;
}

DummySink::~DummySink() {
  delete[] fReceiveBuffer;
  delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 0
unsigned int Ue(char *pBuff, unsigned int nLen, unsigned int &nStartBit)
{
    //计算0bit的个数
    unsigned int nZeroNum = 0;
    while (nStartBit < nLen * 8)
    {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
        {
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;


    //计算结果
    unsigned long dwRet = 0;
    for (unsigned int i=0; i<nZeroNum; i++)
    {
        dwRet <<= 1;
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
    return (1 << nZeroNum) - 1 + dwRet;
}


int Se(char *pBuff, unsigned int nLen, unsigned int &nStartBit)
{


	int UeVal=Ue(pBuff,nLen,nStartBit);
	double k=UeVal;
	int nValue=ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2==0)
	{
		nValue=-nValue;
	}

	return nValue;


}


unsigned long u(unsigned int BitCount,char * buf,unsigned int &nStartBit)
{
    unsigned long dwRet = 0;
    for (unsigned int i=0; i<BitCount; i++)
    {
        dwRet <<= 1;
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
return dwRet;
}

bool h264_decode_seq_parameter_set(char * buf,unsigned int nLen,int &Width,int &Height)
{
	unsigned int StartBit=0;
	u(1,buf,StartBit);
	u(2,buf,StartBit);
	int nal_unit_type=u(5,buf,StartBit);
	if(nal_unit_type==7)
	{
		int profile_idc=u(8,buf,StartBit);
		u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
		u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
		u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
		u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
		u(4,buf,StartBit);
		u(8,buf,StartBit);

		Ue(buf,nLen,StartBit);

		if( profile_idc == 100 || profile_idc == 110 ||
		profile_idc == 122 || profile_idc == 144 )
		{
			int chroma_format_idc=Ue(buf,nLen,StartBit);
			if( chroma_format_idc == 3 )
			u(1,buf,StartBit);
			Ue(buf,nLen,StartBit);
			Ue(buf,nLen,StartBit);
			u(1,buf,StartBit);
			int seq_scaling_matrix_present_flag=u(1,buf,StartBit);


			if( seq_scaling_matrix_present_flag )
			{
				for( int i = 0; i < 8; i++ )
				{
					u(1,buf,StartBit);
				}
			}
		}

		Ue(buf,nLen,StartBit);
		int pic_order_cnt_type=Ue(buf,nLen,StartBit);
		if( pic_order_cnt_type == 0 )
		{
			Ue(buf,nLen,StartBit);
		}


		else if( pic_order_cnt_type == 1 )
		{
			u(1,buf,StartBit);
			Se(buf,nLen,StartBit);
			Se(buf,nLen,StartBit);
			int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);

			int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];
			for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
			offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
			delete [] offset_for_ref_frame;
		}
		Ue(buf,nLen,StartBit);
		u(1,buf,StartBit);
		int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
		int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);

		Width=(pic_width_in_mbs_minus1+1)*16;
		Height=(pic_height_in_map_units_minus1+1)*16;

		return true;
	}
	return false;
}



void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
  #if 0
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
#endif
  if(!strcmp(fSubsession.mediumName(), "video"))  
  { 
  	if((fReceiveBuffer[0] & 0xf) == 0x7 && m_getVideoInfo == 0)
	{
	  h264_decode_seq_parameter_set((char*)fReceiveBuffer, frameSize,m_frameInfo.width,m_frameInfo.height );
	  m_getVideoInfo = 1;
	}

	 if(!(m_prevTime.tv_sec == presentationTime.tv_sec && m_prevTime.tv_usec == presentationTime.tv_usec)
			  && m_prevTime.tv_sec != 0 && m_prevTime.tv_usec != 0)
	  {
		 // printf("fReceiveBuffer[0] & 0xf:%0x'n", fReceiveBuffer[0] & 0xf);
		  m_frameInfo.type = VIDEO_TYPE;
		  m_ourRegiserInfo.frameCall(&m_frameInfo);
		 // memset(m_frameInfo.data, 0, DUMMY_SINK_RECEIVE_BUFFER_SIZE);
		  m_frameInfo.frameSize = 0;
		  m_frameInfo.iFrame = 0;
	  }

      
		if (m_frameInfo.frameSize + 4 < DUMMY_SINK_RECEIVE_BUFFER_SIZE)
		{

		   if((fReceiveBuffer[0] & 0xf) == 0x5)
		   {
			   m_frameInfo.iFrame = 1;
		   }
		   m_frameInfo.data[m_frameInfo.frameSize] = 0x00;
		   m_frameInfo.data[m_frameInfo.frameSize + 1] = 0x00;
		   m_frameInfo.data[m_frameInfo.frameSize + 2] = 0x00;
		   m_frameInfo.data[m_frameInfo.frameSize + 3] = 0x01;

		   m_frameInfo.frameSize += 4;

		   if( m_frameInfo.frameSize + frameSize <	DUMMY_SINK_RECEIVE_BUFFER_SIZE)
		   {
			   memcpy(m_frameInfo.data + m_frameInfo.frameSize, fReceiveBuffer, frameSize);
			   m_frameInfo.frameSize += frameSize;
		   }
		   else
		   {
			   m_frameInfo.frameSize = 0;
			   m_frameInfo.iFrame = 0;
			   printf("\033[32m""frameSize is over %d\n""\033[0m", DUMMY_SINK_RECEIVE_BUFFER_SIZE);
		   }

			
			if( m_prevTime.tv_sec == 0 && m_prevTime.tv_usec == 0)
			 {
				 m_prevTime = presentationTime;
			 }
			 else
			 {
				 if(m_frameInfo.fps == 0 && !(m_prevTime.tv_sec == presentationTime.tv_sec && m_prevTime.tv_usec == presentationTime.tv_usec))
				 {
					 m_frameInfo.fps = (1000 * 1000)/
					 ((presentationTime.tv_sec - m_prevTime.tv_sec) * 1000 * 1000 + (presentationTime.tv_usec - m_prevTime.tv_usec));
				 }
	
			 } 
		}
		else
		{
			m_frameInfo.frameSize = 0;
		 	m_frameInfo.iFrame = 0;
		}
		
		m_prevTime = presentationTime;
  	}
  	else if (0 == strcmp(fSubsession.mediumName(), "audio"))
  	{
		m_audioframeInfo.type = AUDIO_TYPE;
 	 	memcpy(m_audioframeInfo.data, fReceiveBuffer, frameSize);
 	 	m_audioframeInfo.frameSize = frameSize;
 	 	m_ourRegiserInfo.frameCall(&m_audioframeInfo);
	}

  continuePlaying();
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}


