// XMLTools.cpp : Defines the initialization routines for the DLL.
//

// notepad++
#include "stdafx.h"
#include "XMLTools.h"
#include "PluginInterface.h"
#include "Scintilla.h"

// dialogs
#include "InputDlg.h"
#include "XPathEvalDlg.h"
#include "SelectFileDlg.h"
#include "MessageDlg.h"
#include "XSLTransformDlg.h"
#include "HowtoUseDlg.h"
#include "Report.h"

// other
#include <stack>
#include <shlwapi.h>
#include <shlobj.h>
#include <stdlib.h>
#include <sstream>
#include <assert.h>

// libxml
#include "LoadLibrary.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>

//#define __XMLTOOLS_DEBUG__

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  Note!
//
//    If this DLL is dynamically linked against the MFC
//    DLLs, any functions exported from this DLL which
//    call into MFC must have the AFX_MANAGE_STATE macro
//    added at the very beginning of the function.
//
//    For example:
//
//    extern "C" BOOL PASCAL EXPORT ExportedFunction()
//    {
//      AFX_MANAGE_STATE(AfxGetStaticModuleState());
//      // normal function body here
//    }
//
//    It is very important that this macro appear in each
//    function, prior to any calls into MFC.  This means that
//    it must appear as the first statement within the 
//    function, even before any object variable declarations
//    as their constructors may generate calls into the MFC
//    DLL.
//
//    Please see MFC Technical Notes 33 and 58 for additional
//    details.
//

// This is the name which will be displayed in Plugins Menu
const TCHAR PLUGIN_NAME[] = TEXT("XML Tools");

const TCHAR localConfFile[] = TEXT("doLocalConf.xml");

// The number of functionality
const int TOTAL_FUNCS = 27;
int nbFunc = TOTAL_FUNCS;

NppData nppData;

// XML Loading status
int libloadstatus = -1;

// INI file support
TCHAR iniFilePath[MAX_PATH];
const TCHAR sectionName[] = TEXT("XML Tools");

// Declaration of functionality (FuncItem) Array
FuncItem funcItem[TOTAL_FUNCS];
bool doCheckXML = false, doValidation = false, doCloseTag = false, doAutoIndent = false, doAttrAutoComplete = false, doAutoXMLType = false;
int menuitemCheckXML = -1, menuitemValidation = -1, menuitemCloseTag = -1, menuitemAutoIndent = -1, menuitemAttrAutoComplete = -1, menuitemAutoXMLType = -1;
std::wstring lastXMLSchema(TEXT(""));

// Here're the declaration my functions ///////////////////////////////////////
void insertXMLCheckTag();
void autoXMLCheck();
void manualXMLCheck();

void insertValidationTag();
void autoValidation();
void manualValidation();

void insertXMLCloseTag();
void closeXMLTag();

void insertTagAutoIndent();
void tagAutoIndent();

void insertAttributeAutoComplete();
void attributeAutoComplete();

void setAutoXMLType();
void insertAutoXMLType();

void prettyPrintXML();
void prettyPrintXMLBreaks();
void prettyPrintText();
void linarizeXML();

void getCurrentXPath();
void evaluateXPath();

void performXSLTransform();

void convertXML2Text();
void convertText2XML();

void commentSelection();
void uncommentSelection();

void aboutBox();
void howtoUse();

void performXMLCheck(int informIfNoError);
void savePluginParams();

///////////////////////////////////////////////////////////////////////////////

void registerShortcut(FuncItem *item, bool enableALT, bool enableCTRL, bool enableSHIFT, unsigned char key) {
  if (!item) return;
  item->_pShKey = new ShortcutKey();
  item->_pShKey->_isAlt = enableALT;
  item->_pShKey->_isCtrl = enableCTRL;
  item->_pShKey->_isShift = enableSHIFT;
  item->_pShKey->_key = key;
}

/**
 * Macro for setting an int into a std::wstring variable
 * @param x The integer value
 * @param s The destination std::wstring
 */
#define int_to_string(x,s) {                     \
  std::ostringstream tmp;                        \
  if (tmp << x) s += tmp.str();                  \
}

// get given lang as string (for debug purposes only)
char* getLangType(LangType lg) {
  if (lg == L_TXT) return "L_TXT";
  if (lg == L_PHP) return "L_PHP";
  if (lg == L_C) return "L_C";
  if (lg == L_CPP) return "L_CPP";
  if (lg == L_CS) return "L_CS";
  if (lg == L_OBJC) return "L_OBJC";
  if (lg == L_JAVA) return "L_JAVA";
  if (lg == L_RC) return "L_RC";
  if (lg == L_HTML) return "L_HTML";
  if (lg == L_XML) return "L_XML";
  if (lg == L_MAKEFILE) return "L_MAKEFILE";
  if (lg == L_PASCAL) return "L_PASCAL";
  if (lg == L_BATCH) return "L_BATCH";
  if (lg == L_INI) return "L_INI";
  if (lg == L_NFO) return "L_NFO";
  if (lg == L_USER) return "L_USER";
  if (lg == L_ASP) return "L_ASP";
  if (lg == L_SQL) return "L_SQL";
  if (lg == L_VB) return "L_VB";
  if (lg == L_JS) return "L_JS";
  if (lg == L_CSS) return "L_CSS";
  if (lg == L_PERL) return "L_PERL";
  if (lg == L_PYTHON) return "L_PYTHON";
  if (lg == L_LUA) return "L_LUA";
  if (lg == L_TEX) return "L_TEX";
  if (lg == L_FORTRAN) return "L_FORTRAN";
  if (lg == L_BASH) return "L_BASH";
  if (lg == L_FLASH) return "L_FLASH";
  if (lg == L_NSIS) return "L_NSIS";
  if (lg == L_TCL) return "L_TCL";
  if (lg == L_LISP) return "L_LISP";
  if (lg == L_SCHEME) return "L_SCHEME";
  if (lg == L_ASM) return "L_ASM";
  if (lg == L_DIFF) return "L_DIFF";
  if (lg == L_CAML) return "L_CAML";
  if (lg == L_ADA) return "L_ADA";
  if (lg == L_VERILOG) return "L_VERILOG";
  if (lg == L_MATLAB) return "L_MATLAB";
  if (lg == L_HASKELL) return "L_HASKELL";
  if (lg == L_INNO) return "L_INNO";
  if (lg == L_SEARCHRESULT) return "L_SEARCHRESULT";
  if (lg == L_CMAKE) return "L_CMAKE";
  if (lg == L_YAML) return "L_YAML";
  if (lg == L_EXTERNAL) return "L_EXTERNAL";
  
  return "";
}

/////////////////////////////////////////////////////////////////////////////
// CXMLToolsApp

BEGIN_MESSAGE_MAP(CXMLToolsApp, CWinApp)
  //{{AFX_MSG_MAP(CXMLToolsApp)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXMLToolsApp construction

