#pragma once

#include "gtest/internal/gtest-port.h"

#ifdef GTEST_OS_WINDOWS
//此处不包含winsock.h了
#include <windows.h>
#endif

#ifdef GTEST_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define HANDLE void*
#define UINT32 unsigned int
#define UINT8 unsigned char
#define UIN16 unsigned short
#define SOCKET unsigned int
#define INVALID_SOCKET 0
#define SOCKET_ERROR -1
#endif
/*相关接口*/
namespace testing {
  /*ulLogtype枚举宏定义：消息的打印级别*/
#define DT_SETUP             0        /*全局初始化事件*/
#define DT_TEARDOWN          1        /*全局加载事件*/
#define DT_TESTCASE_BEGIN    2        /*用例开始事件*/
#define DT_TESTCASE_END      3        /*用例结束事件*/

#define DT_MSG_MAGIC_WORD  0x4F4F4F4F



//是下面MsgStruct的消息内容

#define DT_MSG_TRACE_HEADER   \
unsigned int  ulMagicWord;  \
unsigned int  usProdType;   \
unsigned int  usLength     \

#pragma pack(push,1)
//传向socket的消息内容中的消息头
  struct DT_MSG_TRACE_HEADER_SUCT
  {
    DT_MSG_TRACE_HEADER;
  };

  typedef struct
  {
    DT_MSG_TRACE_HEADER;
  }DT_MSG_TRACE_STRU;

  //传向Socket的适应msgtype为字符串类型的消息体
  typedef struct
  {
    DT_MSG_TRACE_HEADER;
    unsigned int   direction;     //发送方向
    unsigned int   protocal;      //协议
    unsigned int   msgFlag;       //消息类型是INT32时为1，为0时INT8 *
    unsigned int   msgTypeLen;    //消息类型的长度 (msgType  == 0) strlen(msgTypeStr) + strlen(reservered)
    unsigned char    msgTypeStr[1]; //消息类型
    unsigned int   msgcontentlength;  //消息内容的长度
    unsigned char    msgcontent[1];     //消息内容
  }DT_MSG_TRACE_DATA_STRU_INT8;

  //传向Socket的适应msgtype为int类型的消息体
  typedef struct
  {
    DT_MSG_TRACE_HEADER;
    unsigned int   direction;      //发送方向
    unsigned int   protocal;       //协议
    unsigned int   msgFlag;        //消息类型是INT32时为1，为0时INT8 *
    unsigned int   msgType;        //消息类型
    unsigned int   msgcontentlength;  //消息内容的长度
    unsigned char    msgcontent[1];     //消息内容
  }DT_MSG_TRACE_DATA_STRU_INT32;


  //msgtype消息类型
#define  OUTER_MSG   0      //外部消息
#define  INNER_MSG   1      //内部消息

//传向Socket的适应NodeB的消息体
  typedef struct
  {
    DT_MSG_TRACE_HEADER;
    unsigned int  msgType;            //内外部消息  0: 外部消息  1:内部消息  RecPId的长度
    unsigned int  SenderCpuId;        //发出的Cpuid,若msgType为内部消息时，SenderCpuId = RecCpuId
    unsigned int  SenderPId;          //发出的pid,
    unsigned int  RecCpuId;           //收到的Cpuid,
    unsigned int  RecPId;             //收到的pid,
    unsigned int  msgcontentlength;   //msgContents长度
    unsigned char   msgContents[1];     //消息内容
  }DT_MSG_TRACE_DATA_STRU_NodeB;

  //传向Socket的适应LTE的消息体
  typedef struct
  {
    DT_MSG_TRACE_HEADER;
    unsigned int  msgType;
    unsigned int  SenderCpuId;        //发出的Cpuid
    unsigned int  SenderPId;          //发出的pid,
    unsigned int  RecCpuId;           //收到的Cpuid,
    unsigned int  RecPId;             //收到的pid,
    unsigned int  msgcontentlength;   //msgContents长度
    unsigned char   msgContents[1];     //消息内容
  }DT_MSG_TRACE_DATA_STRU_LTE;

