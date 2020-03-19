#include "stdafx.h"
#include "GimArchive.h"
#include "Windows/DLL.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/NtCheck.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

DEFINE_GUID(CLSID_CFormat7z,0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatTar,0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEE, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatGim,0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE8, 0x00, 0x00);

using namespace NWindows;

typedef UINT32 (WINAPI * CreateObjectFunc)(
	const GUID *clsID,
	const GUID *interfaceID,
	void **outObject);


void PrintString(const UString &s)
{
	printf("%s", (LPCSTR)GetOemString(s));
}

void PrintString(const AString &s)
{
	printf("%s", (LPCSTR)s);
}

void PrintNewLine()
{
	PrintString("\n");
}

void PrintStringLn(const AString &s)
{
	PrintString(s);
	PrintNewLine();
}

void PrintError(const AString &s)
{
	PrintNewLine();
	PrintString(s);
	PrintNewLine();
}
static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
	NCOM::CPropVariant prop;
	RINOK(archive->GetProperty(index, propID, &prop));
	if (prop.vt == VT_BOOL)
		result = VARIANT_BOOLToBool(prop.boolVal);
	else if (prop.vt == VT_EMPTY)
		result = false;
	else
		return E_FAIL;
	return S_OK;
}

static HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
	return IsArchiveItemProp(archive, index, kpidIsDir, result);
}
static const wchar_t *kEmptyFileAlias = L"[Content]";
//////////////////////////////////////////////////////////////
//CArchiveUpdateCallback
STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64 /* size */)
{
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 * /* completeValue */)
{
	return S_OK;
}


STDMETHODIMP CArchiveUpdateCallback::EnumProperties(IEnumSTATPROPSTG ** /* enumerator */)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 /* index */,
	Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
{
	if (newData != NULL)
		*newData = BoolToInt(true);
	if (newProperties != NULL)
		*newProperties = BoolToInt(true);
	if (indexInArchive != NULL)
		*indexInArchive = (UInt32)-1;
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
	NWindows::NCOM::CPropVariant prop;

	if (propID == kpidIsAnti)
	{
		prop = false;
		prop.Detach(value);
		return S_OK;
	}
	{
		const CDirItem &dirItem = (*DirItems)[index];
		switch(propID)
		{
		case kpidPath:  prop = dirItem.Name; break;
		case kpidIsDir:  prop = dirItem.isDir(); break;
		case kpidSize:  prop = dirItem.Size; break;
		case kpidAttrib:  prop = dirItem.Attrib; break;
		case kpidCTime:  prop = dirItem.CTime; break;
		case kpidATime:  prop = dirItem.ATime; break;
		case kpidMTime:  prop = dirItem.MTime; break;
		}
	}
	prop.Detach(value);
	return S_OK;
}

HRESULT CArchiveUpdateCallback::Finilize()
{
	if (m_NeedBeClosed)
	{
		PrintNewLine();
		m_NeedBeClosed = false;
	}
	return S_OK;
}

static void GetStream2(const wchar_t *name)
{
	PrintString("Compressing  ");
	if (name[0] == 0)
		name = kEmptyFileAlias;
	PrintString(name);
}

STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
{
	RINOK(Finilize());

	const CDirItem &dirItem = (*DirItems)[index];
	GetStream2(dirItem.Name);

	if (dirItem.isDir())
		return S_OK;

	{
		CInFileStream *inStreamSpec = new CInFileStream;
		CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
		UString path = DirPrefix + dirItem.FullPath;
		if (!inStreamSpec->Open(path))
		{
			DWORD sysError = ::GetLastError();
			FailedCodes.Add(sysError);
			FailedFiles.Add(path);
			// if (systemError == ERROR_SHARING_VIOLATION)
			{
				PrintNewLine();
				PrintError("WARNING: can't open file");
				// PrintString(NError::MyFormatMessageW(systemError));
				return S_FALSE;
			}
			// return sysError;
		}
		*inStream = inStreamLoc.Detach();
	}
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32 /* operationResult */)
{
	m_NeedBeClosed = true;
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)
{
	if (VolumesSizes.Size() == 0)
		return S_FALSE;
	if (index >= (UInt32)VolumesSizes.Size())
		index = VolumesSizes.Size() - 1;
	*size = VolumesSizes[index];
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)
{
	wchar_t temp[16];
	ConvertUInt32ToString(index + 1, temp);
	UString res = temp;
	while (res.Length() < 2)
		res = UString(L'0') + res;
	UString fileName = VolName;
	fileName += L'.';
	fileName += res;
	fileName += VolExt;
	COutFileStream *streamSpec = new COutFileStream;
	CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
	if (!streamSpec->Create(fileName, false))
		return ::GetLastError();
	*volumeStream = streamLoc.Detach();
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
	if (!PasswordIsDefined)
	{
		if (AskPassword)
		{
			// You can ask real password here from user
			// Password = GetPassword(OutStream);
			// PasswordIsDefined = true;
			PrintError("Password is not defined");
			return E_ABORT;
		}
	}
	*passwordIsDefined = BoolToInt(PasswordIsDefined);
	return StringToBstr(Password, password);
}
//////////////////////////////////////////////////////////////////////////
//
STDMETHODIMP CArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
	return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
	return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
	if (!PasswordIsDefined)
	{
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		PrintError("Password is not defined");
		return E_ABORT;
	}
	return StringToBstr(Password, password);
}
//////////////////////////////////////////////////////////////////////////
//CArchiveExtractCallback
static const wchar_t *kCantDeleteOutputFile = L"ERROR: Can not delete output file ";

static const char *kTestingString    =  "Testing     ";
static const char *kExtractingString =  "Extracting  ";
static const char *kSkippingString   =  "Skipping    ";

