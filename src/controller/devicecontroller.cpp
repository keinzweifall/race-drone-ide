/*
 * Copyright 2016 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include <boost/log/trivial.hpp>
#include <boost/concept_check.hpp>

#include "model/cmdqueue.h"

// @TODO
// should be configurable from external source (cmd line, property-file)
#define JS_IP_ADDRESS "192.168.2.1"
#define JS_DISCOVERY_PORT 44444
#define JS_D2C_PORT 43210

#ifdef __cplusplus
extern "C" {
#endif

#include <libARSAL/ARSAL.h>
#include <libARSAL/ARSAL_Print.h>
#include <libARNetwork/ARNetwork.h>
#include <libARNetworkAL/ARNetworkAL.h>
#include <libARCommands/ARCommands.h>
#include <libARDiscovery/ARDiscovery.h>
#include <libARStream/ARStream.h>
  
#ifdef __cplusplus 
}
#endif

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "devicecontroller.h"

typedef struct {
    DeviceController*_dcp;
    ARSAL_Thread_t _thread;
    int _bufferId;
} READER_THREAD_DATA_T;

struct DEVICE_CONTROLLER_DATA {
  std::string _ipadr;
  int _discoveryport;
  ARNETWORKAL_Manager_t* _alManager;
  ARNETWORK_Manager_t* _netManager;
  ARSTREAM_Reader_t* _streamReader;
  ARSAL_Thread_t _rxThread;
  ARSAL_Thread_t _txThread;
  ARSAL_Thread_t _videoTxThread;
  ARSAL_Thread_t _videoRxThread;
  ARSAL_Thread_t _cmdTxThread;
  int _d2cPort;
  int _c2dPort;
  int _arstreamFragSize;
  int _arstreamFragNb;
  int _arstreamAckDelay;
  uint8_t* _videoFrame;
  uint32_t _videoFrameSize;

  READER_THREAD_DATA_T* _rdata_ack;
  READER_THREAD_DATA_T* _rdata_nonack;
  
  // FILE *_video_out;
  VideoFrameFeed* _video_feed;
  
  RACEDRONE_VALUES_T* _values;
  boost::mutex _mtx;
};


eARDISCOVERY_ERROR discovery_connection_send_json_Callback (uint8_t *dataTx, uint32_t *dataTxSize, void *customData);
eARDISCOVERY_ERROR discovery_connection_receive_json_callback (uint8_t *dataRx, uint32_t dataRxSize, char *ip, void *customData);
void on_disconnect_network (ARNETWORK_Manager_t *manager, ARNETWORKAL_Manager_t *alManager, void *customData);
void register_arcommands_callbacks (DEVICE_CONTROLLER_DATA_T *dcdata);
void unregister_arcommands_callbacks();
uint8_t *video_frame_complete_cb(eARSTREAM_READER_CAUSE cause, uint8_t *frame, uint32_t frameSize, int numberOfSkippedFrames, int isFlushFrame, uint32_t *newBufferCapacity, void *custom);
eARNETWORK_MANAGER_CALLBACK_RETURN arnetworkCmdCallback(int buffer_id, uint8_t *data, void *custom, eARNETWORK_MANAGER_CALLBACK_STATUS cause);


/*
 * @TODO begin this was copy & pasted from sample, change later
 * 
 */
#define JS_NET_CD_NONACK_ID 10
#define JS_NET_CD_ACK_ID 11
#define JS_NET_CD_VIDEO_ACK_ID 13
#define JS_NET_DC_NONACK_ID 127
#define JS_NET_DC_ACK_ID 126
#define JS_NET_DC_VIDEO_ID 125

static ARNETWORK_IOBufferParam_t c2dParams[] = {
    {
        .ID = JS_NET_CD_NONACK_ID,
        .dataType = ARNETWORKAL_FRAME_TYPE_DATA,
        .sendingWaitTimeMs = 5,
        .ackTimeoutMs = -1,
        .numberOfRetry = -1,
        .numberOfCell = 10,
        .dataCopyMaxSize = 128,
        .isOverwriting = 0,
    },
    {
        .ID = JS_NET_CD_ACK_ID,
        .dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK,
        .sendingWaitTimeMs = 20,
        .ackTimeoutMs = 500,
        .numberOfRetry = 3,
        .numberOfCell = 20,
        .dataCopyMaxSize = 128,
        .isOverwriting = 0,
    },
    {
        .ID = JS_NET_CD_VIDEO_ACK_ID,
        .dataType = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED,
        .sendingWaitTimeMs = 0,
        .ackTimeoutMs = 0,
        .numberOfRetry = 0,
        .numberOfCell = 0,
        .dataCopyMaxSize = 0,
        .isOverwriting = 0,
    }
};

static const size_t numC2dParams = sizeof(c2dParams) / sizeof(ARNETWORK_IOBufferParam_t);

