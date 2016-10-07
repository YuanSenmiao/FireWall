// fireView.cpp : implementation of the CFireView class
//

#include "stdafx.h"
#include "fire.h"
#include "fireDoc.h"
#include "fireView.h"
#include <winsock2.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFireView

IMPLEMENT_DYNCREATE(CFireView, CFormView)

BEGIN_MESSAGE_MAP(CFireView, CFormView)
	ON_BN_CLICKED(IDC_ADDRULE, OnAddrule)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_BLOCKPING, OnBlockping)
	ON_BN_CLICKED(IDC_BLOCKALL, OnBlockall)
	ON_BN_CLICKED(IDC_ALLOWALL, OnAllowall)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_VIEWRULES, OnViewrules)
	ON_WM_SHOWWINDOW()
	ON_UPDATE_COMMAND_UI(ID_Start, OnUpdateStart)
	ON_COMMAND(ID_STOP, OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(ID_ALLOWALL, OnUpdateAllowall)
	ON_UPDATE_COMMAND_UI(ID_BLOCKALL, OnUpdateBlockall)
	ON_COMMAND(ID_Start, OnStart)
	ON_COMMAND(ID_BLOCKALL, OnBlockall)
	ON_COMMAND(ID_ALLOWALL, OnAllowall)
	ON_COMMAND(ID_BLOCKPING, OnBlockping)
	ON_UPDATE_COMMAND_UI(ID_BLOCKPING, OnUpdateBlockping)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFireView construction/destruction
//初始化界面状态
CFireView::CFireView()
	: CFormView(CFireView::IDD)
{
	//********************************************************
	m_pBrush = new CBrush;
	ASSERT(m_pBrush);
	m_clrBk = RGB(0x00,0x66,0x99);
	m_clrText = RGB(0xff,0xff,0x00);
	m_pBrush->CreateSolidBrush(m_clrBk);
	m_pColumns = new CStringList;
	ASSERT(m_pColumns);
	_rows = 1;
	start = TRUE;
	block = TRUE;
	allow = TRUE;
	ping = TRUE ;
}

CFireView::~CFireView()
{
	if (m_pBrush)
		delete m_pBrush;
}

void CFireView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESULT, m_cResult);
	DDX_Control(pDX, IDC_VIEWRULES, m_cvrules);
	DDX_Control(pDX, IDC_BLOCKPING, m_cping);
	DDX_Control(pDX, IDC_BLOCKALL, m_cblockall);
	DDX_Control(pDX, IDC_START, m_cstart);
}

BOOL CFireView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	//*****************************************************************
	
	m_filterDriver.LoadDriver("IpFilterDriver", "System32\\Drivers\\IpFltDrv.sys", NULL, TRUE);

	//we don't deregister the driver at destructor
	m_filterDriver.SetRemovable(FALSE);

	//we load the Filter-Hook Driver
	m_ipFltDrv.LoadDriver("DrvFltIp", NULL, NULL, TRUE);
	//****************************************************************
	return CFormView::PreCreateWindow(cs);
}

void CFireView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	m_parent = (CMainFrame*)GetParent();
	ShowHeaders();
}

/////////////////////////////////////////////////////////////////////////////
// CFireView diagnostics

#ifdef _DEBUG
void CFireView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFireView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFireView message handlers

void CFireView::OnAddrule() 
{
	// TODO: Add your control notification handler code here
	m_Addrule.DoModal ();	
}


void CFireView::OnStart() 
{
	CString		_text;
	m_cstart.GetWindowText(_text);
	
	//Start响应事件
	if(_text != "Stop" )
	{
		if(m_ipFltDrv.WriteIo(START_IP_HOOK, NULL, 0) != DRV_ERROR_IO)
		{
			MessageBox("Firewall Started Sucessfully");
			start = FALSE;
			m_cstart.SetWindowText("Stop");
			m_parent ->SetOnlineLed(TRUE);
			m_parent ->SetOfflineLed(FALSE);
		}
	}

	//Stop响应事件
	else
	{
		if(m_ipFltDrv.WriteIo(STOP_IP_HOOK, NULL, 0) != DRV_ERROR_IO)
		{
			MessageBox("Firewall Stopped Succesfully");
			m_cstart.SetWindowText("Start");
			start = TRUE;
			m_parent ->SetOnlineLed(FALSE);
			m_parent ->SetOfflineLed(TRUE);
		}
	}	
}

