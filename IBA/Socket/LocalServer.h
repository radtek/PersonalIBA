#pragma once

#include "..\Singleton.h"
#include "ClientSocket.h"
#include "..\ComputerList.h"
#include "..\ActiveMemberList.h"
#include "..\RealName\IZd.h"

class CComputerListView;
class CActiveMemberView;
class CSeatLayoutView;

namespace NS_ISERVER
{

class CLocalServer : private CTimer
{

	SINGLETON_DECLARE(CLocalServer)

public:
	
	static CLocalServer* GetInstance();

	enum EDOING
	{	
		ED_NOTHING,
		ED_CAPTURESCREEN,
		ED_GETLOG
	};

private :
	
	CClientSocket* m_lpSocket;
	CCommandProcessor* m_lpCommandProcessor;
	CBufferProcess* m_lpBufferProcessor;
	
private :

	void InitLocalServer();
	void SendBuffer(const NET_HEAD_MAN* pHead, const void* pBuf = NULL, BOOL bCheckStatus = TRUE);

	int m_CheckInStatus; //��ʼδ����:0	 ����ɹ� 1: ����ʧ��: -1;
	
	EDOING m_CurrentState; 

	CComputerListView* m_lpComputerListView;
	CActiveMemberView* m_lpActiveMemberView;
	CSeatLayoutView* m_lpSeatLayoutView;

	BOOL m_bNeedReportPCInfo;

	virtual void OnTimer(const UINT uTimerID);

public:
	CComputerList ComputerList;
	CActiveMemberList ActiveMemberList;

public :

	CComputerListView* GetComputerListView() const;
	void SetComputerListView(CComputerListView* newVal);

	CActiveMemberView* GetActiveMemberView() const { return m_lpActiveMemberView; }
	void SetActiveMemberView(CActiveMemberView* newVal);

	CSeatLayoutView* GetSeatLayoutView() const { return m_lpSeatLayoutView; }
	void SetSeatLayoutView(CSeatLayoutView* newVal);

	int GetCheckInStatus() const { return m_CheckInStatus; }
	void SetCheckInStatus(int newVal) { m_CheckInStatus = newVal; }

	EDOING GetCurrentState() const { return m_CurrentState; }
	void SetCurrentState(EDOING newVal) { m_CurrentState = newVal; }

	BOOL GetNeedReportPCInfo() const { return m_bNeedReportPCInfo; }
	void SetNeedReportPCInfo(BOOL newVal) { m_bNeedReportPCInfo = newVal; }

	//std::map<CString,UINT> GetPCClassMap() {return m_lpCommandProcessor->m_PCClassMap;}
public :

	BOOL ConnectServer(void);
	BOOL IsConnected();
	BOOL LocalPing();
	void LocalLogin();
	void LocalCheckin();
	void LocalGetComputerList();
	void LocalGetComputerCount();
	void LocalGetMemberClass();
	void LocalGetIP2TermID();
	void LocalGetPCClassName();
	void LocalGetPCClassMap();
	void LocalPingClient(INT_PTR nIndex);
	void LocalCaptureScreen(INT_PTR nIndex);
	void LocalGetLog(INT_PTR nIndex);
	void LocalShutdownAllComputer(BOOL bReboot);
	void LocalShutdownComputer(INT_PTR nIndex, BOOL bReboot);
	void LocalStartupComputer(INT_PTR nIndex);
	void LocalForceCheckOut(INT_PTR nIndex, CString strMsg);
	void LocalSendShortMessage(INT_PTR nIndex, CString& strMsg);
	void LocalSendShortMessageByMemberID(UINT nMemberID, CString& strMsg);
	void LocalSendGroupShortMessage(INT nPCClass, CString& strMsg);
	
	//nType = 1�ǻ�����0����ͨ,2�����»�(���߿ͻ���ֻ����������������)��3֪ͨ�ͻ������µ�¼
	void LocalCheckOutUser(INT_PTR nIndex, UINT nMemberID, UINT nType = 0);
	void LocalCheckOutAllUser();
	void LocalAutoCheckIn(INT_PTR nIndex);
	
	void LocalBundleTime(UINT nMemberID, UINT nBundleTimeId = 9999);
	void LocalRestartServer();

	//Ҫ��ͻ��˲�������
	void LocalForceUpdateData(INT_PTR nIndex);

	//����ͻ����Զ���¼
	void C_CHECKOUT_SUCCEED(INT_PTR nIndex);

	//����
	void LocalUnlockScreen(INT_PTR nIndex, CString strUnlockPwd);

	// 2011/09/08-8201-gxx: ���󹫸���Ϣ����IServer�����Ļ�ȡ�Ĺ�����Ϣ��
	//   bSigned �Ƿ񹫸汻ǩ�գ��ڳɹ����գ�
	void LocalAskNotice(BOOL bSigned=FALSE);

	// 2011/10/27-8210-gxx: 		
	void LocalAuditUserQuit(CString strTermID, CString strIP,UINT nMemberID);

	// 2014-9-29 - qsc
	void LocalLoginZdReturn(ID_LOGINRETURN_ZD *pLogin_zd);
};

}

using namespace NS_ISERVER;