static ARNETWORK_IOBufferParam_t d2cParams[] = {
    {
        .ID = JS_NET_DC_NONACK_ID,
        .dataType = ARNETWORKAL_FRAME_TYPE_DATA,
        .sendingWaitTimeMs = 20,
        .ackTimeoutMs = -1,
        .numberOfRetry = -1,
        .numberOfCell = 10,
        .dataCopyMaxSize = 128,
        .isOverwriting = 0,
    },
    {
        .ID = JS_NET_DC_ACK_ID,
        .dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK,
        .sendingWaitTimeMs = 20,
        .ackTimeoutMs = 500,
        .numberOfRetry = 3,
        .numberOfCell = 20,
        .dataCopyMaxSize = 128,
        .isOverwriting = 0,
    },
    {
        .ID = JS_NET_DC_VIDEO_ID,
        .dataType = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED,
        .sendingWaitTimeMs = 0,
        .ackTimeoutMs = 0,
        .numberOfRetry = 0,
        .numberOfCell = 0,
        .dataCopyMaxSize = 0,
        .isOverwriting = 0,
    }
};

static const size_t numD2cParams = sizeof(d2cParams) / sizeof(ARNETWORK_IOBufferParam_t);

/*
 * @TODO end this was copy & pasted from sample, change later
 * 
 */

volatile bool g_StopFlag;

DeviceController::DeviceController() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::DeviceController()";

  _handle = new DEVICE_CONTROLLER_DATA_T;
  _handle->_values = new RACEDRONE_VALUES_T;
  _init_handle();
}

DeviceController::~DeviceController() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::~DeviceController()";

  if (isConnected()) {
    disconnect();
  }
  delete _handle->_values;
  delete _handle;
}

bool DeviceController::connect() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::connect()";

  register_arcommands_callbacks(this->_handle);
  
  if ( ! _internal_connect() ) {
    disconnect();
    return false;
  }
  
  return true;
}

bool DeviceController::disconnect() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::disconnect()";
  
  if (!_handle) {
    BOOST_LOG_TRIVIAL(trace) << __LINE__ << "wasn't connected";
    return true;
  }
  
  if (_handle->_video_feed) {
    stopVideo();
  }
  
  if (_handle->_netManager) {
    ARNETWORK_Manager_Stop(_handle->_netManager);
    if (_handle->_rxThread) {
	ARSAL_Thread_Join(_handle->_rxThread, NULL);
	ARSAL_Thread_Destroy(&(_handle->_rxThread));
	_handle->_rxThread = nullptr;
    }
    if (_handle->_txThread) {
	ARSAL_Thread_Join(_handle->_txThread, NULL);
	ARSAL_Thread_Destroy(&(_handle->_txThread));
	_handle->_txThread = nullptr;
    }
  }
  if (_handle->_alManager) {
    ARNETWORKAL_Manager_Unlock(_handle->_alManager);
    ARNETWORKAL_Manager_CloseWifiNetwork(_handle->_alManager);
  }

  ARNETWORK_Manager_Delete(&(_handle->_netManager));
  ARNETWORKAL_Manager_Delete(&(_handle->_alManager));
  
  unregister_arcommands_callbacks();
  
  _init_handle();
}

int DeviceController::ping() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::ping()";

  if (!isConnected()) {
    return -1;
  }

  int estimatedPingTime = ARNETWORK_Manager_GetEstimatedLatency(_handle->_netManager);
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "estimatedPingTime is: " << estimatedPingTime;
  
  return estimatedPingTime;
}

bool DeviceController::isConnected() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::isConnected()";

  if (!_handle || !_handle->_alManager || !_handle->_netManager) {
    return false;
  }

  return true;
}
  
RACEDRONE_VALUES_T DeviceController::getValues() {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::getValues()";
  boost::lock_guard<boost::mutex> guard(_handle->_mtx);
  
  RACEDRONE_VALUES_T pv;

  pv.alert = _handle->_values->alert;
  pv.batteryChargePercentage = _handle->_values->batteryChargePercentage;
  pv.connected = _handle->_values->connected;
  pv.posture = _handle->_values->posture;
  pv.speedInCmPerSec = _handle->_values->speedInCmPerSec;
  pv.speedVal = _handle->_values->speedVal;
  pv.turnVal = _handle->_values->turnVal;
  pv.linkQuality = _handle->_values->linkQuality;

  return pv;
}

bool DeviceController::enterControlLoop() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::enterControlLoop()";

  if (!isConnected()) {
    return false;
  }
  
  g_StopFlag = false;
  
  // start reader loops
  if (ARSAL_Thread_Create(&(_handle->_rdata_ack->_thread), DeviceController::_reader_loop, _handle->_rdata_ack) != 0) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "creation of reader thread failed";
    return false;
  }

  if (ARSAL_Thread_Create(&(_handle->_rdata_nonack->_thread), DeviceController::_reader_loop, _handle->_rdata_nonack) != 0) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "creation of reader thread failed";
    return false;
  }
  
  // start sender loop
  if (ARSAL_Thread_Create(&(_handle->_cmdTxThread), DeviceController::_sender_loop, this) != 0) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "creation of sender thread failed";
    return false;
  }
  
  return true;
}