CXMLToolsApp::CXMLToolsApp() {
  TCHAR nppPath[MAX_PATH];
  GetModuleFileName(::GetModuleHandle(XMLTOOLS_DLLNAME), nppPath, sizeof(nppPath));
  
  // remove the module name : get plugins directory path
  PathRemoveFileSpec(nppPath);
  
  // cd .. : get npp executable path
  PathRemoveFileSpec(nppPath);

  // chargement de la librairie
  libloadstatus = loadLibXML(nppPath);
  if (libloadstatus < 0) nbFunc = 1;
  
  // Make localConf.xml path
  TCHAR localConfPath[MAX_PATH];
  Report::strcpy(localConfPath, nppPath);
  PathAppend(localConfPath, localConfFile);
  //Report::_printf_err("%s", localConfPath);
  
  // Test if localConf.xml exist
  bool isLocal = (PathFileExists(localConfPath) == TRUE);
  
  if (isLocal) {
	  Report::strcpy(iniFilePath, nppPath);
    PathAppend(iniFilePath, TEXT("XMLToolsExt.ini"));
  } else {
    ITEMIDLIST *pidl;
    SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
    SHGetPathFromIDList(pidl, iniFilePath);
    
    PathAppend(iniFilePath, TEXT("Notepad++\\XMLToolsExt.ini"));
  }
  
  int menuentry = 0;
  for (int i = 0; i < nbFunc; ++i) {
    funcItem[i]._init2Check = false;
  }

  if (!libloadstatus) {
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Enable XML syntax auto-check"));
    funcItem[menuentry]._pFunc = insertXMLCheckTag;
    funcItem[menuentry]._init2Check = (::GetPrivateProfileInt(sectionName, TEXT("doCheckXML"), 0, iniFilePath) != 0);
    doCheckXML = funcItem[menuentry]._init2Check;
    menuitemCheckXML = menuentry;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Check XML syntax now"));
    funcItem[menuentry]._pFunc = manualXMLCheck;
    ++menuentry;

    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Enable auto-validation"));
    funcItem[menuentry]._pFunc = insertValidationTag;
    funcItem[menuentry]._init2Check = doValidation = (::GetPrivateProfileInt(sectionName, TEXT("doValidation"), 0, iniFilePath) != 0);
    doValidation = funcItem[menuentry]._init2Check;
    menuitemValidation = menuentry;
    ++menuentry;

    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Validate now"));
    funcItem[menuentry]._pFunc = manualValidation;
    registerShortcut(funcItem+menuentry, true, true, true, 'M');
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Tag auto-close"));
    funcItem[menuentry]._pFunc = insertXMLCloseTag;
    funcItem[menuentry]._init2Check = doCloseTag = (::GetPrivateProfileInt(sectionName, TEXT("doCloseTag"), 0, iniFilePath) != 0);
    doCloseTag = funcItem[menuentry]._init2Check;
    menuitemCloseTag = menuentry;
    ++menuentry;
  /*
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Tag auto-indent"));
    funcItem[menuentry]._pFunc = insertTagAutoIndent;
    funcItem[menuentry]._init2Check = doAutoIndent = (::GetPrivateProfileInt(sectionName, TEXT("doAutoIndent"), 0, iniFilePath) != 0);
    doAutoIndent = funcItem[menuentry]._init2Check;
    menuitemAutoIndent = menuentry;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Auto-complete attributes"));
    funcItem[menuentry]._pFunc = insertAttributeAutoComplete;
    funcItem[menuentry]._init2Check = doAttrAutoComplete = (::GetPrivateProfileInt(sectionName, TEXT("doAttrAutoComplete"), 0, iniFilePath) != 0);
    doAttrAutoComplete = funcItem[menuentry]._init2Check;
    menuitemAttrAutoComplete = menuentry;
    ++menuentry;*/
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Set XML type automatically"));
    funcItem[menuentry]._pFunc = insertAutoXMLType;
    funcItem[menuentry]._init2Check = doAutoXMLType = (::GetPrivateProfileInt(sectionName, TEXT("doAutoXMLType"), 0, iniFilePath) != 0);
    doAutoXMLType = funcItem[menuentry]._init2Check;
    menuitemAutoXMLType = menuentry;
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Pretty print (XML only)"));
    funcItem[menuentry]._pFunc = prettyPrintXML;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Pretty print (XML only - with line breaks)"));
    registerShortcut(funcItem+menuentry, true, true, true, 'B');
    funcItem[menuentry]._pFunc = prettyPrintXMLBreaks;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Pretty print (Text indent)"));
    funcItem[menuentry]._pFunc = prettyPrintText;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Linarize XML"));
    funcItem[menuentry]._pFunc = linarizeXML;
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Current XML Path"));
    registerShortcut(funcItem+menuentry, true, true, true, 'P');
    funcItem[menuentry]._pFunc = getCurrentXPath;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Evaluate XPath expression"));
    funcItem[menuentry]._pFunc = evaluateXPath;
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("XSL Transformation"));
    funcItem[menuentry]._pFunc = performXSLTransform;
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Convert selection XML to text (<> => &&lt;&&gt;)"));
    funcItem[menuentry]._pFunc = convertXML2Text;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Convert selection text to XML (&&lt;&&gt; => <>)"));
    funcItem[menuentry]._pFunc = convertText2XML;
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Comment selection"));
    registerShortcut(funcItem+menuentry, true, true, true, 'C');
    funcItem[menuentry]._pFunc = commentSelection;
    ++menuentry;
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("Uncomment selection"));
    registerShortcut(funcItem+menuentry, true, true, true, 'R');
    funcItem[menuentry]._pFunc = uncommentSelection;
    ++menuentry;
  
    funcItem[menuentry++]._pFunc = NULL;  //----------------------------------------
  
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("About XML Tools"));
    funcItem[menuentry]._pFunc = aboutBox;
    ++menuentry;
  } else {
    Report::strcpy(funcItem[menuentry]._itemName, TEXT("How to use..."));
    funcItem[menuentry]._pFunc = howtoUse;
    ++menuentry;
  }

  assert(menuentry == nbFunc);

  //Report::_printf_inf("menu entries: %d", menuentry);

  /*Report::_printf_inf("%s\ndoCheckXML: %d %d\ndoValidation: %d %d\ndoCloseTag: %d %d\ndoAutoXMLType: %d %d\nisLocal: %d",
    iniFilePath,
    doCheckXML, funcItem[menuitemCheckXML]._init2Check,
    doValidation, funcItem[menuitemValidation]._init2Check,
    doCloseTag, funcItem[menuitemCloseTag]._init2Check,
    doAutoXMLType, funcItem[menuitemAutoXMLType]._init2Check,
    isLocal);*/
}

CXMLToolsApp::~CXMLToolsApp() {
  savePluginParams();

  // Don't forget to deallocate your shortcut here
  for (int i = 0; i < nbFunc; ++i) {
    if (funcItem[i]._pShKey) delete funcItem[i]._pShKey;
  }
}

void savePluginParams() {
  funcItem[menuitemCheckXML]._init2Check = doCheckXML;
  funcItem[menuitemValidation]._init2Check = doValidation;
  funcItem[menuitemCloseTag]._init2Check = doCloseTag;
  //funcItem[menuitemAutoIndent]._init2Check = doAutoIndent;
  //funcItem[menuitemAttrAutoComplete]._init2Check = doAttrAutoComplete;
  funcItem[menuitemAutoXMLType]._init2Check = doAutoXMLType;

  ::WritePrivateProfileString(sectionName, TEXT("doCheckXML"), doCheckXML?TEXT("1"):TEXT("0"), iniFilePath);
  ::WritePrivateProfileString(sectionName, TEXT("doValidation"), doValidation?TEXT("1"):TEXT("0"), iniFilePath);
  ::WritePrivateProfileString(sectionName, TEXT("doCloseTag"), doCloseTag?TEXT("1"):TEXT("0"), iniFilePath);
  //::WritePrivateProfileString(sectionName, TEXT("doAutoIndent"), doAutoIndent?TEXT("1"):TEXT("0"), iniFilePath);
  //::WritePrivateProfileString(sectionName, TEXT("doAttrAutoComplete"), doAttrAutoComplete?TEXT("1"):TEXT("0"), iniFilePath);
  ::WritePrivateProfileString(sectionName, TEXT("doAutoXMLType"), doAutoXMLType?TEXT("1"):TEXT("0"), iniFilePath);
}

/*
 *--------------------------------------------------
 * The 4 extern functions are mandatory 
 * They will be called by Notepad++ plugins system 
 *--------------------------------------------------
*/

// The setInfo function gets the needed infos from Notepad++ plugins system
extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData) {
  nppData = notpadPlusData;
}

// The getName function tells Notepad++ plugins system its name
extern "C" __declspec(dllexport) const TCHAR * getName() {
  return PLUGIN_NAME;
}

