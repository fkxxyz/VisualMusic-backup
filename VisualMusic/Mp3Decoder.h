
// Mp3Decoder.h

#pragma once

#include "AudioDecoderTemplate.h"
#include "libmad\config.h"
#include "libmad\mad.h"
#include <fstream>
#include <vector>

template <class CHARTYPE>
class CMp3Decoder : public CAudioDecoderTemplate<CHARTYPE> {
private:
	static mad_flow mad_input_func(void *data, struct mad_stream *stream);
	static mad_flow mad_header_func(void *data, struct mad_header const *header);
	static mad_flow mad_output_func(void *data, struct mad_header const *header, struct mad_pcm *pcm);
	static mad_flow mad_error_func(void *data, struct mad_stream *stream, struct mad_frame *frame);

public:
	int IsValidFormat(CHARTYPE *filepath){
		assert(0);
		return 0;
	}

	int Decode(CHARTYPE *filepath, CPCMStream<double> *pcm);
};

#include "Mp3Decoder.inl"