//禁用所有的ICMP包, 所有目的地址为本主机的ICMP包都block掉
void CFireView::OnBlockping() 
{
	//MessageBox("此功能需要你来实现！");
	// Your code
	IPFilter pf;

	pf.sourceIp = 0;
	pf.sourceMask = 0;
	pf.sourcePort = 0;

	pf.destinationIp = inet_addr("192.168.200.132");
	//pf.destinationIp = 0;
	//pf.destinationMask = 0;
	pf.destinationMask = inet_addr("255.255.255.255");
	pf.destinationPort = 0;		

	pf.protocol = 1;		
	pf.drop = 1;	
	AddFilter(pf);
}

//禁用所有包
void CFireView::OnBlockall() 
{
	//MessageBox("此功能需要你来实现！");
	// Your code
	IPFilter pf;
	pf.sourceIp = 0;
	pf.destinationIp = 0;
	
	pf.sourceMask = 0;
	pf.destinationMask = 0;
					
	pf.sourcePort	   = 0;
	pf.destinationPort = 0;	

	pf.protocol = 0;		
	pf.drop = 1;	
	AddFilter(pf);

}

//启用所有包
void CFireView::OnAllowall() 
{
	//MessageBox("此功能需要你来实现！");
	// Your code
	if(m_ipFltDrv.WriteIo(CLEAR_FILTER, NULL, 0) == DRV_ERROR_IO)
	{
		AfxMessageBox("Error uninstalling rules.");
	}
	else
	{
		m_cResult.DeleteAllItems();
		AfxMessageBox("Clear all rules to the driver successfully!");
	}
}



//使用指定过滤规则
BOOL CFireView::ImplementRule(void)
{	
	ReadFile();
	return TRUE;
}

void CFireView::OnViewrules() 
{
	m_cResult.DeleteAllItems();//删除原有表格
	OnAllowall();//清空驱动的过滤语句
	ImplementRule();//使用指定过滤规则
}


BOOL CFireView::OpenFile()
{
/* This will open an existing file or open a new file if the file
   doesnot exists
*/
	hFile = CreateFile("saved.rul",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_ALWAYS,
						NULL,
						NULL);

	/* If unable to obtain the handle values*/
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;		// File has been opened succesfully
}

DWORD CFireView::ReadFile()
{
	CString cstr;
	CStdioFile file;
	file.Open(_T("saved.rul"),CFile::modeRead);
	file.SeekToBegin();

	IPFilter pf;
	int nRule = 0;
	while(file.ReadString(cstr))
	{	
		CString strGet(_T(""));
		CStringArray strArry;
		int i = 0;
		while (AfxExtractSubString(strGet, cstr, i++, ','))
		{
			if(i==1)
			{
				pf.sourceIp = inet_addr(strGet);// inet_addr((LPCTSTR)m_sdadd);
			//	MessageBox("sourceIp = "+strGet);
			}
			else if(i == 2)
			{
				pf.sourceMask = inet_addr(strGet);
			//	MessageBox("sourceMask = "+strGet);
			}
			else if(i == 3)
			{
				pf.sourcePort	   = htons(atoi(strGet));
			//	MessageBox("sport = "+strGet);
			}
			else if(i==4)
			{
				pf.destinationIp = inet_addr(strGet);
			//	MessageBox("destinationIp = "+strGet);
			}
			else if( i == 5)
			{
				pf.destinationMask = inet_addr(strGet);
			//	MessageBox("destinationMask = "+strGet);
			}
			else if( i == 6)
			{
				pf.destinationPort	   = htons(atoi(strGet));
			//	MessageBox("destinationPort = "+strGet);
			}
			else if( i == 7)
			{
				pf.protocol = atoi(strGet);
			//	MessageBox("protocol = "+strGet);
			}
			else
			{
				pf.drop = atoi(strGet);
			//	MessageBox("drop = "+strGet);
			}
			strArry.Add(strGet);//逗号分隔开的每一个
		//	AfxMessageBox(strGet);
		}
		AddFilter(pf);
		////////////////////

		int d = strArry.GetSize();//8
		CString printStr(_T(""));
		for (int j=0; j<d; j++)
		{
			//MessageBox(strArry.GetAt(j));
			printStr = strArry.GetAt(j);
			if(printStr.IsEmpty())
				printStr = "ANY";
			if(j==6)
			{
				switch(atoi(printStr))
				{
				case 1:
					printStr = "ICMP";
					break;
				case 6:
					printStr = "TCP";
					break;
				case 17:
					printStr = "UDP";
					break;
				case 0:
					printStr = "ANY";
					break;
				default:
					break;
				}
			}
			if(j==7)
			{
				switch(atoi(printStr))
				{
				case 1:
					printStr = "DENY";
					break;
				case 0:
					printStr = "ALLOW";
					break;
				default:
					break;
				}
			}
			AddItem(nRule,j,printStr,-1);// Add Item			
		}
		/////////////////////////////////
		
		nRule++;
	}
	file.Close();

	return TRUE;
}