bool DeviceController::exitControlLoop() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::exitControlLoop()";

  if (!isConnected()) {
    return false;
  }

  // signal stop
  // @TODO: synchronize access?
  g_StopFlag = true;
  
  ARSAL_Thread_Join(_handle->_cmdTxThread, NULL);
  ARSAL_Thread_Destroy(&(_handle->_cmdTxThread));
  _handle->_cmdTxThread = nullptr;

  ARSAL_Thread_Join(_handle->_rdata_ack->_thread, NULL);
  ARSAL_Thread_Destroy(&(_handle->_rdata_ack->_thread));
  _handle->_rdata_ack->_thread = nullptr;
  delete _handle->_rdata_ack;
  _handle->_rdata_ack = nullptr;

  ARSAL_Thread_Join(_handle->_rdata_nonack->_thread, NULL);
  ARSAL_Thread_Destroy(&(_handle->_rdata_nonack->_thread));
  _handle->_rdata_nonack->_thread = nullptr;
  delete _handle->_rdata_nonack;
  _handle->_rdata_nonack = nullptr;
}

void DeviceController::_init_handle() {
    if (_handle == nullptr) {
      BOOST_LOG_TRIVIAL(error) << __LINE__ << "_handle is null in _init_handle";
      return;
    }
    
    _handle->_ipadr = JS_IP_ADDRESS;
    _handle->_discoveryport = JS_DISCOVERY_PORT;
    
    _handle->_alManager = nullptr;
    _handle->_netManager = nullptr;
    _handle->_streamReader = nullptr;
    _handle->_rxThread = nullptr;
    _handle->_txThread = nullptr;
    _handle->_videoRxThread = nullptr;
    _handle->_videoTxThread = nullptr;
    _handle->_d2cPort = JS_D2C_PORT;
    _handle->_c2dPort = -1;
    _handle->_arstreamAckDelay = -1;
    _handle->_arstreamFragNb = -1;
    _handle->_arstreamFragSize = -1;
    _handle->_videoFrame = nullptr;
    _handle->_videoFrameSize = 0;
    
    _handle->_rdata_ack = new READER_THREAD_DATA_T;
    _handle->_rdata_ack->_bufferId = JS_NET_DC_ACK_ID;
    _handle->_rdata_ack->_dcp = this;
    _handle->_rdata_nonack = new READER_THREAD_DATA_T;
    _handle->_rdata_nonack->_bufferId = JS_NET_DC_NONACK_ID;
    _handle->_rdata_nonack->_dcp = this;

    _handle->_values->alert = false;
    _handle->_values->connected = false;
    _handle->_values->batteryChargePercentage = -1.0;
    _handle->_values->speedInCmPerSec = -1;
    _handle->_values->speedVal = 0;
    _handle->_values->turnVal = 0;
    _handle->_values->linkQuality = 0;
    _handle->_values->posture = "-none-";
}

bool DeviceController::_internal_connect() {
  eARDISCOVERY_ERROR err = ARDISCOVERY_ERROR;
  eARNETWORK_ERROR netError = ARNETWORK_OK;
  eARNETWORKAL_ERROR netAlError = ARNETWORKAL_OK;
  int pingDelay = 0; // 0 means default, -1 means no ping
  
  // discover drone
  ARDISCOVERY_Connection_ConnectionData_t *discoveryData = ARDISCOVERY_Connection_New(discovery_connection_send_json_Callback, discovery_connection_receive_json_callback, _handle, &err);
  if (discoveryData != NULL && err == ARDISCOVERY_OK) {
      eARDISCOVERY_ERROR err = ARDISCOVERY_Connection_ControllerConnection(discoveryData, _handle->_discoveryport, _handle->_ipadr.c_str());
      if (err != ARDISCOVERY_OK) {
	  BOOST_LOG_TRIVIAL(error) << __LINE__ << "error opening discovery connection : " << ARDISCOVERY_Error_ToString(err);
	  return false;
      }
  }

  ARDISCOVERY_Connection_Delete(&discoveryData);
  
  // Init Video Stream
  ARSTREAM_Reader_InitStreamDataBuffer(&d2cParams[2], JS_NET_DC_VIDEO_ID, _handle->_arstreamFragSize, _handle->_arstreamFragNb);
  // optional according to docu 
  ARSTREAM_Reader_InitStreamAckBuffer(&c2dParams[2], JS_NET_CD_VIDEO_ACK_ID);
  
  // initialize ARNetworkALManager
  _handle->_alManager = ARNETWORKAL_Manager_New(&netAlError);
  if (netAlError != ARNETWORKAL_OK) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "error creating network-al-manager : " << ARNETWORKAL_Error_ToString(netAlError);
    return false;
  }
  
  netAlError = ARNETWORKAL_Manager_InitWifiNetwork(_handle->_alManager, _handle->_ipadr.c_str(), _handle->_c2dPort, _handle->_d2cPort, 1);
  if (netAlError != ARNETWORKAL_OK) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "error initializing wifi network : " << ARNETWORKAL_Error_ToString(netAlError);
    return false;
  }
  
  // initialize ARNetworkManager
  // _handle->_netManager = ARNETWORK_Manager_New(_handle->_alManager, numC2dParams, c2dParams, numD2cParams, d2cParams, pingDelay, on_disconnect_network, _handle, &netError);
  _handle->_netManager = ARNETWORK_Manager_New(_handle->_alManager, numC2dParams, c2dParams, numD2cParams, d2cParams, pingDelay, on_disconnect_network, _handle, &netError);
  if (netError != ARNETWORK_OK) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "error creating network-manager : " << ARNETWORK_Error_ToString(netError);
    return false;
  }

  // Create and start Tx and Rx threads.
  if (ARSAL_Thread_Create(&(_handle->_rxThread), ARNETWORK_Manager_ReceivingThreadRun, _handle->_netManager) != 0) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "error starting network reader thread";
    return false;
  }

  if (ARSAL_Thread_Create(&(_handle->_txThread), ARNETWORK_Manager_SendingThreadRun, _handle->_netManager) != 0) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "error starting network sender thread";
    return false;
  }

  _handle->_values->connected = true;
  
  return true;
}


