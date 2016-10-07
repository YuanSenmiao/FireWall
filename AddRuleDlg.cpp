// AddRuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fire.h"
#include "AddRuleDlg.h"
//********************************************************
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddRuleDlg dialog


CAddRuleDlg::CAddRuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddRuleDlg::IDD, pParent)
{
	m_sdadd = _T("");
	m_sdport = _T("");
	m_ssadd = _T("");
	m_ssport = _T("");
	m_smask = _T("");
	m_smask = _T("");
	ipFltDrv.LoadDriver("DrvFltIp", NULL, NULL, TRUE);	
	IsValidRule = TRUE;
}


void CAddRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddRuleDlg)	
	DDX_Text(pDX, IDC_SMASK, m_smask);
	DDX_Text(pDX, IDC_DMASK, m_dmask);
	DDX_Control(pDX, IDC_COMBO2, m_protocol);
	DDX_Control(pDX, IDC_COMBO1, m_action);
	DDX_Text(pDX, IDC_DADD, m_sdadd);
	DDV_MaxChars(pDX, m_sdadd, 15);
	DDX_Text(pDX, IDC_DPORT, m_sdport);
	DDX_Text(pDX, IDC_SADD, m_ssadd);
	DDV_MaxChars(pDX, m_ssadd, 15);
	DDX_Text(pDX, IDC_SPORT, m_ssport);
	//}}AFX_DATA_MAP
	
}