// The getFuncsArray function gives Notepad++ plugins system the pointer FuncItem Array 
// and the size of this array (the number of functions)
extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF) {
  *nbF = nbFunc;
  return funcItem;
}

// For v.3.3 compatibility
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) {
  return TRUE;
}

HWND getCurrentHScintilla(int which) {
  return (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
};

// If you don't need get the notification from Notepad++,
// just let it be empty.
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode) {
  if (libloadstatus != 0) return;

  switch (notifyCode->nmhdr.code) {
    case NPPN_READY: {
      HMENU hMenu = ::GetMenu(nppData._nppHandle);
      if (hMenu) {
        ::CheckMenuItem(hMenu, funcItem[menuitemCheckXML]._cmdID, MF_BYCOMMAND | (doCheckXML?MF_CHECKED:MF_UNCHECKED));
        ::CheckMenuItem(hMenu, funcItem[menuitemValidation]._cmdID, MF_BYCOMMAND | (doValidation?MF_CHECKED:MF_UNCHECKED));
        ::CheckMenuItem(hMenu, funcItem[menuitemCloseTag]._cmdID, MF_BYCOMMAND | (doCloseTag?MF_CHECKED:MF_UNCHECKED));
        //::CheckMenuItem(hMenu, funcItem[menuitemAutoIndent]._cmdID, MF_BYCOMMAND | (doAutoIndent?MF_CHECKED:MF_UNCHECKED));
        //::CheckMenuItem(hMenu, funcItem[menuitemAttrAutoComplete]._cmdID, MF_BYCOMMAND | (doAttrAutoComplete?MF_CHECKED:MF_UNCHECKED));
        ::CheckMenuItem(hMenu, funcItem[menuitemAutoXMLType]._cmdID, MF_BYCOMMAND | (doAutoXMLType?MF_CHECKED:MF_UNCHECKED));
      }
    }
    case NPPN_FILEBEFORESAVE: {
      LangType docType;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);
      if (docType == L_XML) {
        // comme la validation XSD effectue �galement un check de syntaxe, on n'ex�cute
        // le autoXMLCheck() que si doValidation est FALSE et doCheckXML est TRUE.
        if (doValidation) autoValidation();
        else if (doCheckXML) autoXMLCheck();
      }
      break;
    }
    case SCN_CHARADDED: {
      LangType docType;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);
      if (docType == L_XML) {
        // remarque: le closeXMLTag doit s'ex�cuter avant les autres
        if (doCloseTag && notifyCode->ch == '>') closeXMLTag();
        //if (doAutoIndent && notifyCode->ch == '\n') tagAutoIndent();
        //if (doAttrAutoComplete && notifyCode->ch == '\"') attributeAutoComplete();
      }
      break;
    }
    case NPPN_FILEOPENED: {
      // si le fichier n'a pas de type d�fini et qu'il commence par "<?xml ", on lui attribue le type L_XML
      LangType docType = L_EXTERNAL;
      ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);
      //Report::_printf_inf("%s", getLangType(docType));
      if (doAutoXMLType && docType == L_TXT) setAutoXMLType();
      break;
    }
  }
}

#ifdef UNICODE
	extern "C" __declspec(dllexport) BOOL isUnicode() {
		return TRUE;
	}
#endif //UNICODE


void insertXMLCheckTag() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("insertXMLCheckTag()");
  #endif
  doCheckXML = !doCheckXML;
  ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuitemCheckXML]._cmdID, MF_BYCOMMAND | (doCheckXML?MF_CHECKED:MF_UNCHECKED));
  savePluginParams();
}

void insertValidationTag() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("insertValidationTag()");
  #endif
  doValidation = !doValidation;
  ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuitemValidation]._cmdID, MF_BYCOMMAND | (doValidation?MF_CHECKED:MF_UNCHECKED));
  savePluginParams();
}

void insertXMLCloseTag() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("insertXMLCloseTag()");
  #endif
  doCloseTag = !doCloseTag;
  ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuitemCloseTag]._cmdID, MF_BYCOMMAND | (doCloseTag?MF_CHECKED:MF_UNCHECKED));
  savePluginParams();
}

bool tagAutoIndentWarningDisplayed = false;
void insertTagAutoIndent() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("insertTagAutoIndent()");
  #endif
  if (!tagAutoIndentWarningDisplayed) {
    Report::_printf_inf(TEXT("This function is in alpha state and might disappear in future release."));
    tagAutoIndentWarningDisplayed = true;
  }

  doAutoIndent = !doAutoIndent;
  ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuitemAutoIndent]._cmdID, MF_BYCOMMAND | (doAutoIndent?MF_CHECKED:MF_UNCHECKED));
  savePluginParams();
}

bool insertAttributeAutoCompleteWarningDisplayed = false;
void insertAttributeAutoComplete() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("insertAttributeAutoComplete()");
  #endif
  if (!insertAttributeAutoCompleteWarningDisplayed) {
    Report::_printf_inf(TEXT("This function is in alpha state and might disappear in future release."));
    insertAttributeAutoCompleteWarningDisplayed = true;
  }
  
  doAttrAutoComplete = !doAttrAutoComplete;
  ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuitemAttrAutoComplete]._cmdID, MF_BYCOMMAND | (doAttrAutoComplete?MF_CHECKED:MF_UNCHECKED));
  savePluginParams();
}

void insertAutoXMLType() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("insertAutoXMLType()");
  #endif
  doAutoXMLType = !doAutoXMLType;
  ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuitemAutoXMLType]._cmdID, MF_BYCOMMAND | (doAutoXMLType?MF_CHECKED:MF_UNCHECKED));
  savePluginParams();
}

void aboutBox() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("aboutBox()");
  #endif
  //Report::_printf_inf("%s \r\n \r\n- libXML %s \r\n- libXSTL %s", XMLTOOLS_ABOUTINFO, LIBXML_DOTTED_VERSION, LIBXSLT_DOTTED_VERSION);
  Report::_printf_inf(TEXT(XMLTOOLS_ABOUTINFO));
}

void howtoUse() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("howtoUse()");
  #endif
  CHowtoUseDlg* dlg = new CHowtoUseDlg();
  dlg->DoModal();
}

///////////////////////////////////////////////////////////////////////////////

void performXMLCheck(int informIfNoError) {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("performXMLCheck()");
  #endif
  int currentEdit, currentLength;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  currentLength = (int) ::SendMessage(hCurrentEditView, SCI_GETLENGTH, 0, 0);

  char *data = new char[currentLength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', currentLength+1);

  TextRange tr;
  tr.chrg.cpMin = 0;
  tr.chrg.cpMax = currentLength;
  tr.lpstrText = data;

  ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));

  xmlDocPtr doc;

  doc = pXmlReadMemory(data, currentLength, "noname.xml", NULL, 0);

  if (doc == NULL) {
    xmlErrorPtr err;
    err = pXmlGetLastError();

    if (err != NULL) {
      if (err->line > 0)
        ::SendMessage(hCurrentEditView, SCI_GOTOLINE, err->line-1, 0);

      Report::_printf_err(TEXT("XML Parsing error at line %d: \r\n:s"), err->message);
    } else {
      Report::_printf_err(TEXT("Failed to parse document"));
    }
  } else if (informIfNoError) {
    Report::_printf_inf(TEXT("No error detected."));
  }

  pXmlFreeDoc(doc);

  tr.lpstrText = NULL;
  delete [] data;
  data = NULL;
}

void autoXMLCheck() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("autoXMLCheck()");
  #endif
  performXMLCheck(0);
}

void manualXMLCheck() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("manualXMLCheck()");
  #endif
  performXMLCheck(1);
}

///////////////////////////////////////////////////////////////////////////////