VideoFrameFeed* DeviceController::startVideo() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::startVideo()";
  
  eARSTREAM_ERROR err = ARSTREAM_OK;

  _handle->_video_feed = new VideoFrameFeed(_handle->_arstreamFragSize, 100);

  // check if frame size is set
  if (_handle->_arstreamFragSize == 0 || _handle->_arstreamFragNb == 0) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "drone did not send video parameters in json data.";
    return nullptr;
  }
  
  _handle->_videoFrameSize = _handle->_arstreamFragSize * _handle->_arstreamFragNb;
  _handle->_videoFrame = (uint8_t*)std::malloc(_handle->_videoFrameSize);
	
  _handle->_streamReader = ARSTREAM_Reader_New (_handle->_netManager, JS_NET_DC_VIDEO_ID, JS_NET_CD_VIDEO_ACK_ID, video_frame_complete_cb, _handle->_videoFrame, _handle->_videoFrameSize, _handle->_arstreamFragSize, _handle->_arstreamAckDelay, _handle, &err);
  if (err != ARSTREAM_OK) {
    BOOST_LOG_TRIVIAL(error) << "Error during ARStream creation : " << ARSTREAM_Error_ToString(err);
    return nullptr;
  }

  if (ARSAL_Thread_Create(&(_handle->_videoRxThread), ARSTREAM_Reader_RunDataThread, _handle->_streamReader) != 0) {
    BOOST_LOG_TRIVIAL(error) << "Creation of video Rx thread failed.";
    return nullptr;
  }

  if (ARSAL_Thread_Create(&(_handle->_videoTxThread), ARSTREAM_Reader_RunAckThread, _handle->_streamReader) != 0) {
    BOOST_LOG_TRIVIAL(error) << "Creation of video Tx thread failed.";
    return nullptr;
  }

  // tell drone to send video feed
  _send_media_stream(true);
  
  return _handle->_video_feed;
}

void DeviceController::stopVideo() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::stopVideo()";
  
  // tell drone to stop sending video feed
  _send_media_stream(false);
  
  usleep(10000);

  delete _handle->_video_feed;
  _handle->_video_feed = nullptr;
  
  if (_handle->_streamReader) {
    ARSTREAM_Reader_StopReader(_handle->_streamReader);

    // Optional, but better for speed:
    ARNETWORKAL_Manager_Unlock(_handle->_alManager);

    if (_handle->_videoRxThread != NULL) {
      ARSAL_Thread_Join(_handle->_videoRxThread, NULL);
      ARSAL_Thread_Destroy(&(_handle->_videoRxThread));
      _handle->_videoRxThread = NULL;
    }
    if (_handle->_videoTxThread != NULL) {
      ARSAL_Thread_Join(_handle->_videoTxThread, NULL);
      ARSAL_Thread_Destroy(&(_handle->_videoTxThread));
      _handle->_videoTxThread = NULL;
    }

    ARSTREAM_Reader_Delete (&(_handle->_streamReader));
  }

  if (_handle->_videoFrame) {
    free (_handle->_videoFrame);
    _handle->_videoFrame = NULL;
  }
}

int DeviceController::_send_media_stream(bool streamOn) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::_send_begin_stream()";
  
  int sentStatus = 1;
  u_int8_t cmdBuffer[128];
  int32_t cmdSize = 0;
  eARCOMMANDS_GENERATOR_ERROR cmdError;
  eARNETWORK_ERROR netError = ARNETWORK_ERROR;

  // Send Streaming begin command
  cmdError = ARCOMMANDS_Generator_GenerateJumpingSumoMediaStreamingVideoEnable(cmdBuffer, sizeof(cmdBuffer), &cmdSize, (streamOn) ? 1 : 0);
  if (cmdError == ARCOMMANDS_GENERATOR_OK) {
    netError = ARNETWORK_Manager_SendData(_handle->_netManager, JS_NET_CD_ACK_ID, cmdBuffer, cmdSize, NULL, &(arnetworkCmdCallback), 1);
  }

  if ((cmdError != ARCOMMANDS_GENERATOR_OK) || (netError != ARNETWORK_OK)) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "failed to send JumpingSump.MediaStreamingVideoEnable cmd : " << cmdError << ", " << ARNETWORK_Error_ToString(netError);
    sentStatus = 0;
  }

  return sentStatus;
}

