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

#include <boost/log/trivial.hpp>
#include <boost/concept_check.hpp>

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
  
  // FILE *video_out;
  // int _writeImgs;
  // int _frameNb;
};


eARDISCOVERY_ERROR discovery_connection_send_json_Callback (uint8_t *dataTx, uint32_t *dataTxSize, void *customData);
eARDISCOVERY_ERROR discovery_connection_receive_json_callback (uint8_t *dataRx, uint32_t dataRxSize, char *ip, void *customData);
void on_disconnect_network (ARNETWORK_Manager_t *manager, ARNETWORKAL_Manager_t *alManager, void *customData);
void register_arcommands_callbacks (DeviceController *pdc);
void unregister_arcommands_callbacks();

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
  _init_handle();
}

DeviceController::~DeviceController() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::~DeviceController()";

  if (isConnected()) {
    disconnect();
  }
  delete _handle;
}

bool DeviceController::connect() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "DeviceController::connect()";

  register_arcommands_callbacks(this);
  
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
  RACEDRONE_VALUES_T pv;

  pv.alert = rand() % 2;
  pv.batteryChargePercentage = rand() % 100;
  pv.connected = true;
  pv.posture = "normal";
  pv.speedInCmPerSec = rand() % 30;
  pv.speedVal = rand() % 100;
  pv.turnVal = rand() % 100;
  pv.linkQuality = rand() % 7;

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
  // ARSTREAM_Reader_InitStreamDataBuffer (&d2cParams[2], JS_NET_DC_VIDEO_ID, _handle->_arstreamFragSize, _handle->_arstreamFragNb);
  // // optional according to docu ARSTREAM_Reader_InitStreamAckBuffer (&c2dParams[2], JS_NET_CD_VIDEO_ACK_ID);
  
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
  // temporarily without video, thus numC2dParams-1, numD2cParams-1
  _handle->_netManager = ARNETWORK_Manager_New(_handle->_alManager, numC2dParams-1, c2dParams, numD2cParams-1, d2cParams, pingDelay, on_disconnect_network, _handle, &netError);
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
  
  return true;
}

eARNETWORK_MANAGER_CALLBACK_RETURN arnetworkCmdCallback(int buffer_id, uint8_t *data, void *custom, eARNETWORK_MANAGER_CALLBACK_STATUS cause)
{
  eARNETWORK_MANAGER_CALLBACK_RETURN retval = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
  
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "arnetworkCmdCallback bufferId: " << buffer_id << ", cause: " << cause;

  if (cause == ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT) {
      retval = ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP;
  }

  return retval;
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
  
  while (!g_StopFlag) {
    usleep(50000);
    // send_all_states_cmd(dcp->_handle);
    send_PCMD_cmd(dcp->_handle, 1, 10, 0);
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
}

void speed_changed_cb (int8_t speed, int16_t realSpeed, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "speed: " << (int)speed << ", realSpeed in cm/s:" << (int)realSpeed;
}

void all_states_cb (void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "all states send";
}

void battery_state_changed_cb (uint8_t percent, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "battery charge: " << (unsigned int)percent;
}

void network_state_link_quality_state_changed_cb (uint8_t quality, void *pdc) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "link quality: " << (unsigned int)quality;
}

void register_arcommands_callbacks (DeviceController *pdc)
{
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingPCMDCallback(pcmd_cb, pdc);
  ARCOMMANDS_Decoder_SetCommonCommonAllStatesCallback(all_states_cb, pdc);
  ARCOMMANDS_Decoder_SetCommonCommonStateBatteryStateChangedCallback(battery_state_changed_cb, pdc);
  ARCOMMANDS_Decoder_SetJumpingSumoNetworkStateLinkQualityChangedCallback(network_state_link_quality_state_changed_cb, pdc);
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingStateSpeedChangedCallback(speed_changed_cb, pdc);
}

void unregister_arcommands_callbacks ()
{
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingPCMDCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetCommonCommonAllStatesCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetCommonCommonStateBatteryStateChangedCallback (NULL, NULL);
  ARCOMMANDS_Decoder_SetJumpingSumoNetworkStateLinkQualityChangedCallback(NULL, NULL);
  ARCOMMANDS_Decoder_SetJumpingSumoPilotingStateSpeedChangedCallback(NULL, NULL);
}