void XMLValidation(int informIfNoError) {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("XMLValidation()");
  #endif
  // 1. On valide le XML
  bool abortValidation = false;
  std::wstring xml_schema(TEXT(""));
  int currentEdit, currentLength;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  currentLength = (int) ::SendMessage(hCurrentEditView, SCI_GETLENGTH, 0, 0);
  
  char *data = new char[currentLength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', currentLength+1);
  
  TextRange tr;
  tr.chrg.cpMin = 0;
  tr.chrg.cpMax = currentLength;
  tr.lpstrText = data;
  
  ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
  
  xmlDocPtr doc;
  xmlNodePtr rootnode;
  xmlSchemaPtr schema;
  xmlSchemaValidCtxtPtr vctxt;
  xmlSchemaParserCtxtPtr pctxt;
  xmlDtdPtr dtdPtr;
  bool doFreeDTDPtr = false;
  bool xsdValidation = false;
  bool dtdValidation = false;
  
  doc = pXmlReadMemory(data, currentLength, "noname.xml", NULL, 0);
  
  if (doc == NULL) {
    xmlErrorPtr err;
    err = pXmlGetLastError();
    
    if (err != NULL) {
      if (err->line > 0)
        ::SendMessage(hCurrentEditView, SCI_GOTOLINE, err->line-1, 0);
      
      Report::_printf_err(TEXT("XML Parsing error at line %d: \r\n%s"), err->message);
    } else {
      Report::_printf_err(TEXT("Failed to parse document"));
    }

    abortValidation = true;
  }

  // 2. Si le XMl est valide
  if (!abortValidation) {
    // 2.1. On essaie de retrouver le sch�ma ou la dtd das le document
    rootnode = pXmlDocGetRootElement(doc);
    if (rootnode == NULL) {
      Report::_printf_err(TEXT("Empty XML document"));
    } else {
      // 2.1.a. On recherche l'attribut "xsi:noNamespaceSchemaLocation" dans la balise root
      // Exemple de balise root:
      //  <descript xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="Descript_Shema.xsd">
      xmlChar* propval = pXmlGetProp(rootnode, reinterpret_cast<const unsigned char*>("noNamespaceSchemaLocation"));
      if (propval) {
        xml_schema = std::wstring(reinterpret_cast<const TCHAR*>(propval));
        xsdValidation = true;
      }

      // 2.1.b On recheche un DOCTYPE avant la balise root
      dtdPtr = doc->intSubset;
      if (dtdPtr) {
        std::wstring dtd_filename(TEXT(""));
        if (dtdPtr->SystemID) {
          dtd_filename = std::wstring(reinterpret_cast<const TCHAR*>(dtdPtr->SystemID));
        } else if (dtdPtr->ExternalID) {
          dtd_filename = std::wstring(reinterpret_cast<const TCHAR*>(dtdPtr->ExternalID));
        }
        
        dtdPtr = pXmlParseDTD(dtdPtr->ExternalID, dtdPtr->SystemID);
        doFreeDTDPtr = true;

        if (dtdPtr == NULL) {
          Report::_printf_err(TEXT("Unable to load the DTD\r\n%s"),dtd_filename.c_str());
          abortValidation = true;
        } else {
          dtdValidation = true;
        }
      }
    }

    // V�rification des �l�ments: si on a � la fois une DTD et un sch�ma, on prend le sch�ma
    if (xsdValidation) dtdValidation = false;
  }
  
  if (!abortValidation) {
    // 2.2. Si l'attribut est absent, on demande � l'utilisateur de fournir le chemin du fichier XSD
    if (xml_schema.length() == 0 && !dtdValidation) {
      CSelectFileDlg* dlg = new CSelectFileDlg();
      dlg->m_sSelectedFilename = lastXMLSchema.c_str();

      CString rootSample = "<";
      rootSample += reinterpret_cast<const char*>(rootnode->name);
      rootSample += "\r\n\txmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
      rootSample += "\r\n\txsi:noNamespaceSchemaLocation=\"XSD_FILE_PATH\">";

      dlg->m_sRootElementSample = rootSample;
      if (dlg->DoModal() == IDOK) {
        xml_schema = dlg->m_sSelectedFilename;
      }
    }

    // 2.3.a. On proc�de � la validation par sch�ma
    if (xml_schema.length() > 0 && !dtdValidation) {
      if ((pctxt = pXmlSchemaNewParserCtxt(reinterpret_cast<const char*>(xml_schema.c_str()))) == NULL) {
        Report::_printf_err(TEXT("Unable to initialize parser."));
      } else {
        // Chargement du contenu du XML Schema
        schema = pXmlSchemaParse(pctxt);
        pXmlSchemaFreeParserCtxt(pctxt);
        if (schema == NULL) {
          Report::_printf_err(TEXT("Unable to parse schema file."));
        } else {
          // Cr�ation du contexte de validation
          if ((vctxt = pXmlSchemaNewValidCtxt(schema)) == NULL) {
            pXmlSchemaFree(schema);
            Report::_printf_err(TEXT("Unable to create validation context."));
          } else {
            // Traitement des erreurs de validation
            Report::clearLog();
            Report::registerMessage(NULL, TEXT("Validation of current file using XML schema:\r\n\r\n"));
            pXmlSchemaSetValidErrors(vctxt, (xmlSchemaValidityErrorFunc) Report::registerError, (xmlSchemaValidityWarningFunc) Report::registerWarn, stderr);

            // Validation
            if (!pXmlSchemaValidateDoc(vctxt, doc)) {
              if (informIfNoError) Report::_printf_inf(TEXT("XML Schema validation:\r\nXML is valid."));
            } else {
              CMessageDlg* msgdlg = new CMessageDlg();
              msgdlg->m_sMessage = Report::getLog();
              msgdlg->DoModal();
            }
          }

          // 2.4. On lib�re le parseur
          pXmlSchemaFree(schema);
          pXmlSchemaFreeValidCtxt(vctxt);
        }
      }
    }

    // 2.3.b On proc�de � la validation par DTD
    if (dtdPtr && dtdValidation) {
      xmlValidCtxtPtr vctxt;

      // Cr�ation du contexte de validation
      if ((vctxt = pXmlNewValidCtxt())) {
        // Affichage des erreurs de validation
        Report::clearLog();
        Report::registerMessage(NULL, TEXT("Validation of current file using DTD:\r\n\r\n"));
        vctxt->userData = (void *) stderr;
        vctxt->error = (xmlValidityErrorFunc) Report::registerError;
        vctxt->warning = (xmlValidityWarningFunc) Report::registerWarn;

        // Validation
        if (pXmlValidateDtd(vctxt, doc, dtdPtr)) {
          if (informIfNoError) Report::_printf_inf(TEXT("DTD validation:\r\nXML is valid."));
        } else {
          CMessageDlg* msgdlg = new CMessageDlg();
          msgdlg->m_sMessage = Report::getLog();
          msgdlg->DoModal();
        }
        // Lib�ration de la m�moire
        pXmlFreeValidCtxt(vctxt);
      }
      if (doFreeDTDPtr) pXmlFreeDtd(dtdPtr);
    }
  }

  // 3. On lib�re la m�moire
  pXmlFreeDoc(doc);
  tr.lpstrText = NULL;
  delete [] data;
  data = NULL;

  // 4. On enregistre le nom du sch�ma utilis� pour le proposer la prochaine fois
  lastXMLSchema = xml_schema;
}

void autoValidation() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("autoValidation()");
  #endif
  XMLValidation(0);
}

void manualValidation() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("manualValidation()");
  #endif
  XMLValidation(1);
}

///////////////////////////////////////////////////////////////////////////////

