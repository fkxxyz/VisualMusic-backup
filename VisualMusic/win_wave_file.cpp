
// WinWaveFile.cpp

#include "stdafx.h"
#include "win_wave_file.h"

BOOL CWaveFile::Open(LPCTSTR sz_file_name, const DWORD dw_open_flags) {
  // Ҫô��Ҫôд����ֹ���ɶ��ֿ�д
  assert((dw_open_flags & MMIO_READWRITE) == 0);

  // �� riff �ļ�
  MMIOINFO info = {0};
  if (!CWinMMIO::Open((LPTSTR)sz_file_name, &info, dw_open_flags)) {
    SetLastError(LastErrorTab[info.wErrorRet - MMIOERR_BASE]);
    return FALSE;
  }

  // ����д�ֱ�ͬ�Ĳ���
  if (dw_open_flags & MMIO_WRITE) {
    
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
    if (ckFmt.cksize == 16) {

      // ֻ�� 16 ���ֽ��ǾͶ�ȡ 16 ���ֽڣ�û�и�����Ϣ
      if (CWinMMIO::Read((HPSTR)&m_wave_format_ex_, 16) != 16)
        goto llBadFormat;
      m_wave_format_ex_.cbSize = 0;
      m_p_wave_format_ex_ = &m_wave_format_ex_;

    } else {

      // ��ȡ 18 ���ֽ�
      if (ckFmt.cksize < 18)
        goto llBadFormat;
      if (CWinMMIO::Read((HPSTR)&m_wave_format_ex_, 18) != 18)
        goto llBadFormat;

      // ��鸽����Ϣ�Ĵ�С
      if (m_wave_format_ex_.cbSize < 0)
        goto llBadFormat;

      // ��� fmt ��͸�����Ϣ�ܴ�С���������ڴ�
      SIZE_T size = 18 + m_wave_format_ex_.cbSize;
      m_p_wave_format_ex_ = (PWAVEFORMATEX) new char [size];

      // ���� fmt ����Ϣ����ȡ������Ϣ
      *m_p_wave_format_ex_ = m_wave_format_ex_;
      if (CWinMMIO::Read(
          (HPSTR)m_p_wave_format_ex_ + 18, m_wave_format_ex_.cbSize) !=
              m_wave_format_ex_.cbSize)
        goto llBadFormat;
    }

    // Ѱ�� data ��
    m_ck_data_.ckid = FOURCC_data;
    if (CWinMMIO::Descend(&m_ck_data_, &ckRiff, 0))
      goto llBadFormat;
  }

  m_dw_open_flags_ = dw_open_flags;

  return TRUE;

llBadFormat:
  SetLastError(ERROR_BAD_FORMAT);
  return FALSE;
}

BOOL CWaveFile::WriteFormatExInfo(PWAVEFORMATEX pWaveFormatEx) {
  assert(m_hIO != NULL);
  assert(m_dw_open_flags_ & MMIO_WRITE);
  assert(m_p_wave_format_ex_ == NULL);

  // ���ڴ��б���һ�� pWaveFormatEx ��Ϣ������������Ϣ��
  m_wave_format_ex_ = *pWaveFormatEx;
  m_wave_format_ex_.cbSize = 0;
  m_p_wave_format_ex_ = &m_wave_format_ex_;

  // ��ͷ��ʼд
  CWinMMIO::Seek(0);

  // д�� riff ��ͷ
  MMCKINFO ckRiff = {FOURCC_RIFF, 0, FOURCC_WAVE};
  CWinMMIO::CreateChunk(&ckRiff, MMIO_CREATERIFF);

  // д�� fmt ��ͷ
  MMCKINFO ckFmt = {FOURCC_fmt, pWaveFormatEx->cbSize == 0 ? 16 : 18};
  CWinMMIO::CreateChunk(&ckFmt, NULL);

  // д�� wav ������Ϣ���丽����Ϣ
  CWinMMIO::Write(
      (char *)pWaveFormatEx,
      (pWaveFormatEx->cbSize == 0 ? 16 : 18) + pWaveFormatEx->cbSize);

  // д�� data ��ͷ
  m_ck_data_.ckid = FOURCC_data;
  m_ck_data_.cksize = 0;
  CWinMMIO::CreateChunk(&m_ck_data_, NULL);

  return TRUE;
}