uint8_t *video_frame_complete_cb(eARSTREAM_READER_CAUSE cause, uint8_t *frame, uint32_t frameSize, int numberOfSkippedFrames, int isFlushFrame, uint32_t *newBufferCapacity, void *custom)
{
  uint8_t *ret = NULL;
  DEVICE_CONTROLLER_DATA_T *dcd = (DEVICE_CONTROLLER_DATA_T *)custom;

  switch(cause) {
    case ARSTREAM_READER_CAUSE_FRAME_COMPLETE:
      /* Here, the mjpeg video frame is in the "frame" pointer, with size "frameSize" bytes
      You can do what you want, but keep it as short as possible, as the video is blocked until you return from this callback.
      Typically, you will either copy the frame and return the same buffer to the library, or store the buffer
      in a fifo for pending operations, and provide a new one.
      In this sample, we do nothing and just pass the buffer back*/
      // fwrite(frame, frameSize, 1, dcd->_video_out);
      // fflush (dcd->_video_out);
      dcd->_video_feed->setNewFrame(dcd->_videoFrame);
      ret = dcd->_videoFrame;
      *newBufferCapacity = dcd->_videoFrameSize;
      
    break;
    case ARSTREAM_READER_CAUSE_FRAME_TOO_SMALL:
      /* This case should not happen, as we've allocated a frame pointer to the maximum possible size. */
      BOOST_LOG_TRIVIAL(error) << __LINE__ << "ARSTREAM_READER_CAUSE_FRAME_TOO_SMALL";
      ret = dcd->_videoFrame;
      *newBufferCapacity = dcd->_videoFrameSize;
    break;
    case ARSTREAM_READER_CAUSE_COPY_COMPLETE:
      /* Same as before ... but return value are ignored, so we just do nothing */
    break;
    case ARSTREAM_READER_CAUSE_CANCEL:
      /* Called when the library closes, return values ignored, so do nothing here */
    break;
    default:
    break;
  }

  return ret;
}


eARNETWORK_MANAGER_CALLBACK_RETURN arnetworkCmdCallback(int buffer_id, uint8_t *data, void *custom, eARNETWORK_MANAGER_CALLBACK_STATUS cause) {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "arnetworkCmdCallback bufferId: " << buffer_id << ", cause: " << cause;

  return (cause == ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT) ? ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP : ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
}


int send_all_states_cmd(DEVICE_CONTROLLER_DATA_T* p) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "send_all_states_cmd";

  int sentStatus = 1;
  u_int8_t cmdBuffer[128];
  int32_t cmdSize = 0;
  eARCOMMANDS_GENERATOR_ERROR cmdError;
  eARNETWORK_ERROR netError = ARNETWORK_ERROR;
  
  cmdError = ARCOMMANDS_Generator_GenerateCommonCommonAllStates(cmdBuffer, sizeof(cmdBuffer), &cmdSize);
  if (cmdError == ARCOMMANDS_GENERATOR_OK) {
      netError = ARNETWORK_Manager_SendData(p->_netManager, JS_NET_CD_NONACK_ID, cmdBuffer, cmdSize, NULL, &(arnetworkCmdCallback), 1);
  }
  
  if ((cmdError != ARCOMMANDS_GENERATOR_OK) || (netError != ARNETWORK_OK)) {
      BOOST_LOG_TRIVIAL(error) << __LINE__ << "failed to send Common.AllStates cmd : " << cmdError << ", " << ARNETWORK_Error_ToString(netError);
      sentStatus = 0;
  }
  
  return sentStatus;
}

int send_PCMD_cmd(DEVICE_CONTROLLER_DATA_T* p, u_int8_t flag, u_int8_t speed, u_int8_t turn)
{
    int sentStatus = 1;
    u_int8_t cmdBuffer[128];
    int32_t cmdSize = 0;
    eARCOMMANDS_GENERATOR_ERROR cmdError;
    eARNETWORK_ERROR netError = ARNETWORK_ERROR;
    
    // Send Posture command
    cmdError = ARCOMMANDS_Generator_GenerateJumpingSumoPilotingPCMD(cmdBuffer, sizeof(cmdBuffer), &cmdSize, flag, speed, turn);
    if (cmdError == ARCOMMANDS_GENERATOR_OK)
    {
        // The commands sent in loop should be sent to a buffer not acknowledged ; here JS_NET_CD_NONACK_ID
        netError = ARNETWORK_Manager_SendData(p->_netManager, JS_NET_CD_NONACK_ID, cmdBuffer, cmdSize, NULL, &(arnetworkCmdCallback), 1);
    }
    
    if ((cmdError != ARCOMMANDS_GENERATOR_OK) || (netError != ARNETWORK_OK))
    {
        sentStatus = 0;
    }
    
    return sentStatus;
}