void closeXMLTag() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("closeXMLTag()");
  #endif
  char buf[512];
  int currentEdit;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);    
  int currentPos = int(::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, 0));
  int beginPos = currentPos - (sizeof(buf) - 1);
  int startPos = (beginPos > 0)?beginPos:0;
  int size = currentPos - startPos;
  int insertStringSize = 2;
  char insertString[516] = "</";

  if (size >= 3) {
    struct TextRange tr = {{startPos, currentPos}, buf};
    ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

    if (buf[size-2] != '/') {
      const char* pBegin = &buf[0];
      const char* pCur = &buf[size - 2];
      int insertStringSize = 2;

      for (; pCur > pBegin && *pCur != '<' && *pCur != '>' ;) --pCur;

      if (*pCur == '<') {
        ++pCur;

        while (StrChr(TEXT(":_-."), *pCur) || IsCharAlphaNumeric(*pCur)) {
          insertString[insertStringSize++] = *pCur;
          ++pCur;
        }
      }

      insertString[insertStringSize++] = '>';
      insertString[insertStringSize] = '\0';

      if (insertStringSize > 3) {
        ::SendMessage(hCurrentEditView, SCI_BEGINUNDOACTION, 0, 0);
        ::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, (LPARAM)insertString);
        ::SendMessage(hCurrentEditView, SCI_SETSEL, currentPos, currentPos);
        ::SendMessage(hCurrentEditView, SCI_ENDUNDOACTION, 0, 0);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void tagAutoIndent() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("tagAutoIndent()");
  #endif
  // On n'indente que si l'on est dans un noeud (au niveau de l'attribut ou
  // au niveau du contenu. Donc on recherche le dernier < ou >. S'il s'agit
  // d'un >, on regarde qu'il n'y ait pas de / avant (sinon on se retrouve
  // au m�me niveau et il n'y a pas d'indentation � faire)
  // Si le dernier symbole que l'on trouve est un <, alors on indente.

  char buf[512];
  int currentEdit;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);    
  int currentPos = int(::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, 0));
  int beginPos = currentPos - (sizeof(buf) - 1);
  int startPos = (beginPos > 0)?beginPos:0;
  int size = currentPos - startPos;
  
  struct TextRange tr = {{startPos, currentPos}, buf};
  ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

  int tabwidth = ::SendMessage(hCurrentEditView, SCI_GETTABWIDTH, 0, 0);
  int usetabs = ::SendMessage(hCurrentEditView, SCI_GETUSETABS, 0, 0);
  if (tabwidth <= 0) tabwidth = 4;

  bool ignoreIndentation = false;
  if (size >= 1) {
    const char* pBegin = &buf[0];
    const char* pCur = &buf[size - 1];
  
    for (; pCur > pBegin && *pCur != '>' ;) --pCur;
    if (pCur > pBegin) {
      if (*(pCur-1) == '/') ignoreIndentation = true;  // si on a "/>", on abandonne l'indentation
      // maintenant, on recherche le <
      while (pCur > pBegin && *pCur != '<') --pCur;
      if (*pCur == '<' && *(pCur+1) == '/') ignoreIndentation = true; // si on a "</", on abandonne aussi
        
      int insertStringSize = 0;
      char insertString[516] = { '\0' };

      --pCur;
      // on r�cup�re l'indentation actuelle
      while (pCur > pBegin && *pCur != '\n' && *pCur != '\r') {
        if (*pCur == '\t') insertString[insertStringSize++] = '\t';
        else insertString[insertStringSize++] = ' ';

        --pCur;
      }

      // et on ajoute une indentation
      if (!ignoreIndentation) {
        if (usetabs) insertString[insertStringSize++] = '\t';
        else {
          for (int i = 0; i < tabwidth; ++i) insertString[insertStringSize++] = ' ';
        }
      }

      currentPos += insertStringSize;

      // on a trouv� le <, il reste � ins�rer une indentation apr�s le curseur
      ::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, (LPARAM)insertString);
      ::SendMessage(hCurrentEditView, SCI_SETSEL, currentPos, currentPos);  
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void attributeAutoComplete() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf(TEXT("attributeAutoComplete()"));
  #endif
  Report::_printf_inf(TEXT("attributeAutoComplete()"));
}

///////////////////////////////////////////////////////////////////////////////

void setAutoXMLType() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("setAutoXMLType()");
  #endif
  int currentEdit;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  
  // on r�cup�re les 6 premiers caract�res du fichier
  char head[8] = { '\0' };
  ::SendMessage(hCurrentEditView, SCI_GETTEXT, 7, reinterpret_cast<LPARAM>(&head));
  ::SendMessage(hCurrentEditView, SCI_SETSEL, 0, 0);
  
  if (strlen(head) >= 6 && !strcmp(head, "<?xml ")) {
    LangType newType = L_XML;
    ::SendMessage(nppData._nppHandle, NPPM_SETCURRENTLANGTYPE, 0, (LPARAM) newType);
  }
}

///////////////////////////////////////////////////////////////////////////////

void getCurrentXPath() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("getCurrentXPath()");
  #endif
  int currentEdit, currentLength;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  currentLength = (int) ::SendMessage(hCurrentEditView, SCI_GETLENGTH, 0, 0);

  char *data = new char[currentLength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', currentLength+1);

  int currentPos = int(::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, 0));

  TextRange tr;
  tr.chrg.cpMin = 0;
  tr.chrg.cpMax = currentLength;
  tr.lpstrText = data;

  ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));

  std::string str(data);

  // end tag pos
  int begpos = str.find_first_of("<");
  int endpos = str.find_last_of(">");

  // let's reach the end of current tag (if we are inside a tag)
  std::wstring tmpmsg(TEXT("Current node cannot be resolved."));
  if (currentPos > begpos && currentPos <= endpos){
    currentPos = str.find_last_of("<>", currentPos-1)+1;
    bool isinsideclosingtag = (currentPos > 0 && str.at(currentPos-1) == '<' && str.at(currentPos) == '/');

    if (isinsideclosingtag) {
      // if we are inside closing tag (inside </x>, let's go back before '<' char so we are inside node)
      --currentPos;
    } else {
      // let's get the end of current tag or text
      currentPos = str.find_first_of("<>", currentPos);
      // if inside a auto-closing tag (ex. '<x/>'), let's go back before '/' char, the '>' is added before slash)
      if (currentPos > 0 && str.at(currentPos-1) == '/'
                         && str.at(currentPos)   == '>') --currentPos;
    }

    str.erase(currentPos);
    str += "><X>";

    xmlDocPtr doc = pXmlReadMemory(str.c_str(), str.length(), "noname.xml", NULL, XML_PARSE_RECOVER);
    xmlNodePtr cur_node = pXmlDocGetRootElement(doc);

    std::wstring nodepath(TEXT(""));
    while (cur_node != NULL && cur_node->last != NULL) {
      if (cur_node->type == XML_ELEMENT_NODE) {
        nodepath += TEXT("/");
        nodepath += std::wstring(reinterpret_cast<const TCHAR*>(cur_node->name));
      }
      cur_node = cur_node->last;
    }

    pXmlFreeDoc(doc);

    if (nodepath.length() > 0) {
      tmpmsg = nodepath + TEXT("\n\n(Path has been copied into clipboard)");
      
      ::OpenClipboard(NULL);
      ::EmptyClipboard();
      HGLOBAL hClipboardData;
      hClipboardData = GlobalAlloc(GMEM_DDESHARE, nodepath.length()+1);
      char * pchData;
      pchData = (char*)GlobalLock(hClipboardData);
      Report::strcpy(pchData, nodepath.c_str());
      ::GlobalUnlock(hClipboardData);
      ::SetClipboardData(CF_TEXT, pchData);
      ::CloseClipboard();
    }
  }

  Report::_printf_inf(tmpmsg.c_str());
    
  delete [] data;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

int  execute_xpath_expression(const xmlChar* xpathExpr, const xmlChar* nsList);
int  register_namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList);
void print_xpath_nodes(xmlNodeSetPtr nodes);

void evaluateXPath() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("evaluateXPath()");
  #endif
  CXPathEvalDlg *pDlg = new CXPathEvalDlg();
  pDlg->Create(CXPathEvalDlg::IDD,NULL);
  pDlg->ShowWindow(SW_SHOW);
}

#else
void evaluateXPath() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("evaluateXPath()");
  #endif
  Report::_printf_err("Function not available.");
}

