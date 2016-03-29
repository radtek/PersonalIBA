#include "stdafx.h"
#include "..\IBA.h"
#include "IBADAL.h"
#include ".\updatedbthread.h"

CUpdateDBThread::CUpdateDBThread(void)
: m_pDbConnInfo(NULL)
{
}

CUpdateDBThread::~CUpdateDBThread(void)
{
}

int CUpdateDBThread::Run()
{
	CoInitialize(NULL);

	if (!CIBADAL::GetInstance()->GetDBConnInfo(m_pDbConnInfo)) return FALSE;

	DoUpdateDB();

	m_pDbConnInfo->Release();
	m_pDbConnInfo = NULL;
	
	CoUninitialize();

	return 0;
}

BOOL CUpdateDBThread::ExecuteSQL(CString strSQL)
{
	TRACE(strSQL);

	BOOL bRet = m_pDbConnInfo->GetConn()->Execute(strSQL);

	if (!bRet)
	{
		CIBALog::GetInstance()->WriteFormat(_T("CUpdateDBThread::ExecuteSQL - %s"), 
			m_pDbConnInfo->GetConn()->GetErrorDescription());
	}

	return bRet;
}

BOOL CUpdateDBThread::TableExists(LPCTSTR lpszTableName)
{
	BOOL bRet = FALSE; 

	CADORecordset Rs(m_pDbConnInfo->GetConn());

	//CString strSQL;

	//strSQL.Format(_T("select TABLE_NAME from `INFORMATION_SCHEMA`.`TABLES` where `TABLE_NAME`='%s'"), lpszTableName);

	if (Rs.Open(_T("SHOW TABLES FROM NETBAR")))
	{
		while (!Rs.IsEOF())
		{
			CString strTmp;

			if (Rs.GetFieldValue(0, strTmp))
			{
				if (0 == strTmp.CompareNoCase(lpszTableName))
				{
					bRet = TRUE;

					break;
				}
			}

			Rs.MoveNext();
		}

		Rs.Close();
	}

	return bRet;
}

BOOL CUpdateDBThread::FieldExists(LPCTSTR lpszTableName, LPCTSTR lpszFieldName)
{
	BOOL bRet = FALSE; 

	CADORecordset Rs(m_pDbConnInfo->GetConn());

	CString strSQL;

	strSQL.Format(_T("Describe %s %s"), lpszTableName, lpszFieldName);

	if (Rs.Open(strSQL))
	{
		bRet = (Rs.GetRecordCount() > 0);

		Rs.Close();
	}

	return bRet;
}

