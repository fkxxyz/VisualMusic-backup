
// WinWaveFile.cpp

#include "stdafx.h"
#include "win_wave_file.h"

BOOL CWaveFile::Open(LPCTSTR sz_file_name, const DWORD dw_open_flags) {
  // 要么读要么写，禁止即可读又可写
  assert((dw_open_flags & MMIO_READWRITE) == 0);

  // 打开 riff 文件
  MMIOINFO info = {0};
  if (!CWinMMIO::Open((LPTSTR)sz_file_name, &info, dw_open_flags)) {
    SetLastError(LastErrorTab[info.wErrorRet - MMIOERR_BASE]);
    return FALSE;
  }

  // 读和写分别不同的操作
  if (dw_open_flags & MMIO_WRITE) {
    
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
    if (ckFmt.cksize == 16) {

      // 只有 16 个字节那就读取 16 个字节，没有附加信息
      if (CWinMMIO::Read((HPSTR)&m_wave_format_ex_, 16) != 16)
        goto llBadFormat;
      m_wave_format_ex_.cbSize = 0;
      m_p_wave_format_ex_ = &m_wave_format_ex_;

    } else {

      // 读取 18 个字节
      if (ckFmt.cksize < 18)
        goto llBadFormat;
      if (CWinMMIO::Read((HPSTR)&m_wave_format_ex_, 18) != 18)
        goto llBadFormat;

      // 检查附加信息的大小
      if (m_wave_format_ex_.cbSize < 0)
        goto llBadFormat;

      // 算出 fmt 块和附加信息总大小，并分配内存
      SIZE_T size = 18 + m_wave_format_ex_.cbSize;
      m_p_wave_format_ex_ = (PWAVEFORMATEX) new char [size];

      // 复制 fmt 块信息，读取附加信息
      *m_p_wave_format_ex_ = m_wave_format_ex_;
      if (CWinMMIO::Read(
          (HPSTR)m_p_wave_format_ex_ + 18, m_wave_format_ex_.cbSize) !=
              m_wave_format_ex_.cbSize)
        goto llBadFormat;
    }

    // 寻找 data 块
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

  // 在内存中保存一份 pWaveFormatEx 信息（不带附加信息）
  m_wave_format_ex_ = *pWaveFormatEx;
  m_wave_format_ex_.cbSize = 0;
  m_p_wave_format_ex_ = &m_wave_format_ex_;

  // 从头开始写
  CWinMMIO::Seek(0);

  // 写入 riff 块头
  MMCKINFO ckRiff = {FOURCC_RIFF, 0, FOURCC_WAVE};
  CWinMMIO::CreateChunk(&ckRiff, MMIO_CREATERIFF);

  // 写入 fmt 块头
  MMCKINFO ckFmt = {FOURCC_fmt, pWaveFormatEx->cbSize == 0 ? 16 : 18};
  CWinMMIO::CreateChunk(&ckFmt, NULL);

  // 写入 wav 参数信息及其附加信息
  CWinMMIO::Write(
      (char *)pWaveFormatEx,
      (pWaveFormatEx->cbSize == 0 ? 16 : 18) + pWaveFormatEx->cbSize);

  // 写入 data 块头
  m_ck_data_.ckid = FOURCC_data;
  m_ck_data_.cksize = 0;
  CWinMMIO::CreateChunk(&m_ck_data_, NULL);

  return TRUE;
}