  //传向Socket的公用的dopra 消息
  typedef struct
  {
    DT_MSG_TRACE_HEADER;
    unsigned int  msgType;
    unsigned int  SenderCpuId;        //发出的Cpuid
    unsigned int  SenderPId;          //发出的pid,
    unsigned int  RecCpuId;           //收到的Cpuid,
    unsigned int  RecPId;             //收到的pid,
    unsigned int  msgcontentlength;   //msgContents长度
    unsigned char   msgContents[1];     //消息内容
  }DT_MSG_TRACE_DATA_STRU_Common;

  //传向Socket的5G产品的Service消息
  typedef struct
  {
    DT_MSG_TRACE_HEADER;
    unsigned int SenderServiceID;      // 服务提供者ID
    unsigned int ReceiverServiceID;    // 服务使用者ID
    unsigned int ServiceType;          // 服务类型ID
    unsigned int ServiceID;             // API或者ABI的ID
    unsigned int MsgContentSize;       // 原始服务消息长度
    unsigned char MsgContents[1];      // 原始消息内容
  } DT_MSG_TRACE_DATA_STRU_SERVICE;
#pragma pack(pop)


  /*MessageStruct,统一的消息头结构*/
  typedef struct
  {
    unsigned int ulMsgHead;          /*消息头，4字节，统一为0xf634f634*/
    unsigned int ulMsgLen;           /*消息总长度(包括消息体)，4字节*/
    unsigned int ulMsgHeadLen;       /*消息头长度，4字节*/
    unsigned int ulVersionId;        /*版本号，4字节，目前为0x00000001*/
    unsigned char  aucdate_time[24];    /*消息发送时间，24字节，字符串如：2009-08-24 12:13:15.999 */
    unsigned int ulReserved;         /*保留字节*/
    unsigned int ulSource;           /*发送的消息类型，值域0-1,具体意义见上面的ulSource宏定义*/
    unsigned int ulLogtype;          /*消息的打印级别，值域0-9,具体意义见上面的ulLogtype宏定义*/
    unsigned int ulMsgBodyLen;       /*消息体长度*/
  } MsgStruct;

#define SERVERPORT        6868
#define LOCALPORT         7878

  /*udpSocket消息通信类定义*/
  /*请不要自己直接使用下面的类，否则会给定位问题带来麻烦*/
  class UdpSocket
  {
  public:
    UdpSocket(void);
    ~UdpSocket(void);

  private:
    UINT32 m_localPort;
    std::string m_strServerIP;
    SOCKET m_hHandle;
    sockaddr_in addrTo;
    bool m_bTraceFlag;
    bool m_bPortTraceFlag;  /*串口开关*/
    bool m_bDocPrintFlag;   /*Doc界面打印开关*/
    std::string m_SuiteName;      /*当前发送消息的测试套名称*/
    std::string m_preSuiteName;   /*上一个测试套名称*/
    std::string m_CaseName;       /*测试用例名称*/
    HANDLE m_FileHandle;          /*记录串口消息的文件句柄*/
    std::string m_fileFolderPath;       /*创建的AutoStar文件夹的路径*/
    bool m_bNewFileFlag;