static const char *kUnsupportedMethod = "Unsupported Method";
static const char *kCRCFailed = "CRC Failed";
static const char *kDataError = "Data Error";
static const char *kUnknownError = "Unknown Error";
void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const UString &directoryPath)
{
	NumErrors = 0;
	_archiveHandler = archiveHandler;
	_directoryPath = directoryPath;
	NFile::NName::NormalizeDirPathPrefix(_directoryPath);
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 /* size */)
{
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 * /* completeValue */)
{
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
	ISequentialOutStream **outStream, Int32 askExtractMode)
{
	*outStream = 0;
	_outFileStream.Release();

	{
		// Get Name
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));

		UString fullPath;
		if (prop.vt == VT_EMPTY)
			fullPath = kEmptyFileAlias;
		else
		{
			if (prop.vt != VT_BSTR)
				return E_FAIL;
			fullPath = prop.bstrVal;
		}
		_filePath = fullPath;
	}

	if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
		return S_OK;

	{
		// Get Attrib
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
		if (prop.vt == VT_EMPTY)
		{
			_processedFileInfo.Attrib = 0;
			_processedFileInfo.AttribDefined = false;
		}
		else
		{
			if (prop.vt != VT_UI4)
				return E_FAIL;
			_processedFileInfo.Attrib = prop.ulVal;
			_processedFileInfo.AttribDefined = true;
		}
	}

	RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

	{
		// Get Modified Time
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
		_processedFileInfo.MTimeDefined = false;
		switch(prop.vt)
		{
		case VT_EMPTY:
			// _processedFileInfo.MTime = _utcMTimeDefault;
			break;
		case VT_FILETIME:
			_processedFileInfo.MTime = prop.filetime;
			_processedFileInfo.MTimeDefined = true;
			break;
		default:
			return E_FAIL;
		}

	}
	{
		// Get Size
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
		bool newFileSizeDefined = (prop.vt != VT_EMPTY);
		UInt64 newFileSize;
		if (newFileSizeDefined)
			newFileSize = ConvertPropVariantToUInt64(prop);
	}


	{
		// Create folders for file
		int slashPos = _filePath.ReverseFind(WCHAR_PATH_SEPARATOR);
		if (slashPos >= 0)
			NFile::NDirectory::CreateComplexDirectory(_directoryPath + _filePath.Left(slashPos));
	}

	UString fullProcessedPath = _directoryPath + _filePath;
	_diskFilePath = fullProcessedPath;

	if (_processedFileInfo.isDir)
	{
		NFile::NDirectory::CreateComplexDirectory(fullProcessedPath);
	}
	else
	{
		NFile::NFind::CFileInfoW fi;
		if (fi.Find(fullProcessedPath))
		{
			if (!NFile::NDirectory::DeleteFileAlways(fullProcessedPath))
			{
				PrintString(UString(kCantDeleteOutputFile) + fullProcessedPath);
				return E_ABORT;
			}
		}

		_outFileStreamSpec = new COutFileStream;
		CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
		if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
		{
			PrintString((UString)L"can not open output file " + fullProcessedPath);
			return E_ABORT;
		}
		_outFileStream = outStreamLoc;
		*outStream = outStreamLoc.Detach();
	}
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
	_extractMode = false;
	switch (askExtractMode)
	{
	case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
	};
	switch (askExtractMode)
	{
	case NArchive::NExtract::NAskMode::kExtract:  PrintString(kExtractingString); break;
	case NArchive::NExtract::NAskMode::kTest:  PrintString(kTestingString); break;
	case NArchive::NExtract::NAskMode::kSkip:  PrintString(kSkippingString); break;
	};
	PrintString(_filePath);
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
	switch(operationResult)
	{
	case NArchive::NExtract::NOperationResult::kOK:
		break;
	default:
		{
			NumErrors++;
			PrintString("     ");
			switch(operationResult)
			{
			case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
				PrintString(kUnsupportedMethod);
				break;
			case NArchive::NExtract::NOperationResult::kCRCError:
				PrintString(kCRCFailed);
				break;
			case NArchive::NExtract::NOperationResult::kDataError:
				PrintString(kDataError);
				break;
			default:
				PrintString(kUnknownError);
			}
		}
	}

	if (_outFileStream != NULL)
	{
		if (_processedFileInfo.MTimeDefined)
			_outFileStreamSpec->SetMTime(&_processedFileInfo.MTime);
		RINOK(_outFileStreamSpec->Close());
	}
	_outFileStream.Release();
	if (_extractMode && _processedFileInfo.AttribDefined)
		NFile::NDirectory::MySetFileAttributes(_diskFilePath, _processedFileInfo.Attrib);
	PrintNewLine();
	return S_OK;
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
	if (!PasswordIsDefined)
	{
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		PrintError("Password is not defined");
		return E_ABORT;
	}
	return StringToBstr(Password, password);
}
//////////////////////////////////////////////////////////////////////////
//根据给定目标路径查找文件获取存档信息
void GetArchiveItemFromPath(LPCTSTR strDirPath,CObjectVector<CDirItem> &dirItems)
{
	NFile::NFind::CFileInfo folder,file;
	NFile::NFind::CFindFile find,find_file;
	CDirItem di;
	AString dirPath,filePath;
	dirPath=strDirPath;
	dirPath+="*.*";
	find.FindFirst(dirPath,folder);
	while(find.FindNext(folder))
	{
		if(folder.IsDots())
			continue;
		di.Attrib = folder.Attrib;
		di.Size = folder.Size;
		di.CTime = folder.CTime;
		di.ATime = folder.ATime;
		di.MTime = folder.MTime;
		di.Name = MultiByteToUnicodeString(folder.Name);
		di.FullPath = MultiByteToUnicodeString(strDirPath);
		di.FullPath+=MultiByteToUnicodeString("\\");
		di.FullPath+=MultiByteToUnicodeString(folder.Name);
		dirItems.Add(di);
		//
		filePath=strDirPath;
		filePath+=folder.Name;
		filePath+="\\*.*";
		find_file.FindFirst(filePath,file);
		while(find_file.FindNext(file))
		{
			if (file.IsDots())
				continue;
			di.Attrib = file.Attrib;
			di.Size = file.Size;
			di.CTime = file.CTime;
			di.ATime = file.ATime;
			di.MTime = file.MTime;
			di.Name = MultiByteToUnicodeString(file.Name);
			di.FullPath = MultiByteToUnicodeString(strDirPath);
			di.FullPath+= MultiByteToUnicodeString(folder.Name);
			di.FullPath+=MultiByteToUnicodeString("\\");
			di.FullPath+=MultiByteToUnicodeString(file.Name);
			dirItems.Add(di);
		}
	}
}
//根据给定文件列表获取存档信息
void GetArchiveItemFromFileList(CObjectVector<AString> FileList,CObjectVector<CDirItem> &ItemList)
{
	NFile::NFind::CFileInfo fi;
	for (int i=0;i<FileList.Size();i++)
	{
		if (fi.Find(FileList[i]))
		{
			CDirItem di;
			di.Attrib = fi.Attrib;
			di.Size = fi.Size;
			di.CTime = fi.CTime;
			di.ATime = fi.ATime;
			di.MTime = fi.MTime;
			di.Name = MultiByteToUnicodeString(fi.Name);
			di.FullPath = MultiByteToUnicodeString(FileList[i]);
			ItemList.Add(di);
		}
	}
}
//进行压缩打包
BOOL ArchiveFile(CObjectVector<CDirItem> &dirItems,LPCWSTR ArchivePackPath)
{
	//加载7z.dll
	NWindows::NDLL::CLibrary lib;
	if (!lib.Load(TEXT("7z.dll")))
		return FALSE;
	CreateObjectFunc createObjectFunc = (CreateObjectFunc)lib.GetProc("CreateObject");//获取COM 接口
	if (createObjectFunc == 0)
		return FALSE;
	//保存打包位置
	UString archiveName=ArchivePackPath;//打包后的位置    
	//创建 打包文件
	COutFileStream *outFileStreamSpec = new COutFileStream;
	CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
	if (!outFileStreamSpec->Create(archiveName, true))
		return FALSE;
	//取IOUTARCHIVE 接口
	CMyComPtr<IOutArchive> outArchive;    
	if (createObjectFunc(&CLSID_CFormat7z, &IID_IOutArchive, (void **)&outArchive) != S_OK)
		return FALSE;
	//压缩时 会调用的回调函数
	CArchiveUpdateCallback *updateCallbackSpec = new CArchiveUpdateCallback;         
	CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
	updateCallbackSpec->Init(&dirItems);

	HRESULT result = outArchive->UpdateItems(outFileStream, dirItems.Size(), updateCallback);
	updateCallbackSpec->Finilize();            //压缩

	if (result != S_OK)
	{
		PrintError("Update Error");
		return FALSE;
	}
	for (int i = 0; i < updateCallbackSpec->FailedFiles.Size(); i++)    //打印失败文件
	{
		PrintNewLine();
		PrintString((UString)L"Error for file: " + updateCallbackSpec->FailedFiles[i]);
	}
	if (updateCallbackSpec->FailedFiles.Size() != 0)//判断有几个失败的文件
		return FALSE;
	return TRUE;
}
//进行解压
BOOL UnpackFile(LPCWSTR ArchiveName)
{
	NWindows::NDLL::CLibrary lib;
	if (!lib.Load(TEXT("7z.dll")))
		return FALSE;
	CreateObjectFunc createObjectFunc = (CreateObjectFunc)lib.GetProc("CreateObject");
	if (createObjectFunc == 0)
		return FALSE;
	CMyComPtr<IInArchive> archive;
	if (createObjectFunc(&CLSID_CFormat7z, &IID_IInArchive, (void **)&archive) != S_OK)
		return FALSE;
	CInFileStream *fileSpec = new CInFileStream;
	CMyComPtr<IInStream> file = fileSpec;
	if (!fileSpec->Open(ArchiveName))
		return FALSE;
	CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
	CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
	openCallbackSpec->PasswordIsDefined = false;
	if (archive->Open(file, 0, openCallback) != S_OK)
	{
		PrintError("Can not open archive");
		return FALSE;
	}
	CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
	CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
	extractCallbackSpec->Init(archive, L""); // second parameter is output folder path
	extractCallbackSpec->PasswordIsDefined = false;
	// extractCallbackSpec->PasswordIsDefined = true;
	// extractCallbackSpec->Password = L"1";
	HRESULT result = archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);
	if (result != S_OK)
	{
		PrintError("Extract Error");
		return FALSE;
	}
	return TRUE;
}