// send commands to device
void* DeviceController::_sender_loop(void* p) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "entering sender_loop...";

  DeviceController* dcp = (DeviceController*) p;
  
  // empty queue before start
  dcp->_cmdq->clear();
  
  while (!g_StopFlag) {
    COMMAND_T cmd = dcp->_cmdq->get();

    switch (cmd.type) {
      case eCMD_NOCOMMAND:
	break;
      case eCMD_EMERGENCYSTOP:
	break;
      case eCMD_PILOTING:
	send_PCMD_cmd(dcp->_handle, cmd.flag, cmd.speed, cmd.turn);
	break;
      case eCMD_ANIMATION:
	break;
    }
  }
  
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "leaving sender_loop...";

  return nullptr;
}

// read from network layer and dispatch via Decoder
void* DeviceController::_reader_loop(void* p) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "entering reader_loop...";

  READER_THREAD_DATA_T* rdata = (READER_THREAD_DATA_T*) p;
  DeviceController* dcp = rdata->_dcp;

  // Allocate some space for incoming data.
  const size_t maxLength = 128 * 1024;
  uint8_t* readData = (uint8_t*) malloc(maxLength);
  if (readData == NULL) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "couldn't allocate reader buffer, exiting reader_loop";
    return nullptr;
  }

  // evaluate the read loop until stopped or too many errors
  unsigned int cntErrors = 0;
  while (cntErrors < 1000 && !g_StopFlag) {
    int length = 0;
    int timeOutInMs = 1000;

    // read data and decode/dispatch command
    eARNETWORK_ERROR netError = ARNETWORK_Manager_ReadDataWithTimeout(dcp->_handle->_netManager, rdata->_bufferId, readData, maxLength, &length, timeOutInMs);
    if (netError == ARNETWORK_OK) {
      eARCOMMANDS_DECODER_ERROR cmdError = ARCOMMANDS_Decoder_DecodeBuffer((uint8_t *)readData, length);
      if (cmdError != ARCOMMANDS_DECODER_OK && cmdError != ARCOMMANDS_DECODER_ERROR_NO_CALLBACK) {
	char msg[128];
	ARCOMMANDS_Decoder_DescribeBuffer ((uint8_t *)readData, length, msg, sizeof(msg));
	BOOST_LOG_TRIVIAL(error) << __LINE__ << "error " << cmdError << " decoding command ARCOMMANDS_Decoder_DecodeBuffer() : " << msg;
      }
    } else if (netError == ARNETWORK_ERROR_BUFFER_EMPTY) {
      // buffer was empty, nothing to read, everything ok
    } else {
      BOOST_LOG_TRIVIAL(error) << __LINE__ << "error reading from network layer with ARNETWORK_Manager_ReadDataWithTimeout() : " << ARNETWORK_Error_ToString(netError);
      cntErrors++;
    }
  }

  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "leaving reader_loop...";

  if (readData != NULL) {
    free (readData);
    readData = NULL;
  }
  
  return nullptr;
}

eARDISCOVERY_ERROR discovery_connection_send_json_Callback (uint8_t *dataTx, uint32_t *dataTxSize, void *customData)
{
  DEVICE_CONTROLLER_DATA_T *dcdata = (DEVICE_CONTROLLER_DATA_T *)customData;
  eARDISCOVERY_ERROR err = ARDISCOVERY_ERROR;
    
  if ((dataTx != NULL) && (dataTxSize != NULL) && (dcdata != NULL))
    {
      rapidjson::StringBuffer s;
      rapidjson::Writer<rapidjson::StringBuffer> writer(s);
      
      writer.StartObject();
      writer.String(ARDISCOVERY_CONNECTION_JSON_D2CPORT_KEY);
      writer.Int(dcdata->_d2cPort);
      writer.String(ARDISCOVERY_CONNECTION_JSON_CONTROLLER_NAME_KEY);
      writer.String("RACE_DRONE_IDE");
      writer.String(ARDISCOVERY_CONNECTION_JSON_CONTROLLER_TYPE_KEY);
      writer.String("HOST");
      writer.EndObject();

      BOOST_LOG_TRIVIAL(trace) << __LINE__ << s.GetString() << ", length is: " << s.GetSize();
      
      // copy string
      strcpy((char*)dataTx, s.GetString());
      // string length + 1 trailing '\0'
      *dataTxSize = s.GetSize() + 1;
      
      BOOST_LOG_TRIVIAL(trace) << __LINE__ << (const char*)dataTx << ", length is: " << *dataTxSize;

      err = ARDISCOVERY_OK;
    }
    
    return err;
}