    int Init_Socket();  /*socket初始化*/
    int CreateSocket(void);   /*创建socket*/
    int MessageSend(UINT32 ulSource, UINT32 ulLogtype, UINT32 usDataLen, UINT8 *poctData);  /*消息发送*/
    int WritePortMessageToTxt(UINT32 ulLogtype, UINT8 *poctData); /*写串口消息到文本*/
    unsigned int DT_MessageHead_INT32(UINT32 ucProt, UINT32 ucDirection, UINT8 *pstrMsgType, UINT32 usDataLen, UINT8 *poctData, UINT8 *pOutData, UINT32 msgFlag);
    unsigned int DT_MessageHead_INT8(UINT32 ucProt, UINT32 ucDirection, UINT8 *pstrMsgType, UINT32 usDataLen, UINT8 *poctData, UINT8 *pOutData, UINT32 msgFlag);
    unsigned int DT_MessageHead_NodeB(UINT32 MsgType, UINT32 SenderCpuId, UINT32 SenderPId, UINT32 RecCpuId, UINT32 RecPId, UINT32 usDataLen, UINT8 *poctData, UINT8 *pOutData);
    unsigned int DT_MessageHead_LTE(UINT32 MsgType, UINT32 SenderCpuId, UINT32 SenderPId, UINT32 RecCpuId, UINT32 RecPId, UINT32 usDataLen, UINT8 *poctData, UINT8 *pOutData);
    unsigned int DT_MessageHead_InnerMsg(UINT32 MsgType, UINT32 SenderCpuId, UINT32 SenderPId, UINT32 RecCpuId, UINT32 RecPId, UINT32 usDataLen, UINT8 *poctData, UINT8 *pOutData);
  public:
    int SocketBind(void);    /*socket绑定*/
    int UdpSendMessage(UINT32 ulSource, UINT32 ulLogtype, UINT32 usDataLen, UINT8 *poctData);   /*消息发送*/
    int ReceiveMessage(UINT8 *recvData);    /*消息接收*/
    int CloseSocket(void);                  /*socket关闭*/
    int SetServerIP(std::string strIP);
    int SetLocalPort(UINT32 localPort);
    int OpenTrace(bool bTrace);   /*消息发送开关*/
    int ComportMsgPrint(bool bFlag);  /*串口打印开关*/
    int SetSuiteCaseName(const char *pSuiteName, const char *pCaseName);
    void DT_SendMessageToTracer_INT32(UINT32 ulSource, UINT32 ucProt, UINT32 ucDirection, UINT32 MsgType, UINT32 usDataLen, UINT8 *poctData);    /*INT32的消息类型的消息发送*/
    void DT_SendMessageToTracer_INT8(UINT32 ulSource, UINT32 ucProt, UINT32 ucDirection, UINT8 *pstrMsgType, UINT32 usDataLen, UINT8 *poctData);  /*字符串的消息类型的消息发送*/
    void DT_SendMessageToTracer_NodeB(UINT32 MsgType, UINT32 SenderCpuId, UINT32 SenderPId, UINT32 RecCpuId, UINT32 RecPId, UINT32 usDataLen, UINT8 *poctData); /*NodeB的消息发送*/
    void DT_SendMessageToTracer_LTE(UINT32 MsgType, UINT32 SenderCpuId, UINT32 SenderPId, UINT32 RecCpuId, UINT32 RecPId, UINT32 usDataLen, UINT8 *poctData); /*NodeB的消息发送*/
    void DT_SendMessageToTracer_InnerMsg(UINT32 MsgType, UINT32 SenderCpuId, UINT32 SenderPId, UINT32 RecCpuId, UINT32 RecPId, UINT32 usDataLen, UINT8 *poctData); /*NodeB的消息发送*/
    void DT_SendMessageToTracer_ServiceMsg(  // 5G产品, 服务消息跟踪
      UINT32 ulSource,
      UINT32 SenderServiceId,
      UINT32 ReceiverServiceId,
      UINT32 ServiceType,  // 0 : ABI  1 : API
      UINT32 ServiceID,  // API ID or ABI ID
      UINT32 usDataLen,
      UINT8 *poctData);

    bool GetPrintDocFlag();	/*获取Doc界面打印开关*/
    void SetPrintDocFlag(bool bFlag);   /*设置Doc界面打印开关*/
    void SetNewFileCreateFlag(bool bFlag); /*设置串口打印重定向到文件时,本进程范围内不覆盖之前的打印*/

  };

}