BOOL CFireView::CloseFile()
{
	if(!hFile)
	{
	//	MessageBox("File handle does not exist");
		return FALSE;
	}

// if there is an appropriate handle then close it and return app values
    else{
		if(CloseHandle(hFile) != 0)
		{
			return TRUE;
		}
		else 
			return FALSE;
	}
}

//将字符串解析为filter特定格式
void CFireView:: ParseToIp(CString str)
{
}


//增加过滤规则表列
BOOL CFireView::AddColumn(LPCTSTR strItem,int nItem,int nSubItem,int nMask,int nFmt)
{
	LV_COLUMN lvc;
	lvc.mask = nMask;
	lvc.fmt = nFmt;
	lvc.pszText = (LPTSTR) strItem;
	lvc.cx = m_cResult.GetStringWidth(lvc.pszText) + 25;
	if(nMask & LVCF_SUBITEM)
	{
		if(nSubItem != -1)
			lvc.iSubItem = nSubItem;
		else
			lvc.iSubItem = nItem;
	}
	return m_cResult.InsertColumn(nItem,&lvc);
}

//增加过滤规则表一个元素
BOOL CFireView::AddItem(int nItem,int nSubItem,LPCTSTR strItem ,int nImageIndex)
{
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nItem;
	lvItem.iSubItem = nSubItem;
	lvItem.pszText = (LPTSTR) strItem;

	if(nImageIndex != -1)
	{
		lvItem.mask |= LVIF_IMAGE;
		lvItem.iImage |= LVIF_IMAGE;
	}
	if(nSubItem == 0)
		return m_cResult.InsertItem(&lvItem);

	return m_cResult.SetItem(&lvItem);
}

void CFireView::AddHeader(LPTSTR hdr)
{
	if (m_pColumns)
		m_pColumns->AddTail(hdr);
}

void CFireView::ShowHeaders()
{
	int nIndex = 0;
	POSITION pos = m_pColumns->GetHeadPosition();
	while (pos)
	{
		CString hdr = (CString)m_pColumns->GetNext(pos);
		AddColumn(hdr,nIndex++);
	}
}

void CFireView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFormView::OnShowWindow(bShow, nStatus);
	AddHeader(_T("Source IP"));
	AddHeader(_T("Source MASK"));
	AddHeader(_T("Source PORT"));
	AddHeader(_T("Dest IP"));
	AddHeader(_T("Dest MASK"));
	AddHeader(_T("Dest PORT"));
	AddHeader(_T("PROTOCOL"));
	AddHeader(_T("ACTION"));
}

void CFireView::OnStop() 
{
	OnStart();	
}

void CFireView::OnUpdateStart(CCmdUI* pCmdUI) 
{	
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(start);	
}

void CFireView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(!start);
}

void CFireView::OnUpdateAllowall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(allow);
}

void CFireView::OnUpdateBlockall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(block);
}

void CFireView::OnUpdateBlockping(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(ping);	
}

BOOL CFireView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}
//***********************************************************************

HBRUSH CFireView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor)
	{
	case CTLCOLOR_BTN:
	case CTLCOLOR_STATIC:
		pDC->SetBkColor(m_clrBk);
		pDC->SetTextColor(m_clrText);
	case CTLCOLOR_DLG:
		return static_cast<HBRUSH>(m_pBrush->GetSafeHandle());
	}
	return CFormView::OnCtlColor(pDC,pWnd,nCtlColor);
}


DWORD CFireView::AddFilter(IPFilter pf)
{
	DWORD result = m_ipFltDrv.WriteIo(ADD_FILTER, &pf, sizeof(pf));
	if (result != DRV_SUCCESS) 
	{
		AfxMessageBox("Unable to add rule to the driver");
		return FALSE;
	}

	else
	{
		//AfxMessageBox("Add rule to the driver successfully!");
		return TRUE;
	}
}