eARDISCOVERY_ERROR discovery_connection_receive_json_callback (uint8_t *dataRx, uint32_t dataRxSize, char *ip, void *customData)
{
    DEVICE_CONTROLLER_DATA_T *dcdata = (DEVICE_CONTROLLER_DATA_T *)customData;
    eARDISCOVERY_ERROR err = ARDISCOVERY_ERROR;
    
    if ((dataRx != NULL) && (dataRxSize != 0) && (dcdata != NULL))
    {
        char *json_s = (char*) malloc(dataRxSize + 1);
        strncpy(json_s, (char *)dataRx, dataRxSize);
        json_s[dataRxSize] = '\0';

        BOOST_LOG_TRIVIAL(trace) << __LINE__ << "json data from drone: " << json_s;
	
	rapidjson::Document d;
	d.Parse(json_s);
        free(json_s);
	
	// check for mandatory entries
	if (!d.HasMember(ARDISCOVERY_CONNECTION_JSON_STATUS_KEY)) {
	  BOOST_LOG_TRIVIAL(error) << __LINE__ << "mandatory member " << ARDISCOVERY_CONNECTION_JSON_STATUS_KEY << " missing in json data from drone.";
	  return ARDISCOVERY_ERROR;
	}
	if (!d.HasMember(ARDISCOVERY_CONNECTION_JSON_C2DPORT_KEY)) {
	  BOOST_LOG_TRIVIAL(error) << __LINE__ << "mandatory member " << ARDISCOVERY_CONNECTION_JSON_C2DPORT_KEY << " missing in json data from drone.";
	  return ARDISCOVERY_ERROR;
	}
	
	// get entries
	rapidjson::Value &v = d[ARDISCOVERY_CONNECTION_JSON_STATUS_KEY];
	int lStatus = v.GetInt();
	
	if (lStatus != 0) {
	  BOOST_LOG_TRIVIAL(error) << __LINE__ << "drone is refusing connection " << ARDISCOVERY_CONNECTION_JSON_C2DPORT_KEY << " missing in json data from drone.";
	  return ARDISCOVERY_ERROR;
	}
	
	v = d[ARDISCOVERY_CONNECTION_JSON_C2DPORT_KEY];
	dcdata->_c2dPort = v.GetInt();
	
	dcdata->_arstreamFragSize = 0;
	dcdata->_arstreamFragNb = 0;
	
	if (d.HasMember(ARDISCOVERY_CONNECTION_JSON_ARSTREAM_FRAGMENT_SIZE_KEY)) {
	  v = d[ARDISCOVERY_CONNECTION_JSON_ARSTREAM_FRAGMENT_SIZE_KEY];
	  dcdata->_arstreamFragSize = v.GetInt();
	}
	
	if (d.HasMember(ARDISCOVERY_CONNECTION_JSON_ARSTREAM_FRAGMENT_MAXIMUM_NUMBER_KEY)) {
	  v = d[ARDISCOVERY_CONNECTION_JSON_ARSTREAM_FRAGMENT_MAXIMUM_NUMBER_KEY];
	  dcdata->_arstreamFragNb = v.GetInt();
	}

	if (d.HasMember(ARDISCOVERY_CONNECTION_JSON_ARSTREAM_MAX_ACK_INTERVAL_KEY)) {
	  v = d[ARDISCOVERY_CONNECTION_JSON_ARSTREAM_MAX_ACK_INTERVAL_KEY];
	  dcdata->_arstreamAckDelay = v.GetInt();
	}

	err = ARDISCOVERY_OK;
    }
    
    return err;
}

void on_disconnect_network (ARNETWORK_Manager_t *manager, ARNETWORKAL_Manager_t *alManager, void *ptr)
{
  DeviceController* pdcp = (DeviceController*) ptr;
  // Network Disconnected
  pdcp->disconnect();
  
  // @TODO signal this to controller
}

/*
 * 
 * 
 * 
 * C A L L B A C K    S E C T I O N
 * 
 * 
 * 
void ARCOMMANDS_Decoder_SetJumpingSumoPilotingPCMDCallback (ARCOMMANDS_Decoder_JumpingSumoPilotingPCMDCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoPilotingPostureCallback (ARCOMMANDS_Decoder_JumpingSumoPilotingPostureCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoPilotingAddCapOffsetCallback (ARCOMMANDS_Decoder_JumpingSumoPilotingAddCapOffsetCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoPilotingStatePostureChangedCallback (ARCOMMANDS_Decoder_JumpingSumoPilotingStatePostureChangedCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoPilotingStateAlertStateChangedCallback (ARCOMMANDS_Decoder_JumpingSumoPilotingStateAlertStateChangedCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoPilotingStateSpeedChangedCallback (ARCOMMANDS_Decoder_JumpingSumoPilotingStateSpeedChangedCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsJumpStopCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsJumpStopCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsJumpCancelCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsJumpCancelCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsJumpLoadCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsJumpLoadCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsJumpCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsJumpCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsSimpleAnimationCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsSimpleAnimationCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsStateJumpLoadChangedCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsStateJumpLoadChangedCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsStateJumpTypeChangedCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsStateJumpTypeChangedCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetJumpingSumoAnimationsStateJumpMotorProblemChangedCallback (ARCOMMANDS_Decoder_JumpingSumoAnimationsStateJumpMotorProblemChangedCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetCommonCommonCurrentDateCallback (ARCOMMANDS_Decoder_CommonCommonCurrentDateCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetCommonCommonCurrentTimeCallback (ARCOMMANDS_Decoder_CommonCommonCurrentTimeCallback_t callback, void *custom);
void ARCOMMANDS_Decoder_SetCommonCommonRebootCallback (ARCOMMANDS_Decoder_CommonCommonRebootCallback_t callback, void *custom);
 * 
 */