#endif

///////////////////////////////////////////////////////////////////////////////

void performXSLTransform() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("performXSLTransform()");
  #endif
  CXSLTransformDlg *pDlg = new CXSLTransformDlg();
  pDlg->Create(CXSLTransformDlg::IDD,NULL);
  pDlg->ShowWindow(SW_SHOW);
}

///////////////////////////////////////////////////////////////////////////////

void prettyPrint(bool autoindenttext, bool addlinebreaks) {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("prettyPrint()");
  #endif
  int currentEdit, currentLength, isReadOnly, xOffset, yOffset;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);

  isReadOnly = (int) ::SendMessage(hCurrentEditView, SCI_GETREADONLY, 0, 0);
  if (isReadOnly) return;
  
  xOffset = (int) ::SendMessage(hCurrentEditView, SCI_GETXOFFSET, 0, 0);
  yOffset = (int) ::SendMessage(hCurrentEditView, SCI_GETFIRSTVISIBLELINE, 0, 0);

  currentLength = (int) ::SendMessage(hCurrentEditView, SCI_GETLENGTH, 0, 0);

  char *data = new char[currentLength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', currentLength+1);

  TextRange tr;
  tr.chrg.cpMin = 0;
  tr.chrg.cpMax = currentLength;
  tr.lpstrText = data;

  ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));

  int tabwidth = ::SendMessage(hCurrentEditView, SCI_GETTABWIDTH, 0, 0);
  int usetabs = ::SendMessage(hCurrentEditView, SCI_GETUSETABS, 0, 0);
  if (tabwidth <= 0) tabwidth = 4;

  xmlDocPtr doc;

  doc = pXmlReadMemory(data, currentLength, "noname.xml", NULL, 0);
  if (doc == NULL) {
    Report::_printf_err(TEXT("Errors detected in content. Please correct them before applying pretty print."));
    delete [] data;
    return;
  }

  pXmlFreeDoc(doc);

  std::string str(data);

  // count the < and > signs; > are ignored if tagsignlevel <= 0. This prevent indent errors if text or attributes contain > sign.
  int tagsignlevel = 0;
  // some state variables
  bool in_comment = false, in_header = false, in_attribute = false, in_nodetext = false, in_cdata = false;
  // some counters
  long curpos = 0, xmllevel = 0, strlength = 0;
  // some char value (pc = previous char, cc = current char, nc = next char, nnc = next next char)
  char pc, cc, nc, nnc;

  // Proceed to first pass if break adds are enabled
  if (addlinebreaks) {
    while (curpos < (long)str.length() && (curpos = str.find_first_of("<>",curpos)) > -1) {
      cc = str.at(curpos);

      if (cc == '<' && curpos < (long)str.length()-3 && !str.compare(curpos,4,"<!--")) {
        // Let's skip the comment
        curpos = str.find("-->",curpos+1);
      } else if (cc == '<' && curpos < (long)str.length()-8 && !str.compare(curpos,9,"<![CDATA[")) {
        // Let's skip the CDATA block
        curpos = str.find("]]>",curpos+1);
      } else if (cc == '>') {
        // Let's see if '>' is a end tag char (it might also be simple text)
        // To perform test, we search last of "<>". If it is a '>', current '>' is
        // simple text, if not, it is a end tag char. '>' text chars are ignored.
        int prevspecchar = str.find_last_of("<>",curpos-1);
        if (prevspecchar >= 0 && str.at(prevspecchar) == '<') {
          // We have found a '>' char, let's see if next non space/tab is a '<'
          bool isclosingtag = (curpos > 0 && str.at(curpos-1) == '/');
          int nextchar = str.find_first_not_of(" \t",curpos+1);
          int deltapos = nextchar-curpos;
          if (nextchar > -1 &&
              str.at(nextchar) == '<' &&
              curpos < (long)str.length()-(deltapos+1)) {
            // we compare previous and next tags; if they are same, we don't add line break
            long startprev = str.rfind("<",curpos);
            long endnext = str.find(">",nextchar);

            if (startprev > -1 && endnext > -1 &&
                curpos > startprev &&
                endnext > nextchar) {
              int tagend = str.find_first_of(" />",startprev+1);
              std::string tag1(str.substr(startprev+1,tagend-startprev-1));
              tagend = str.find_first_of(" />",nextchar+2);
              std::string tag2(str.substr(nextchar+2,tagend-nextchar-2));
              if (strcmp(tag1.c_str(),tag2.c_str()) || isclosingtag) {
                // Case of "<data><data>..." -> add a line break between tags
                str.insert(++curpos,"\r\n");
              } else if (str.at(nextchar+1) == '/' && !isclosingtag) {
                // Case of "<data id="..."></data>" -> replace by "<data id="..."/>"
                str.replace(curpos,endnext-curpos,"/");
                //str.insert(++curpos,"#");
              }
            }
          }
        }
      }

      ++curpos;           // go to next char
    }
  /*
    while (curpos < str.length()-2 && (curpos = str.find("><",curpos)) > -1)
    {
      // we compare previous and next tags; if they are same, we don't add line break
      long startprev = str.rfind("<",curpos);
      long endnext = str.find(">",curpos+1);
    
      if (startprev > -1 &&
          endnext > -1 &&
          curpos > startprev &&
          endnext > curpos+1 &&
          strcmp(str.substr(startprev+1,curpos-startprev-1).c_str(),
                 str.substr(curpos+3,endnext-curpos-3).c_str()))
        str.insert(++curpos,"\n");

      ++curpos;// go to next char
    }*/

    // reinitialize cursor pos for second pass
    curpos = 0;
  }

  // Proceed to reformatting (second pass)
  int prevspecchar = -1;
  while (curpos < (long)str.length() && (curpos = str.find_first_of("<>\n\"",curpos)) > -1) {
    strlength = str.length();
    if (str.at(curpos) != '\n') {
      if (curpos < strlength-3 && !str.compare(curpos,4,"<!--")) in_comment = true;
      if (curpos < strlength-8 && !str.compare(curpos,9,"<![CDATA[")) in_cdata = true;
      else if (curpos < strlength-1 && !str.compare(curpos,2,"<?")) in_header = true;
      else if (curpos < strlength && !str.compare(curpos,1,"\"") &&
               prevspecchar >= 0 && str.at(prevspecchar) == '<') in_attribute = !in_attribute;
    }

    if (!in_comment && !in_cdata && !in_header) {
      if (curpos > 0) pc = str.at(curpos-1);
      else pc = ' ';

      cc = str.at(curpos);

      if (curpos < strlength-1) nc = str.at(curpos+1);
      else nc = ' ';

      if (curpos < strlength-2) nnc = str.at(curpos+2);
      else nnc = ' ';
        
      if (cc == '<') {
        prevspecchar = curpos++;
        ++tagsignlevel;
        in_nodetext = false;
        if (nc != '/' && (nc != '!' || nnc == '[')) xmllevel += 2;
      } else if (cc == '>' && !in_attribute) {
        // > are ignored inside attributes
        if (pc != '/' && pc != ']') { --xmllevel; in_nodetext = true; }
        else xmllevel -= 2;

        if (xmllevel < 0) xmllevel = 0;
        --tagsignlevel;
        prevspecchar = curpos++;
      } else if (cc == '\n') {
        // \n are ignored inside attributes
        int nextchar = str.find_first_not_of(" \t",++curpos);

        bool skipformat = false;
        if (!autoindenttext && nextchar > -1) {
          cc = str.at(nextchar);
          skipformat = (cc != '<' && cc != '\r' && cc != '\n');
        }
        if (nextchar >= curpos && xmllevel >= 0 && !skipformat) {
          if (nextchar < 0) nextchar = curpos;
          int delta = 0;
          str.erase(curpos,nextchar-curpos);

          strlength = str.length();
          if (curpos < strlength) {
            cc = str.at(curpos);
            // we set delta = 1 if we technically can + if we are in a text or inside an attribute
            if (xmllevel > 0 && curpos < strlength-1 && ( (cc == '<' && str.at(curpos+1) == '/') || in_attribute) ) delta = 1;
            else if (cc == '\n' || cc == '\r') delta = xmllevel;
          }

          if (usetabs) str.insert(curpos,(xmllevel-delta),'\t');
          else str.insert(curpos,tabwidth*(xmllevel-delta),' ');
        }
      } else {
        ++curpos;
      }
    } else {
      if (in_comment && curpos > 1 && !str.compare(curpos-2,3,"-->")) in_comment = false;
      else if (in_cdata && curpos > 1 && !str.compare(curpos-2,3,"]]>")) in_cdata = false;
      else if (in_header && curpos > 0 && !str.compare(curpos-1,2,"?>")) in_header = false;
      ++curpos;
    }
  }

  // Send formated string to scintilla
  ::SendMessage(hCurrentEditView, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(str.c_str()));

  // Restore scrolling
  ::SendMessage(hCurrentEditView, SCI_LINESCROLL, 0, yOffset);
  ::SendMessage(hCurrentEditView, SCI_SETXOFFSET, xOffset, 0);

  tr.lpstrText = NULL;
  delete [] data;
}

