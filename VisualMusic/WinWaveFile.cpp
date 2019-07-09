
// WinWaveFile.cpp

#include "stdafx.h"
#include "WinWaveFile.h"

BOOL CWaveFile::Open(LPTSTR szFileName, DWORD dwOpenFlags){
	// Ҫô��Ҫôд����ֹ���ɶ��ֿ�д
	assert((dwOpenFlags & MMIO_READWRITE) == 0);

	// �� riff �ļ�
	MMIOINFO info = {0};
	if (!CWinMMIO::Open(szFileName, &info, dwOpenFlags)){
		SetLastError(LastErrorTab[info.wErrorRet - MMIOERR_BASE]);
		return FALSE;
	}

	// ����д�ֱ�ͬ�Ĳ���
	if (dwOpenFlags & MMIO_WRITE){
		
	} else {

		// ��ȡ riff �ĵ�һ����
		MMCKINFO ckRiff;
		if (CWinMMIO::Descend(&ckRiff, NULL, 0))
			goto llBadFormat;

		// �����һ�����Ƿ�Ϊ riff���Լ������Ƿ�Ϊ wave
		if (ckRiff.ckid != FOURCC_RIFF || ckRiff.fccType != FOURCC_WAVE)
			goto llBadFormat;

		// Ѱ�� fmt ��
		MMCKINFO ckFmt;
		ckFmt.ckid = FOURCC_fmt;
		if (CWinMMIO::Descend(&ckFmt, &ckRiff, 0))
			goto llBadFormat;

		// ��� fmt ��Ĵ�С�Ƿ�Ϸ�
		if (ckFmt.cksize < 16)
			goto llBadFormat;

		// ��ȡ fmt �鼰�丽����Ϣ
		if (ckFmt.cksize == 16){

			// ֻ�� 16 ���ֽ��ǾͶ�ȡ 16 ���ֽڣ�û�и�����Ϣ
			if (CWinMMIO::Read((HPSTR)&m_WaveFormatEx, 16) != 16)
				goto llBadFormat;
			m_WaveFormatEx.cbSize = 0;
			m_pWaveFormatEx = &m_WaveFormatEx;

		} else {

			// ��ȡ 18 ���ֽ�
			if (ckFmt.cksize < 18)
				goto llBadFormat;
			if (CWinMMIO::Read((HPSTR)&m_WaveFormatEx, 18) != 18)
				goto llBadFormat;

			// ��鸽����Ϣ�Ĵ�С
			if (m_WaveFormatEx.cbSize < 0)
				goto llBadFormat;

			// ��� fmt ��͸�����Ϣ�ܴ�С���������ڴ�
			SIZE_T size = 18 + m_WaveFormatEx.cbSize;
			m_pWaveFormatEx = (PWAVEFORMATEX) new char [size];

			// ���� fmt ����Ϣ����ȡ������Ϣ
			*m_pWaveFormatEx = m_WaveFormatEx;
			if (CWinMMIO::Read((HPSTR)m_pWaveFormatEx + 18, m_WaveFormatEx.cbSize) != m_WaveFormatEx.cbSize)
				goto llBadFormat;
		}

		// Ѱ�� data ��
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

	// ���ڴ��б���һ�� pWaveFormatEx ��Ϣ������������Ϣ��
	m_WaveFormatEx = *pWaveFormatEx;
	m_WaveFormatEx.cbSize = 0;
	m_pWaveFormatEx = &m_WaveFormatEx;

	// ��ͷ��ʼд
	CWinMMIO::Seek(0);

	// д�� riff ��ͷ
	MMCKINFO ckRiff = {FOURCC_RIFF, 0, FOURCC_WAVE};
	CWinMMIO::CreateChunk(&ckRiff, MMIO_CREATERIFF);

	// д�� fmt ��ͷ
	MMCKINFO ckFmt = {FOURCC_fmt, pWaveFormatEx->cbSize == 0 ? 16 : 18};
	CWinMMIO::CreateChunk(&ckFmt, NULL);

	// д�� wav ������Ϣ���丽����Ϣ
	CWinMMIO::Write((char *)pWaveFormatEx, (pWaveFormatEx->cbSize == 0 ? 16 : 18) + pWaveFormatEx->cbSize);

	// д�� data ��ͷ
	m_ckData.ckid = FOURCC_data;
	m_ckData.cksize = 0;
	CWinMMIO::CreateChunk(&m_ckData, NULL);

	return TRUE;

}