void pcmd_cb (uint8_t flag, int8_t speed, int8_t turn, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "flag: " << (int)flag << ", speed: " << (int)speed << ", turn:" << (int)turn;

  DEVICE_CONTROLLER_DATA_T *dc = (DEVICE_CONTROLLER_DATA_T *)pdc;
  boost::lock_guard<boost::mutex> guard(dc->_mtx);
  dc->_values->speedVal = speed;
  dc->_values->turnVal = turn;
}

void speed_changed_cb (int8_t speed, int16_t realSpeed, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "speed: " << (int)speed << ", realSpeed in cm/s:" << (int)realSpeed;

  DEVICE_CONTROLLER_DATA_T *dc = (DEVICE_CONTROLLER_DATA_T *)pdc;
  boost::lock_guard<boost::mutex> guard(dc->_mtx);
  dc->_values->speedInCmPerSec = realSpeed;
  dc->_values->speedVal = speed;
}

void video_enabled_changed_cb (eARCOMMANDS_JUMPINGSUMO_MEDIASTREAMINGSTATE_VIDEOENABLECHANGED_ENABLED cause, void *pdc) {
  switch(cause) {
    case ARCOMMANDS_JUMPINGSUMO_MEDIASTREAMINGSTATE_VIDEOENABLECHANGED_ENABLED_ENABLED:
      BOOST_LOG_TRIVIAL(trace) << __LINE__ << "video enabled";
      break;
    case ARCOMMANDS_JUMPINGSUMO_MEDIASTREAMINGSTATE_VIDEOENABLECHANGED_ENABLED_DISABLED:
      BOOST_LOG_TRIVIAL(trace) << __LINE__ << "video disabled";
      break;
    case ARCOMMANDS_JUMPINGSUMO_MEDIASTREAMINGSTATE_VIDEOENABLECHANGED_ENABLED_ERROR:
      BOOST_LOG_TRIVIAL(trace) << __LINE__ << "video enabled error";
      break;
    case ARCOMMANDS_JUMPINGSUMO_MEDIASTREAMINGSTATE_VIDEOENABLECHANGED_ENABLED_MAX:
      BOOST_LOG_TRIVIAL(trace) << __LINE__ << "video enabled max";
      break;
    default:
      BOOST_LOG_TRIVIAL(warning) << __LINE__ << "unknown value for cause:" << cause;
      break;
  }
}

void all_states_cb (void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "all states send";
}

void battery_state_changed_cb (uint8_t percent, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "battery charge: " << (unsigned int)percent;

  DEVICE_CONTROLLER_DATA_T *dc = (DEVICE_CONTROLLER_DATA_T *)pdc;
  boost::lock_guard<boost::mutex> guard(dc->_mtx);
  dc->_values->batteryChargePercentage = percent;
}

void network_state_link_quality_state_changed_cb (uint8_t quality, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "link quality: " << (unsigned int)quality;

  DEVICE_CONTROLLER_DATA_T *dc = (DEVICE_CONTROLLER_DATA_T *)pdc;
  boost::lock_guard<boost::mutex> guard(dc->_mtx);
  dc->_values->linkQuality = quality;
}

void register_arcommands_callbacks (DEVICE_CONTROLLER_DATA_T *dcdata)
{
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingPCMDCallback(pcmd_cb, dcdata);
  ARCOMMANDS_Decoder_SetCommonCommonAllStatesCallback(all_states_cb, dcdata);
  ARCOMMANDS_Decoder_SetCommonCommonStateBatteryStateChangedCallback(battery_state_changed_cb, dcdata);
  ARCOMMANDS_Decoder_SetJumpingSumoNetworkStateLinkQualityChangedCallback(network_state_link_quality_state_changed_cb, dcdata);
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingStateSpeedChangedCallback(speed_changed_cb, dcdata);
  ARCOMMANDS_Decoder_SetJumpingSumoMediaStreamingStateVideoEnableChangedCallback(video_enabled_changed_cb, dcdata);
}

void unregister_arcommands_callbacks ()
{
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingPCMDCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetCommonCommonAllStatesCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetCommonCommonStateBatteryStateChangedCallback (NULL, NULL);
  ARCOMMANDS_Decoder_SetJumpingSumoNetworkStateLinkQualityChangedCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingStateSpeedChangedCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetJumpingSumoMediaStreamingStateVideoEnableChangedCallback(NULL, NULL);
}

