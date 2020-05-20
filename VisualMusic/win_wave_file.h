
// win_wave_file.h

#pragma once
#include "win_mm_wave_out.h"

#define FOURCC_WAVE     mmioFOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt      mmioFOURCC('f', 'm', 't', ' ')
#define FOURCC_data      mmioFOURCC('d', 'a', 't', 'a')

class CWaveFile : protected CWinMMIO {
 public:
  CWaveFile::CWaveFile():m_p_wave_format_ex_(NULL) {}

  BOOL Open(LPCTSTR sz_file_name, const DWORD dw_open_flags = MMIO_READ);

  inline MMRESULT Close() {
    assert(m_hIO != NULL);

    // 如果是写 wav ，则关闭前要在 riff 块和 data 块中加上大小信息
    if (m_dw_open_flags_ & MMIO_WRITE) {
      assert(m_p_wave_format_ex_ != NULL);

      // 得出整个文件的大小
      LONG size = CWinMMIO::Seek(0, SEEK_END);

      // 得出并写入 riff 块大小
      DWORD dw_riff_size = size - 8;
      if (CWinMMIO::Seek(4) != 4)
        assert(0);
      if (CWinMMIO::Write((char *)&dw_riff_size, 4) != 4)
        assert(0);

      // 得出并写入 data 块大小
      DWORD dw_data_size = size - m_ck_data_.dwDataOffset;
      if (CWinMMIO::Seek(m_ck_data_.dwDataOffset - 4) !=
          m_ck_data_.dwDataOffset - 4)
        assert(0);
      if (CWinMMIO::Write((char *)&dw_data_size, 4) != 4)
        assert(0);
    }

    if (m_p_wave_format_ex_ && m_p_wave_format_ex_ != &m_wave_format_ex_)
      delete m_p_wave_format_ex_;
    m_p_wave_format_ex_ = NULL;

    return CWinMMIO::Close();
  }

  inline PWAVEFORMATEX GetFormatExInfo() const{
    assert(m_hIO != NULL);

    return m_p_wave_format_ex_;
  }

  BOOL WriteFormatExInfo(PWAVEFORMATEX pWaveFormatEx);

  inline LONG Write( const char * pch, LONG cch) {
    assert(m_hIO != NULL);
    assert(m_dw_open_flags_ & MMIO_WRITE);
    assert(m_p_wave_format_ex_ != NULL);

    return CWinMMIO::Write(pch, cch);
  }

  inline DWORD GetDataSize() const{
    assert(m_hIO != NULL);
    assert((m_dw_open_flags_ & MMIO_WRITE) == 0);

    return m_ck_data_.cksize;
  }

  inline BOOL DataSeek( LONG _data_offset) {
    assert(m_hIO != NULL);
    assert(m_p_wave_format_ex_ != NULL);

    LONG lOffset = _data_offset + m_ck_data_.dwDataOffset;
    return CWinMMIO::Seek(lOffset) == lOffset;
  }

  inline LONG Read( OUT HPSTR pch, IN LONG cch) {
    assert(m_hIO != NULL);
    assert((m_dw_open_flags_ & MMIO_WRITE) == 0);

    return CWinMMIO::Read(pch, cch);
  }

 protected:
  PWAVEFORMATEX m_p_wave_format_ex_;
  WAVEFORMATEX m_wave_format_ex_;

  DWORD m_dw_open_flags_;

  MMCKINFO m_ck_data_;
};