void CUpdateDBThread::DoUpdateDB()
{
	CString strSQL;

	//如果表不存在则创建

	// 2011/07/15-8201-gxx: 本地消费表，保存本地用户结账后的信息
	if (!TableExists(_T("LocalConsume")))
	{
		IBA_LOG0(_T("create table LocalConsume"));

		strSQL.Empty();

		strSQL = _T("CREATE TABLE `LocalConsume` (\
			`submitTime` datetime NOT NULL,\
			`netBarId` smallint(6) NOT NULL default '0',\
			`refNo` int(11) NOT NULL default '0',\
			`serialNo` int(11) default NULL,\
			`serialNum` varchar(20) default NULL,\
			`name` varchar(20) default NULL,\
			`idNumber` varchar(50) default NULL,\
			`memberId` int(11) default NULL,\
			`creditAmount` int(11) default NULL,\
			`consumeAmount` int(11) default NULL,\
			`returnAmount` int(11) default NULL,\
			`checkinTime` datetime default NULL,\
			`checkoutTime` datetime default NULL,");
		strSQL += _T("\
			`termId` varchar(8) default NULL,\
			`ClassId` tinyint(4) default '0',\
			`PayType` tinyint(4) default '0',\
			`PCClass` tinyint(4) default '0',\
			`timeConsume` int(11) default NULL,\
			`operator` varchar(30) default NULL,\
			`dataStatus` tinyint(4) default NULL,\
			`checkData` varchar(32) default NULL,\
			PRIMARY KEY  (`submitTime`,`netBarId`,`refNo`),\
			KEY `lc_serialnum` (`serialNum`),\
			KEY `lc_memberId` (`memberId`),\
			KEY `lc_termId` (`termId`),\
			KEY `lc_name` (`name`),\
			KEY `lc_idnumber` (`idNumber`)\
			) ENGINE=InnoDB DEFAULT CHARSET=gbk");

		ExecuteSQL(strSQL);
	}


	if (!TableExists(_T("MemberUpgrade")))
	{	
		strSQL.Empty();
		
		strSQL += L"CREATE TABLE MemberUpgrade (\r\n";
		strSQL += L"operationTime datetime NOT NULL default '0000-00-00 00:00:00',\r\n";
		strSQL += L"netBarId int(11) NOT NULL default '0',\r\n";
		strSQL += L"refNo int(11) NOT NULL default '0',\r\n";
		strSQL += L"serialNo int(11) default NULL,\r\n";
		strSQL += L"memberId int(11) default NULL,\r\n";
		strSQL += L"oldClassId int(11) default NULL,\r\n";
		strSQL += L"classId int(11) default '0',\r\n";
		strSQL += L"reason varchar(120) default NULL,\r\n";
		strSQL += L"operator varchar(30) default NULL,\r\n";
		strSQL += L"PRIMARY KEY (operationTime,netBarId,refNo)\r\n)";
		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}
	
	if (!TableExists(_T("IDReader")))
	{	
		strSQL.Empty();

		strSQL += L"CREATE TABLE IDReader (\r\n";
		strSQL += L"serialNum int(11) NOT NULL default '0',\r\n";
		strSQL += L"psk varchar(16) NOT NULL default '',\r\n";
		strSQL += L"operationTime datetime default NULL,\r\n";
		strSQL += L"serialNo int(11) default NULL,\r\n";
		strSQL += L"netBarId int(11) default NULL,\r\n";
		strSQL += L"PRIMARY KEY (serialNum)\r\n)";
		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	if (!TableExists(_T("StoreManageVoucher")))
	{	
		strSQL.Empty();

		strSQL += L"CREATE TABLE StoreManageVoucher (\r\n";
		strSQL += L"voucherNo varchar(12) NOT NULL ,\r\n";
		strSQL += L"voucherType int(11) NOT NULL ,\r\n";
		strSQL += L"supplierCode varchar(10) ,\r\n";
		strSQL += L"netBarId int(11) NOT NULL ,\r\n";
		strSQL += L"storeId int(11) NOT NULL ,\r\n";
		strSQL += L"destNetBarId int(11) NOT NULL default '0',\r\n";
		strSQL += L"destStoreId int(11) ,\r\n";
		strSQL += L"refNo int(11) default '0',\r\n";
		strSQL += L"serialNo int(11) ,\r\n";
		strSQL += L"payType int(11) ,\r\n";
		strSQL += L"userId varchar(30) NOT NULL ,\r\n";
		strSQL += L"voucherTime datetime NOT NULL ,\r\n";
		strSQL += L"voucherStatus int(11) ,\r\n";
		strSQL += L"accountYear CHAR(4) ,\r\n";
		strSQL += L"accountMonth CHAR(2) ,\r\n";
		strSQL += L"description varchar(200) ,\r\n";
		strSQL += L"cashRegisterId int(11) ,\r\n";
		strSQL += L"PRIMARY KEY (voucherNo , voucherType)\r\n)";
		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	if (!TableExists(_T("StoreManageDetail")))
	{	
		strSQL.Empty();

		strSQL += L"CREATE TABLE StoreManageDetail (\r\n";
		strSQL += L"voucherNo varchar(12) NOT NULL ,\r\n";
		strSQL += L"voucherType int(11) NOT NULL ,\r\n";
		strSQL += L"commodityCode varchar(16) NOT NULL ,\r\n";
		strSQL += L"price int(11) default '0' ,\r\n";
		strSQL += L"amount int(11) default '0' ,\r\n";
		strSQL += L"quantity int(11) default '0' ,\r\n";

		strSQL += L"PRIMARY KEY (voucherNo , voucherType, commodityCode)\r\n)";
		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	if (!TableExists(_T("NetBarCashier")))
	{	
		strSQL.Empty();

		strSQL += L"CREATE TABLE NetBarCashier (\r\n";
		strSQL += L"netBarId int(11) NOT NULL ,\r\n";
		strSQL += L"userId varchar(30) NOT NULL ,\r\n";
		strSQL += L"userPrivileges varchar(254) ,\r\n";

		strSQL += L"PRIMARY KEY (netBarId, userId)\r\n)";
		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	if (!TableExists(_T("CheckOutInfo")))
	{	
		strSQL.Empty();

		strSQL += L"CREATE TABLE CheckOutInfo (\r\n";
		strSQL += L"terminalId varchar(30) NOT NULL,\r\n";
		strSQL += L"checkOutTime datetime default NULL,\r\n";
		strSQL += L"serialNum varchar(30) NOT NULL,\r\n";
		strSQL += L"className varchar(30) NOT NULL,\r\n";
		strSQL += L"memberId int(11) default NULL,\r\n";
		strSQL += L"classState int(1) NOT NULL default '0',\r\n";
		strSQL += L"classId int(6) NOT NULL default '0',\r\n";
		strSQL += L"balancemoney int(11) NOT NULL default '0',\r\n";
		strSQL += L"guarantyMoney int(11) NOT NULL default '0',\r\n";
		strSQL += L"userName varchar(10) NOT NULL,\r\n";
		strSQL += L"personalId varchar(20) NOT NULL,\r\n";
		strSQL += L"returnState int(1) NOT NULL default '0',\r\n";

		strSQL += L"PRIMARY KEY (serialNum, terminalId)\r\n)";

		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	// 2014-10-15 - qsc

	if (!TableExists(_T("ZdRealName")))
	{	
		strSQL.Empty();

		strSQL += L"CREATE TABLE ZdRealName (\r\n";
		strSQL += L"serialNum varchar(30) NOT NULL,\r\n";
		strSQL += L"terminalId varchar(30) NOT NULL,\r\n";
		strSQL += L"updatetime datetime default NULL,\r\n";
		
		strSQL += L"PRIMARY KEY (serialNum, terminalId)\r\n)";

		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	// 2015-8-28 liyajun 本地版数据库表格
	if (!TableExists(_T("LocalReport")))
	{
		strSQL.Empty();

		strSQL += L"CREATE TABLE LocalReport (\r\n";
		strSQL += L"memberId int(11) default NULL,\r\n";
		strSQL += L"netId varchar(30) NOT NULL,\r\n";
		strSQL += L"password varchar(32) NOT NULL,\r\n";
		strSQL += L"reportTime datetime default NULL,\r\n";
		strSQL += L"localRegister int(2) default NULL,\r\n";

		strSQL += L"PRIMARY KEY (memberId)\r\n)";

		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}
	if (!TableExists(_T("CostRate")))
	{
		strSQL.Empty();

		strSQL += L"CREATE TABLE CostRate (\r\n";
		strSQL += L"pcClass int(11) NOT NULL,\r\n";
		strSQL += L"classId int(11) NOT NULL,\r\n";
		strSQL += L"rate varchar(96) NOT NULL,\r\n";

		strSQL += L"PRIMARY KEY (pcClass,classId)\r\n)";

		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}
	if (!TableExists(_T("LocalMember"))) 
	{
		strSQL.Empty();

		strSQL += L"CREATE TABLE LocalMember (\r\n";
		strSQL += L"memberId int(11) NOT NULL,\r\n";
		strSQL += L"balance int(11) default NULL,\r\n";
		strSQL += L"beforeUsed int(11) default NULL, \r\n";
		strSQL += L"memberName varchar(30) default NULL,\r\n";
		strSQL += L"netId varchar(30) default NULL,\r\n";
		strSQL += L"idNumber varchar(30) default NULL,\r\n";
		strSQL += L"checkinTime datetime default NULL,\r\n";
		strSQL += L"localCheckinTime datetime default NULL,\r\n";
		strSQL += L"isLocalCheckin int(11) default NULL,\r\n";
		strSQL += L"lastUpdateTime int(11) default NULL,\r\n";
		strSQL += L"totalMoney int(11) default NULL,\r\n";
		strSQL += L"pcClass int(11) default NULL,\r\n";
		strSQL += L"classId int(11) default NULL,\r\n";
		strSQL += L"termId varchar(30) default NULL,\r\n";
		strSQL += L"rate varchar(96) default NULL,\r\n";
		strSQL += L"payType int(11) default NULL,\r\n";
		strSQL += L"nextCheckinTime int(11) default NULL,\r\n";
		strSQL += L"checkoutTime datetime default NULL,\r\n";
		strSQL += L"isReturned int(11) default 0,\r\n";

		strSQL += L"PRIMARY KEY (memberId)\r\n)";

		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}
	if (!TableExists(_T("LocalCredit"))) 
	{
		strSQL.Empty();

		strSQL += L"CREATE TABLE LocalCredit (\r\n";
		strSQL += L"memberId int(11) NOT NULL,\r\n";
		strSQL += L"amount int(11) default NULL,\r\n";
		strSQL += L"creditTime datetime NOT NULL,\r\n";
		strSQL += L"checkcode varchar(32) NOT NULL,\r\n";

		strSQL += L"PRIMARY KEY (memberId,creditTime)\r\n)";

		strSQL += L"ENGINE=InnoDB DEFAULT CHARSET=gbk";

		ExecuteSQL(strSQL);
	}

	//为占座位接收微信中间件发送的数据添加表
	if (!TableExists(_T("OccupySeat")))
	{
		strSQL.Empty();

		strSQL = _T("CREATE TABLE OccupySeat (\
					starttime datetime default NULL,\
					endtime datetime default NULL,\
					name varchar(20) NOT NULL,\
					termId varchar(20) default NULL,\
					checkcode varchar(40) default NULL,\
					PRIMARY KEY  (termId))\
					ENGINE=InnoDB DEFAULT CHARSET=gbk");
		ExecuteSQL(strSQL);
	}

	//添加AllowReturn表
	if(!TableExists(_T("AllowReturn")))
	{
		strSQL.Empty();

		strSQL = _T("Create table AllowReturn (\
					userClass int(4) NOT NULL PRIMARY KEY,\
					AllowReturn int(4) NOT NULL,\
					IsMember int(4) NOT NULL)\
					ENGINE=InnoDB DEFAULT CHARSET=gbk");
		ExecuteSQL(strSQL);
	}

	//如果字段不存在则添加

	if (!FieldExists(_T("OccupySeat"),_T("checkcode")))
	{
		strSQL = _T("alter table OccupySeat add checkcode varchar(40)  default NULL");

		ExecuteSQL(strSQL);
	}

	if (!FieldExists(_T("CheckOutInfo"), _T("returnState")))
	{
		strSQL = _T("alter table CheckOutInfo add returnState int(1) unsigned default 0");

		ExecuteSQL(strSQL);
	}

	//liyajun 去掉terminalId主键
	if (FieldExists(_T("CheckOutInfo"), _T("serialNum")))
	{
		strSQL = _T("alter table checkoutinfo  drop primary key ");
		
		ExecuteSQL(strSQL);

		strSQL = _T("alter table checkoutinfo modify serialNum varchar(30) primary key");

		ExecuteSQL(strSQL);

	}
/*********************************************************************************************/
	//CheckoutInfo表为本地模式允许空值
	if (FieldExists(_T("CheckOutInfo"), _T("terminalId")))
	{
		strSQL = _T("alter table CheckOutInfo modify terminalId  varchar(30) NULL ");

		ExecuteSQL(strSQL);
	}

	//if (FieldExists(_T("CheckOutInfo"), _T("serialNum"))) 
	//{
	//	strSQL = _T("alter table CheckOutInfo modify serialNum varchar(30) NULL ");

	//	ExecuteSQL(strSQL);
	//}

	if (FieldExists(_T("CheckOutInfo"), _T("className"))) 
	{
		strSQL = _T("alter table CheckOutInfo modify className  varchar(30) NULL ");

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("classState")))
	{
		strSQL = _T("alter table CheckOutInfo modify classState  int(1) NULL "); 

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("classId")))
	{
		strSQL = _T("alter table CheckOutInfo modify classId  int(6) NULL ");

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("userName")))
	{
		strSQL = _T("alter table CheckOutInfo modify userName  varchar(10) NULL ");

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("balancemoney")))
	{
		strSQL = _T("alter table CheckOutInfo modify balancemoney  int(11) NULL ");

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("guarantyMoney")))
	{
		strSQL = _T("alter table CheckOutInfo modify guarantyMoney  int(11) NULL ");

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("personalId")))
	{
		strSQL = _T("alter table CheckOutInfo modify personalId  varchar(20) NULL ");

		ExecuteSQL(strSQL);
	}
	if (FieldExists(_T("CheckOutInfo"), _T("returnState")))
	{
		strSQL = _T("alter table CheckOutInfo modify returnState  int(1) NULL ");

		ExecuteSQL(strSQL);
	}
/*********************************************************************************************/
	if (!FieldExists(_T("ActiveMember"), _T("remark")))
	{
		strSQL = _T("alter table ActiveMember add remark varchar(200)  default NULL");

		ExecuteSQL(strSQL);
	}

	// 2011/05/19-gxx: 
	if (!FieldExists(_T("ActiveMember"),_T("registerBT")))
	{
		strSQL = _T("alter table ActiveMember add registerBT int(1)  default '0'");

		ExecuteSQL(strSQL);
	}

	// 2011/07/08-8201-gxx: 累计封顶额
	if (!FieldExists(_T("ActiveMember"),_T("dynamicMaxAmount")))
	{
		strSQL = _T("alter table ActiveMember add dynamicMaxAmount int(11)  default '0'");

		ExecuteSQL(strSQL);
	}

	// 2011/07/08-8201-gxx: 是否本地用户
	if (!FieldExists(_T("ActiveMember"),_T("isLocal")))
	{
		strSQL = _T("alter table ActiveMember add isLocal int(1)  default '0'");

		ExecuteSQL(strSQL);
	}

	// 2011/07/08-8201-gxx: 上机密码
	if (!FieldExists(_T("ActiveMember"),_T("password")))
	{
		strSQL = _T("alter table ActiveMember add password varchar(32)  default NULL");

		ExecuteSQL(strSQL);
	}

	// 2011/07/12-8201-gxx: 上网账号别名
	if (!FieldExists(_T("ActiveMember"),_T("netIdAlias")))
	{
		strSQL = _T("alter table ActiveMember add netIdAlias varchar(8) default ''");

		ExecuteSQL(strSQL);
	}
	// 2011/10/25-8230-gxx: 包房房主的用户号
	if (!FieldExists(_T("ActiveMember"),_T("roomMemberId")))
	{
		strSQL = _T("alter table ActiveMember add roomMemberId int(11)  default '0'");

		ExecuteSQL(strSQL);
	}
	// 2012/05/04-8242-gxx: 
	if (!FieldExists(_T("ActiveMember"),_T("scanType")))
	{
		strSQL = _T("alter table ActiveMember add scanType int(4)  default '0'");

		ExecuteSQL(strSQL);
	}


	// 2011/07/13-8201-gxx: 保存收银员密码
	if (!FieldExists(_T("cashregister"),_T("password")))
	{
		strSQL = _T("alter table cashregister add password varchar(32) default ''");

		ExecuteSQL(strSQL);
	}

	// 2011/07/13-8201-gxx: 
	if (FieldExists(_T("systemconfig"),_T("keyValue3")))
	{
		strSQL = _T("alter table systemconfig modify keyValue3 varchar(500)");

		ExecuteSQL(strSQL);
	}

	// 2014-10-29 - qsc 
	if (!FieldExists(_T("ZdRealName"),_T("UserName")))
	{
		strSQL = _T("alter table ZdRealName add  UserName varchar(20)  default NULL");

		ExecuteSQL(strSQL);
	}

	// 2014-11-6 - qsc 换机还没传
	if (!FieldExists(_T("ZdRealName"),_T("HaveChange")))
	{
		strSQL = _T("alter table ZdRealName add HaveChange int(1)  default '0'");

		ExecuteSQL(strSQL);
	}
	//清空一些数据

	//清空用户视图的历史数据

	strSQL = _T("delete from activemember where activationTime is null");
	ExecuteSQL(strSQL);

	strSQL = _T("update activemember set status = 3 where date_add(activationTime, interval 1 day) < now() and status = 0");
	ExecuteSQL(strSQL);

	//清空本地计费收银台记录
	strSQL = _T("delete from systemConfig where keyString='localCounting'");
	ExecuteSQL(strSQL);

	//在配置文件设置的日期清空本地数据库中LocalMember及LocalCredit表的内容
	int nStartDate = GetPrivateProfileInt(_T("CleanupLocalModeData"),_T("ExecuteStartDate"),-1,_T(".\\IBAConfig\\IBA.ini"));
	int nEndDate = GetPrivateProfileInt(_T("CleanupLocalModeData"),_T("ExecuteEndDate"),-1,_T(".\\IBAConfig\\IBA.ini"));
	int nCompeted = GetPrivateProfileInt(_T("CleanupLocalModeData"),_T("Completed"),2,_T(".\\IBAConfig\\IBA.ini"));
	if(-1 == nStartDate || -1==nEndDate || 2 == nCompeted)
	{
		BOOL bSucced1 = WritePrivateProfileString(_T("CleanupLocalModeData"),_T("ExecuteStartDate"),_T("20160218"),_T(".\\IBAConfig\\IBA.ini"));
		BOOL bSucced2 = WritePrivateProfileString(_T("CleanupLocalModeData"),_T("ExecuteEndDate"),_T("20201231"),_T(".\\IBAConfig\\IBA.ini"));
		BOOL bSucced3 = WritePrivateProfileString(_T("CleanupLocalModeData"),_T("Completed"),_T("0"),_T(".\\IBAConfig\\IBA.ini"));
		if(bSucced1&bSucced2&bSucced3)
		{
			nStartDate = 20160218;
			nEndDate = 20201231;
			nCompeted = 0;
		}
	}

	if(FALSE == nCompeted)
	{
		COleDateTime nowTime = COleDateTime::GetCurrentTime();
		CString strNowTime = nowTime.Format(_T("%Y%m%d"));
		int nNowTime = _ttoi(strNowTime.GetString());
		if(nStartDate > 0 && nEndDate > 0)
		{
			if(nEndDate >= nNowTime &&  nStartDate <= nNowTime)
			{
				strSQL = _T("delete from localmember where 1=1");
				ExecuteSQL(strSQL);
				strSQL = _T("delete from localcredit where 1=1");
				ExecuteSQL(strSQL);	
				WritePrivateProfileString(_T("CleanupLocalModeData"),_T("Completed"),_T("1"),_T(".\\IBAConfig\\IBA.ini"));
			}
		}	
	}

	//添加设置好旺宝定时器时间间隔的值
	int nRefreshInterval = GetPrivateProfileInt(_T("HaoWangBao"),_T("RefreshInterval"),-1,_T(".\\IBAConfig\\IBA.ini"));
	if(-1 == nRefreshInterval)
	{
		WritePrivateProfileString(_T("HaoWangBao"),_T("RefreshInterval"),_T("180"),_T(".\\IBAConfig\\IBA.ini"));
	}

	return;
}