void prettyPrintXML() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("prettyPrintXML()");
  #endif
  prettyPrint(false, false);
}

void prettyPrintXMLBreaks() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("prettyPrintXMLBreaks()");
  #endif
  prettyPrint(false, true);
}

void prettyPrintText() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("prettyPrintText()");
  #endif
  prettyPrint(true, false);
}

///////////////////////////////////////////////////////////////////////////////

void linarizeXML() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("linarizeXML()");
  #endif
  int currentEdit, currentLength;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  currentLength = (int) ::SendMessage(hCurrentEditView, SCI_GETLENGTH, 0, 0);

  char *data = new char[currentLength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', currentLength+1);

  int currentPos = int(::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, 0));

  TextRange tr;
  tr.chrg.cpMin = 0;
  tr.chrg.cpMax = currentLength;
  tr.lpstrText = data;

  ::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));

  std::string str(data);

  unsigned int curpos = 0;
  while ((curpos = str.find_first_of("\r\n", curpos)) > -1) {
    unsigned int nextchar = str.find_first_not_of("\r\n", curpos);
    str.erase(curpos, nextchar-curpos);

    // on supprime aussi tous les espaces du d�but de ligne
    if (curpos < str.length()) {
      nextchar = str.find_first_not_of(" \t", curpos);
      if (nextchar >= curpos) {
        // et si le 1e caract�re de la ligne suivante est diff�rent de '<' et que
        // le dernier de la pr�c�dente est diff�rent de '>', autrement dit si on
        // est dans du texte, on laisse un espace

        bool enableInsert = false;
        if (curpos > 0 && str.at(nextchar) != '<' && str.at(curpos-1) != '>') {
          enableInsert = true;
          if (nextchar > curpos) --nextchar;
        }

        if (nextchar > curpos) str.erase(curpos, nextchar-curpos);
        else if (enableInsert) str.insert(nextchar, " ");
      }
    }
  }

  // Send formated string to scintilla
  ::SendMessage(hCurrentEditView, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(str.c_str()));

  tr.lpstrText = NULL;
  delete [] data;
}

///////////////////////////////////////////////////////////////////////////////

void convertText2XML() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("convertText2XML()");
  #endif
  int currentEdit, isReadOnly, xOffset, yOffset;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  
  isReadOnly = (int) ::SendMessage(hCurrentEditView, SCI_GETREADONLY, 0, 0);
  if (isReadOnly) return;
  
  xOffset = (int) ::SendMessage(hCurrentEditView, SCI_GETXOFFSET, 0, 0);
  yOffset = (int) ::SendMessage(hCurrentEditView, SCI_GETFIRSTVISIBLELINE, 0, 0);

  int selstart = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONSTART, 0, 0);
  int selend = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONEND, 0, 0)-1;
  int sellength = selend-selstart;

  if (selend <= selstart) {
    Report::_printf_err(TEXT("Please select text to transform before you call the function."));
    return;
  }

  char *data = new char[sellength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', sellength+1);

  ::SendMessage(hCurrentEditView, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(data));

  std::string str(data);
  long curpos = sellength;

  while (curpos >= 0 && (curpos = str.rfind("&quot;", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,6,"\"");
      sellength -= 5;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("&lt;", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,4,"<");
      sellength -= 3;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("&gt;", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,4,">");
      sellength -= 3;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("&amp;", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,5,"&");
      sellength -= 4;
    }
    --curpos;
  }

  // Replace the selection with new string
  ::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(str.c_str()));

  // Defines selection without scrolling
  ::SendMessage(hCurrentEditView, SCI_SETCURRENTPOS, selstart, 0);
  ::SendMessage(hCurrentEditView, SCI_SETANCHOR, selstart+sellength+1, 0);

  // Restore scrolling
  //::SendMessage(hCurrentEditView, SCI_LINESCROLL, 0, yOffset);
  //::SendMessage(hCurrentEditView, SCI_SETXOFFSET, xOffset, 0);

  delete [] data;
}

///////////////////////////////////////////////////////////////////////////////

void convertXML2Text() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("convertXML2Text()");
  #endif
  int currentEdit, isReadOnly, xOffset, yOffset;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);

  isReadOnly = (int) ::SendMessage(hCurrentEditView, SCI_GETREADONLY, 0, 0);
  if (isReadOnly) return;
  
  xOffset = (int) ::SendMessage(hCurrentEditView, SCI_GETXOFFSET, 0, 0);
  yOffset = (int) ::SendMessage(hCurrentEditView, SCI_GETFIRSTVISIBLELINE, 0, 0);

  int selstart = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONSTART, 0, 0);
  int selend = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONEND, 0, 0)-1;
  int sellength = selend-selstart;

  if (selend <= selstart) {
    Report::_printf_err(TEXT("Please select text to transform before you call the function."));
    return;
  }

  char *data = new char[sellength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', sellength+1);

  ::SendMessage(hCurrentEditView, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(data));

  std::string str(data);
  long curpos = sellength;

  while (curpos >= 0 && (curpos = str.rfind("&", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,1,"&amp;");
      sellength += 4;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("<", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,1,"&lt;");
      sellength += 3;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind(">", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,1,"&gt;");
      sellength += 3;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("\"", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,1,"&quot;");
      sellength += 5;
    }
    --curpos;
  }

  // Replace the selection with new string
  ::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(str.c_str()));

  // Defines selection without scrolling
  ::SendMessage(hCurrentEditView, SCI_SETCURRENTPOS, selstart, 0);
  ::SendMessage(hCurrentEditView, SCI_SETANCHOR, selstart+sellength+1, 0);

  // Restore scrolling
  //::SendMessage(hCurrentEditView, SCI_LINESCROLL, 0, yOffset);
  //::SendMessage(hCurrentEditView, SCI_SETXOFFSET, xOffset, 0);

  delete [] data;
}

///////////////////////////////////////////////////////////////////////////////

int validateSelectionForComment(std::string str, long sellength) {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("validateSelectionForComment()");
  #endif
  // Validate the selection
  std::stack<int> checkstack;
  long curpos = 0;
  int errflag = 0;
  while (curpos <= sellength && !errflag && (curpos = str.find_first_of("<-*", curpos)) > -1) {
    if (curpos > sellength) break;

    if (!str.compare(curpos, 4, "<!--")) {
      checkstack.push(0);
    }
    if (!str.compare(curpos, 3, "-->")) {
      if (!checkstack.empty()) {
        if (checkstack.top() != 0) errflag = checkstack.top();
        else checkstack.pop();
      } else {
        errflag = -3;
        break;
      }
    }
    if (!str.compare(curpos, 3, "<![")) {
      int endvalpos = str.find("]**", curpos);
      if (endvalpos >= 0) checkstack.push(atoi(str.substr(curpos+3,endvalpos).c_str()));
    }
    if (!str.compare(curpos, 3, "**[")) {
      if (!checkstack.empty()) {
        int endvalpos = str.find("]>", curpos);
        if (endvalpos >= 0 && atoi(str.substr(curpos+3,endvalpos).c_str()) != checkstack.top()) errflag = -2;
        else checkstack.pop();
      } else {
        errflag = -4;
        break;
      }
    }
    ++curpos;
  }
  if (!checkstack.empty()) errflag = checkstack.size();

  return errflag;
}