BEGIN_MESSAGE_MAP(CAddRuleDlg, CDialog)
	//{{AFX_MSG_MAP(CAddRuleDlg)
	ON_EN_KILLFOCUS(IDC_SADD, OnKillfocusSadd)
	ON_EN_KILLFOCUS(IDC_DADD, OnKillfocusDadd)
	ON_BN_CLICKED(IDC_ADDSAVE, OnAddsave)
	ON_EN_KILLFOCUS(IDC_SPORT, OnKillfocusSport)
	ON_EN_KILLFOCUS(IDC_DPORT, OnKillfocusDport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

//**************************************************************************
//添加至IPFilter，发送指令至驱动
DWORD CAddRuleDlg::AddFilter(IPFilter pf)
{

	DWORD result = ipFltDrv.WriteIo(ADD_FILTER, &pf, sizeof(pf));
	if (result != DRV_SUCCESS) 
	{
		AfxMessageBox("Unable to add rule to the driver");
		return FALSE;
	}
	else
		return TRUE;
}

//*************************************************************************
//检测输入的IP地址是否非法
BOOL check(CString str)
{
	CString str2, tmp;
	tmp = str;
	if(str.IsEmpty())
	{
		AfxMessageBox("empty");
		return FALSE;
	}
	else
	{
		int dotCnt = 0;
		tmp += ".";
		while(!tmp.IsEmpty())
		{
			str2=tmp.SpanExcluding(".");
			AfxMessageBox(str2);
			
			int len = str2.GetLength();
			tmp.Delete(0,len+1);

			if(str2.IsEmpty())
			{
				AfxMessageBox("empty substring!");
				return FALSE;
			}
			else
			{		
				if(str2.SpanIncluding(_T("0123456789"))==str2)
				{
					AfxMessageBox("all number!");
					int num = atoi(str2);
					if(num>=0&&num<=255)
						dotCnt++;
				}
				else
				{
					AfxMessageBox("Not all number!");
					return FALSE;
				}
			}
		}
		/*CString cnt;
		cnt.Format(_T("%d"),dotCnt);
		MessageBox(cnt);*/
		if(dotCnt!=4)
			return FALSE;
	}
	return TRUE;
}

BOOL CAddRuleDlg::Verify(CString str)
{
	int result;
	result = inet_addr(str);//借助SockUtil的IP地址转换函数判断
	if(result == -1)
	{
		MessageBox("Invalid ip address!");
		return FALSE;
	}
	return TRUE;
}

//*****************************************************************

//*****************************************************************

void CAddRuleDlg::OnKillfocusSadd() 
{
	CWnd *pWnd = GetFocus(); 
	if(pWnd == NULL);// no focus
	else
	{
		UINT nID = pWnd->GetDlgCtrlID();
		if(nID==((CButton*)GetDlgItem(IDCANCEL))->GetDlgCtrlID())
			return;
	}	

	// TODO: Add your control notification handler code here
	UpdateData();
	BOOL bresult = Verify(m_ssadd); // 检验
	if(bresult == FALSE)
	{	
		IsValidRule = FALSE;
		MessageBox("Invalid IP Address");	
		((CEdit*)GetDlgItem(IDC_SADD))->SetFocus();
		((CEdit*)GetDlgItem(IDC_SADD))->SetSel(0,-1);
	//	((CEdit*)GetDlgItem(IDC_SADD))->SetWindowText("0.0.0.0");
	}
}

void CAddRuleDlg::OnKillfocusDadd() 
{
	CWnd *pWnd = GetFocus(); 
	if(pWnd == NULL);// no focus
	else
	{
		UINT nID = pWnd->GetDlgCtrlID();
		if(nID==((CButton*)GetDlgItem(IDCANCEL))->GetDlgCtrlID())
			return;
		/*CString temp; 
		temp.Format(_T("%u"),nID);
		MessageBox(temp);*/
	}

	// TODO: Add your control notification handler code here
	// This will check wether the IP address you had given
	// corresponds to a valid IP address or not. If not it
	// will prompt you for a valid IP address.
	
	UpdateData();//这是什么函数？？？
    BOOL bresult = Verify(m_sdadd);
	if(bresult == FALSE)
	{
		MessageBox("Invalid IP Address");
		//((CEdit*)GetDlgItem(IDC_DADD))->SetWindowText("0.0.0.0");
		((CEdit*)GetDlgItem(IDC_DADD))->SetFocus();
		((CEdit*)GetDlgItem(IDC_DADD))->SetSel(0,-1);
	}
}

//将新增的规则写入文件
void CAddRuleDlg::OnAddsave() 
{
	// Your code
	UpdateData();
	// 1. 从前端界面获取用户输入的防火墙规则，并转换为驱动使用的格式
	//MessageBox(m_dmask);
	CString protocol, action;
	unsigned short iprotocol;
	BOOLEAN iaction;
	
	// 获取协议号
	int n = m_protocol.GetCurSel();
	if(n==CB_ERR)
	{
		MessageBox("Please select protocol");
		return;
	}
	m_protocol.GetLBText(n,protocol);
	
	if(protocol == "TCP")
		iprotocol = 6;

	else if(protocol == "UDP")
		iprotocol = 17;

	else if(protocol == "ICMP")
		iprotocol = 1;

	else
		iprotocol = 0;

	// 获取Action
	n = m_action.GetCurSel();
	if(CB_ERR == n)
	{
		MessageBox("Select 'allow' or 'deny' as action");
		return;
	}
	m_action.GetLBText(n,action);

	if(action=="ALLOW")
		iaction = FALSE;//0
	else
		iaction = TRUE;

	
	///////////////////////

	// 2.将获取到的规则进行整理后写入文件（文件名为saved.rul，
	//		可借助本文件中提供的文件读写函数进行）

	char   ch1[100];
	wsprintf(ch1,"%s,%s,%s,%s,%s,%s,%d,%d", m_ssadd, "255.255.255.255", m_ssport,
	 m_sdadd, "255.255.255.255", m_sdport, iprotocol, iaction);
	///////////////////////
	bool cover = false;
	int eflag = isExistedRule(ch1, cover);
	if(eflag==0) return;
	else
	{
		if(!cover)
			MessageBox("This rule is already in file");
		else
			replaceRule(eflag);

			//新规则保存到文件中
			NewFile();
			GotoEnd();
			SaveFile(ch1);
			SaveFile("\n");	
			CloseFile();
	}

	// 3. 创建对应的防火墙过滤规则并且写入设备文件（借助AddFilter函数）
	IPFilter pf;
	pf.sourceIp = inet_addr(m_ssadd);// inet_addr((LPCTSTR)m_sdadd);
	pf.destinationIp = inet_addr(m_sdadd);
	
	pf.sourceMask = inet_addr(m_smask);
	pf.destinationMask = inet_addr(m_dmask);
					
	pf.sourcePort	   = htons(atoi(m_ssport));
	pf.destinationPort = htons(atoi(m_sdport));	

	pf.protocol = iprotocol;		
	pf.drop = iaction;	
	AddFilter(pf);
}

BOOL CAddRuleDlg::NewFile(void)
{
/* This will open an existing file or open a new file if the file
   doesnot exists
*/
	_hFile = CreateFile("saved.rul",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_ALWAYS,
						NULL,
						NULL);

	/* If unable to obtain the handle values*/
	if(_hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
		MessageBox("Unable to create or open file!");
	}
	
	return TRUE;		// File has been opened succesfully
}
//******************************************************
// This will move the file pointer to the end of the file so that 
// it can be easily added to the file

DWORD CAddRuleDlg::GotoEnd(void)
{
	DWORD val;

	DWORD size = GetFileSize(_hFile,NULL);
	if(size == 0)
		return size;
	val = SetFilePointer(_hFile,0,NULL,FILE_END);

	/* If unable to set the file pointer to the end of the file */
 	if(val == 0)
	{
		MessageBox("Unable to set file pointer");
		return GetLastError();
	}
	return val;
}

/* This code will save the data into the file which is given by the parameter*/

DWORD CAddRuleDlg::SaveFile(char*  str)
{
	DWORD   bytesWritten;
	/* Try to write the string passed as parameter to the file and if any 
		error occurs return the appropriate values
	*/
	DWORD	_len =  strlen(str);
	if(WriteFile(_hFile, str, _len, &bytesWritten, NULL) == 0)
	{
		MessageBox("Unalbe to write to the file\n");
		return FALSE;
	}
	return TRUE;	
}

/* This function will close the existing file */
BOOL CAddRuleDlg::CloseFile()
{
	if(!_hFile)
	{
		MessageBox("File handle does not exist");
		return FALSE;
	}

// if there is an appropriate handle then close it and return app values
    else{
		if(CloseHandle(_hFile) != 0)
		{
			return TRUE;
		}
		else 
		{
			return FALSE;
			MessageBox("Close file error!");
		}
	}
}

void CAddRuleDlg::OnKillfocusSport() 
{
	CWnd *pWnd = GetFocus(); 
	if(pWnd == NULL);// no focus
	else
	{
		UINT nID = pWnd->GetDlgCtrlID();
		if(nID==((CButton*)GetDlgItem(IDCANCEL))->GetDlgCtrlID())
			return;
	}
	// TODO: Add your control notification handler code here
	//UpdateData();
	//int num = atoi(m_sdport);
	CString txt;
	((CEdit*)GetDlgItem(IDC_SPORT))->GetWindowText(txt);
	long num = atol(txt);

	if(num<0||num>65535)
	{	
		MessageBox("Invalid port number, should be 0~65535");
		((CEdit*)GetDlgItem(IDC_SPORT))->SetFocus();
		((CEdit*)GetDlgItem(IDC_SPORT))->SetSel(0,-1);
		//((CEdit*)GetDlgItem(IDC_SPORT))->SetWindowText("0");
	}
//	MessageBox(m_ssport);
}

void CAddRuleDlg::OnKillfocusDport() 
{
	CWnd *pWnd = GetFocus(); 
	if(pWnd == NULL);// no focus
	else
	{
		UINT nID = pWnd->GetDlgCtrlID();
		if(nID==((CButton*)GetDlgItem(IDCANCEL))->GetDlgCtrlID())
			return;
	}
	//UpdateData();
	CString txt;
	((CEdit*)GetDlgItem(IDC_DPORT))->GetWindowText(txt);
	long num = atol(txt);
	if(num<0||num>65535)
	{
		MessageBox("Invalid port number, should be 0~65535");
		((CEdit*)GetDlgItem(IDC_DPORT))->SetFocus();
		((CEdit*)GetDlgItem(IDC_DPORT))->SetSel(0,-1);
	}
	//MessageBox(m_sdport);
}

//DEL void CAddRuleDlg::OnCancel() 
//DEL {
//DEL 	// TODO: Add extra cleanup here
//DEL 	CDialog::OnCancel();
//DEL }

//DEL void CAddRuleDlg::checkFocus()
//DEL {
//DEL 	CWnd *pWnd = GetFocus(); 
//DEL 	if(pWnd == NULL);
//DEL 	else
//DEL 	{
//DEL 		UINT nID = pWnd->GetDlgCtrlID();
//DEL 		if(nID==((CButton*)GetDlgItem(IDCANCEL))->GetDlgCtrlID())
//DEL 		{
//DEL 			return;
//DEL 		}
//DEL 		/*CString temp; 
//DEL 		temp.Format(_T("%u"),nID);
//DEL 		MessageBox(temp);*/
//DEL 	}
//DEL }

void CAddRuleDlg::setDefault()
{
	if(m_ssadd.IsEmpty())
		m_ssadd = "0.0.0.0";
	if(m_sdadd.IsEmpty())
		m_sdadd = "0.0.0.0";
	if(m_ssport.IsEmpty())
		m_ssport = "0";
	if(m_sdport.IsEmpty())
		m_sdport = "0";
}

int CAddRuleDlg::isExistedRule(char *chArr, bool &cover)
{
	CString _str;//新添加的规则字符串
	_str.Format("%s",chArr);

	CStdioFile file;
	if(!file.Open(_T("saved.rul"),CFile::modeRead))//打开文件saved.rul
	{
		MessageBox("open file error");
		return 0;
	}
	file.SeekToBegin();//从文件头开始

	CString cstr;
	int row = 0; int len;
	cover = false;
	while(file.ReadString(cstr))//逐行读取字符串
	{		
		row++;
		if(_str==cstr)//完全相同，返回行数
			return 0;
		else//不完全相同
		{			
			len = cstr.GetLength();
			if(len == _str.GetLength())
			{	//如果只有最后一个字符（即action）不同
				if(cstr.Left(len-1)==_str.Left(len-1)&&
					cstr.Right(2)!=_str.Right(2))
				{
					cover = true;//cover标记为true，表示需要覆盖
					MessageBox("Cover!");
					return row;
				}
			}
		}
	}
	file.Close();

	return -1;//新添加的规则没有符合以上情况，正常加入文件
}


void CAddRuleDlg::replaceRule(int nrow)
{//删除saved.rul中的第nrow行内容
	CString aa;
	aa.Format(_T("%d"),nrow);
	MessageBox("将覆盖"+aa);
	CString _str;

	//打开saved.rul
	CStdioFile file;
	if(!file.Open(_T("saved.rul"),CFile::modeReadWrite))
	{
		MessageBox("saved.rul: open file error");
		return;
	}
	file.SeekToBegin();

	//新建tmp.rul，modeCreate方式每次清空
	CStdioFile fileTmp;
	if(!fileTmp.Open(_T("tmp.rul"),CFile::modeCreate|CFile::modeReadWrite))
	{
		MessageBox("tmp.rul: open file error");
		return;
	}
	fileTmp.SeekToEnd();
	
	CString cstr;
	int i = 0; 
	while(file.ReadString(cstr))//逐行读取saved.rul中字符串
	{
		MessageBox(cstr);
		i++;
		if(i==nrow)//除了第nrow行
			continue;
		fileTmp.WriteString(cstr+"\n");
	}//复制了除第nrow行以外的所有内容

	file.Close();
	//清空saved.rul：modeCreate模式打开将文件长度置零
	if(!file.Open(_T("saved.rul"),CFile::modeCreate|CFile::modeReadWrite))
	{
		MessageBox("saved.rul: open file error");
		return;
	}
	file.SeekToEnd();
	fileTmp.SeekToBegin();
	//复制tmp.rul里面所有内容到saved.rul
	while(fileTmp.ReadString(cstr))
		file.WriteString(cstr+"\n");
	
	file.Close();//Close方法以保存修改
	fileTmp.Close();
}
