
// WinWaveFile.cpp

#include "stdafx.h"
#include "WinWaveFile.h"

BOOL CWaveFile::Open(LPTSTR szFileName, DWORD dwOpenFlags){
	// 要么读要么写，禁止即可读又可写
	assert((dwOpenFlags & MMIO_READWRITE) == 0);

	// 打开 riff 文件
	MMIOINFO info = {0};
	if (!CWinMMIO::Open(szFileName, &info, dwOpenFlags)){
		SetLastError(LastErrorTab[info.wErrorRet - MMIOERR_BASE]);
		return FALSE;
	}

	// 读和写分别不同的操作
	if (dwOpenFlags & MMIO_WRITE){
		
	} else {

		// 读取 riff 的第一个块
		MMCKINFO ckRiff;
		if (CWinMMIO::Descend(&ckRiff, NULL, 0))
			goto llBadFormat;

		// 检验第一个块是否为 riff，以及类型是否为 wave
		if (ckRiff.ckid != FOURCC_RIFF || ckRiff.fccType != FOURCC_WAVE)
			goto llBadFormat;

		// 寻找 fmt 块
		MMCKINFO ckFmt;
		ckFmt.ckid = FOURCC_fmt;
		if (CWinMMIO::Descend(&ckFmt, &ckRiff, 0))
			goto llBadFormat;

		// 检查 fmt 块的大小是否合法
		if (ckFmt.cksize < 16)
			goto llBadFormat;

		// 读取 fmt 块及其附加信息
		if (ckFmt.cksize == 16){

			// 只有 16 个字节那就读取 16 个字节，没有附加信息
			if (CWinMMIO::Read((HPSTR)&m_WaveFormatEx, 16) != 16)
				goto llBadFormat;
			m_WaveFormatEx.cbSize = 0;
			m_pWaveFormatEx = &m_WaveFormatEx;

		} else {

			// 读取 18 个字节
			if (ckFmt.cksize < 18)
				goto llBadFormat;
			if (CWinMMIO::Read((HPSTR)&m_WaveFormatEx, 18) != 18)
				goto llBadFormat;

			// 检查附加信息的大小
			if (m_WaveFormatEx.cbSize < 0)
				goto llBadFormat;

			// 算出 fmt 块和附加信息总大小，并分配内存
			SIZE_T size = 18 + m_WaveFormatEx.cbSize;
			m_pWaveFormatEx = (PWAVEFORMATEX) new char [size];

			// 复制 fmt 块信息，读取附加信息
			*m_pWaveFormatEx = m_WaveFormatEx;
			if (CWinMMIO::Read((HPSTR)m_pWaveFormatEx + 18, m_WaveFormatEx.cbSize) != m_WaveFormatEx.cbSize)
				goto llBadFormat;
		}

		// 寻找 data 块
		m_ckData.ckid = FOURCC_data;
		if (CWinMMIO::Descend(&m_ckData, &ckRiff, 0))
			goto llBadFormat;
	}

	m_dwOpenFlags = dwOpenFlags;

	return TRUE;

llBadFormat:
	SetLastError(ERROR_BAD_FORMAT);
	return FALSE;
}

BOOL CWaveFile::WriteFormatExInfo(PWAVEFORMATEX pWaveFormatEx){
	assert(m_hIO != NULL);
	assert(m_dwOpenFlags & MMIO_WRITE);
	assert(m_pWaveFormatEx == NULL);

	// 在内存中保存一份 pWaveFormatEx 信息（不带附加信息）
	m_WaveFormatEx = *pWaveFormatEx;
	m_WaveFormatEx.cbSize = 0;
	m_pWaveFormatEx = &m_WaveFormatEx;

	// 从头开始写
	CWinMMIO::Seek(0);

	// 写入 riff 块头
	MMCKINFO ckRiff = {FOURCC_RIFF, 0, FOURCC_WAVE};
	CWinMMIO::CreateChunk(&ckRiff, MMIO_CREATERIFF);

	// 写入 fmt 块头
	MMCKINFO ckFmt = {FOURCC_fmt, pWaveFormatEx->cbSize == 0 ? 16 : 18};
	CWinMMIO::CreateChunk(&ckFmt, NULL);

	// 写入 wav 参数信息及其附加信息
	CWinMMIO::Write((char *)pWaveFormatEx, (pWaveFormatEx->cbSize == 0 ? 16 : 18) + pWaveFormatEx->cbSize);

	// 写入 data 块头
	m_ckData.ckid = FOURCC_data;
	m_ckData.cksize = 0;
	CWinMMIO::CreateChunk(&m_ckData, NULL);

	return TRUE;

}