void commentSelection() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("commentSelection()");
  #endif
  long currentEdit, xOffset, yOffset;
  int isReadOnly;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  
  isReadOnly = (int) ::SendMessage(hCurrentEditView, SCI_GETREADONLY, 0, 0);
  if (isReadOnly) return;
  
  xOffset = (long) ::SendMessage(hCurrentEditView, SCI_GETXOFFSET, 0, 0);
  yOffset = (long) ::SendMessage(hCurrentEditView, SCI_GETFIRSTVISIBLELINE, 0, 0);
  long selstart = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONSTART, 0, 0);
  long selend = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONEND, 0, 0);
  long sellength = selend-selstart;
  
  if (selend <= selstart) {
    Report::_printf_err(TEXT("Please select text to transform before you call the function."));
    return;
  }

  char *data = new char[sellength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', sellength+1);

  ::SendMessage(hCurrentEditView, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(data));

  while (sellength >= 0 && (data[sellength] == '\r' || data[sellength] == '\n')) {
    data[sellength--] = '\0';
  }

  std::string str(data);

  int errflag = validateSelectionForComment(str, sellength);
  if (errflag) {
    TCHAR msg[512];
    swprintf(msg, TEXT("The current selection covers part only one portion of another comment.\nUncomment process may be not applicable.\n\nDo you want to continue ?"), errflag);
    if (::MessageBox(nppData._nppHandle, msg, TEXT("XML Tools plugin"), MB_YESNO | MB_ICONASTERISK) == IDNO) {
      delete [] data;
      return;
    }
  }

  long curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("<!{", curpos)) > -1) {
    if (curpos >= 0) {
      int endvalpos = str.find("}**", curpos);
      int endval = atoi(str.substr(curpos+3,endvalpos).c_str());
      char tmpstr[64];
      sprintf(tmpstr, "<!{%d}**", endval+1);
      str.replace(curpos,endvalpos-curpos+3,tmpstr);
      sellength += (strlen(tmpstr)-(endvalpos-curpos+3));
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("**{", curpos)) > -1) {
    if (curpos >= 0) {
      int endvalpos = str.find("}>", curpos);
      int endval = atoi(str.substr(curpos+3,endvalpos).c_str());
      char tmpstr[64];
      sprintf(tmpstr, "**{%d}>", endval+1);
      str.replace(curpos,endvalpos-curpos+2,tmpstr);
      sellength += (strlen(tmpstr)-(endvalpos-curpos+2));
    }
    --curpos;
  }

  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("<!--", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,4,"<!{1}**");
      sellength += 3;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("-->", curpos)) > -1) {
    if (curpos >= 0) {
      str.replace(curpos,3,"**{1}>");
      sellength += 3;
    }
    --curpos;
  }

  str.insert(0,"<!--"); sellength += 4;
  str.insert(sellength,"-->"); sellength += 3;

  // Replace the selection with new string
  ::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(str.c_str()));

  // Defines selection without scrolling
  ::SendMessage(hCurrentEditView, SCI_SETCURRENTPOS, selstart, 0);
  ::SendMessage(hCurrentEditView, SCI_SETANCHOR, selstart+sellength, 0);

  // Restore scrolling
  //::SendMessage(hCurrentEditView, SCI_LINESCROLL, 0, yOffset);
  //::SendMessage(hCurrentEditView, SCI_SETXOFFSET, xOffset, 0);

  delete [] data;
}

///////////////////////////////////////////////////////////////////////////////

void uncommentSelection() {
  #ifdef __XMLTOOLS_DEBUG__
    Report::_printf_inf("uncommentSelection()");
  #endif
  long currentEdit, xOffset, yOffset;
  int isReadOnly;
  ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  HWND hCurrentEditView = getCurrentHScintilla(currentEdit);
  
  isReadOnly = (int) ::SendMessage(hCurrentEditView, SCI_GETREADONLY, 0, 0);
  if (isReadOnly) return;
  
  xOffset = (long) ::SendMessage(hCurrentEditView, SCI_GETXOFFSET, 0, 0);
  yOffset = (long) ::SendMessage(hCurrentEditView, SCI_GETFIRSTVISIBLELINE, 0, 0);

  long selstart = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONSTART, 0, 0);
  long selend = ::SendMessage(hCurrentEditView, SCI_GETSELECTIONEND, 0, 0);
  long sellength = selend-selstart;

  if (selend <= selstart) {
    Report::_printf_err(TEXT("Please select text to transform before you call the function."));
    return;
  }

  char *data = new char[sellength+1];
  if (!data) return;  // allocation error, abort check
  memset(data, '\0', sellength+1);

  ::SendMessage(hCurrentEditView, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(data));

  std::string str(data);

  int errflag = validateSelectionForComment(str, sellength);
  if (errflag) {
    TCHAR msg[512];
    swprintf(msg, TEXT("Unable to uncomment the current selection.\nError code is %d."), errflag);
    Report::_printf_err(msg);
    delete [] data;
    return;
  }

  // Proceed to uncomment
  long curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("-->", curpos)) > -1) {
    if (curpos >= 0) {
      str.erase(curpos,3);
      sellength -= 3;
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("<!--", curpos)) > -1) {
    if (curpos >= 0) {
      str.erase(curpos,4);
      sellength -= 4;
    }
    --curpos;
  }

  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("<!{", curpos)) > -1) {
    if (curpos >= 0) {
      int endvalpos = str.find("}**", curpos);
      int endval = atoi(str.substr(curpos+3,endvalpos).c_str());
      if (endval > 1) {
        char tmpstr[64];
        sprintf(tmpstr, "<!{%d}**", endval-1);
        str.replace(curpos,endvalpos-curpos+3,tmpstr);
        sellength += (strlen(tmpstr)-(endvalpos-curpos+3));
      } else {
        str.replace(curpos,endvalpos-curpos+3,"<!--");
        sellength += (4-(endvalpos-curpos+3));
      }
    }
    --curpos;
  }
  curpos = sellength;
  while (curpos >= 0 && (curpos = str.rfind("**{", curpos)) > -1) {
    if (curpos >= 0) {
      int endvalpos = str.find("}>", curpos);
      int endval = atoi(str.substr(curpos+3,endvalpos).c_str());
      if (endval > 1) {
        char tmpstr[64];
        sprintf(tmpstr, "**{%d}>", endval-1);
        str.replace(curpos,endvalpos-curpos+2,tmpstr);
        sellength += (strlen(tmpstr)-(endvalpos-curpos+2));
      } else {
        str.replace(curpos,endvalpos-curpos+2,"-->");
        sellength += (3-(endvalpos-curpos+2));
      }
    }
    --curpos;
  }

  // Replace the selection with new string
  ::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(str.c_str()));

  // Defines selection without scrolling
  ::SendMessage(hCurrentEditView, SCI_SETCURRENTPOS, selstart, 0);
  ::SendMessage(hCurrentEditView, SCI_SETANCHOR, selstart+sellength, 0);

  // Restore scrolling
  //::SendMessage(hCurrentEditView, SCI_LINESCROLL, 0, yOffset);
  //::SendMessage(hCurrentEditView, SCI_SETXOFFSET, xOffset, 0);

  delete [] data;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CXMLToolsApp object

CXMLToolsApp* theApp = new CXMLToolsApp();