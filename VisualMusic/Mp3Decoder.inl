
// Mp3Decoder.inl

const int buf_size = 16384;

template <class CHARTYPE>
struct _TMP_DATA {
	enum mad_error error_code;
	CPCMStream<double> *pcm;
	std::vector<mad_fixed_t> data[2];
	std::ifstream ifs;
	unsigned char mp3_data[buf_size];
};

template <class CHARTYPE>
int CMp3Decoder<CHARTYPE>::Decode(CHARTYPE *filepath, CPCMStream<double> *pcm){
	struct _TMP_DATA<CHARTYPE> tmp_data = {
		MAD_ERROR_NONE,
		pcm
	};

	// 清空 pcm
	pcm->Clear();
	pcm->m_nSamples = 0;

	// 打开 mp3 文件
	tmp_data.ifs.open(filepath, std::ios::binary);

	// 开始解码
	struct mad_decoder decoder;
	mad_decoder_init(&decoder, &tmp_data, mad_input_func, NULL, NULL, mad_output_func, mad_error_func, NULL);
	int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	mad_decoder_finish(&decoder);

	// 关闭文件
	tmp_data.ifs.close();

	// 将结果复制到 pcm
	const int max_value = 268435456;
	pcm->m_max_amplitude = max_value;
	pcm->m_origin_pos = 0.0l;
	for (int n = 0; n < 2; n++){
		pcm->m_Samples[n] = new double [tmp_data.data[0].size()];
		register double *p = pcm->m_Samples[n];
		register int i_end = pcm->m_nSamples;
		for (register int i = 0; i < i_end; i++){
			register int x = tmp_data.data[n][i];
			if (x < -max_value) x = -max_value;
			if (x > max_value) x = max_value;
			*(p++) = x;
		}
	}

	return tmp_data.error_code == MAD_ERROR_NONE;
}

template <class CHARTYPE>
mad_flow CMp3Decoder<CHARTYPE>::mad_input_func(void *data, struct mad_stream *stream){
	struct _TMP_DATA<CHARTYPE> &tmp_data = *(struct _TMP_DATA<CHARTYPE> *)data;

	register unsigned char *p = tmp_data.mp3_data;
	memcpy(tmp_data.mp3_data, stream->next_frame, stream->bufend - stream->next_frame);
	p += stream->bufend - stream->next_frame;

	tmp_data.ifs.read((char *)p, (int)(buf_size - (p - tmp_data.mp3_data)));
	size_t count = tmp_data.ifs.gcount();

	// 将数据送入 mad 流，去解码
	mad_stream_buffer(stream, (unsigned char *)tmp_data.mp3_data, (unsigned long)(count + (p - tmp_data.mp3_data)));

	if (count == 0)
		return MAD_FLOW_STOP;
	return MAD_FLOW_CONTINUE;
}

template <class CHARTYPE>
mad_flow CMp3Decoder<CHARTYPE>::mad_header_func(void *data, struct mad_header const *header){
	return MAD_FLOW_CONTINUE;
}

template <class CHARTYPE>
mad_flow CMp3Decoder<CHARTYPE>::mad_output_func(void *data, struct mad_header const *header, struct mad_pcm *pcm){
	struct _TMP_DATA<CHARTYPE> &tmp_data = *(struct _TMP_DATA<CHARTYPE> *)data;

	if (tmp_data.pcm->m_nSamples == 0){
		// 第一次读取到 mp3 信息

		tmp_data.pcm->m_nSamplesPerSec = pcm->samplerate;
		tmp_data.pcm->m_nChannels = pcm->channels;
	}

	// 更新大小
	tmp_data.pcm->m_nSamples += pcm->length;

	// 插入数据
	tmp_data.data[0].insert(tmp_data.data[0].end(), pcm->samples[0], pcm->samples[0] + pcm->length);
	tmp_data.data[1].insert(tmp_data.data[1].end(), pcm->samples[1], pcm->samples[1] + pcm->length);

	assert(tmp_data.pcm->m_nSamples == tmp_data.data[0].size());
	assert(tmp_data.pcm->m_nSamples == tmp_data.data[1].size());

	return MAD_FLOW_CONTINUE;
}

template <class CHARTYPE>
mad_flow CMp3Decoder<CHARTYPE>::mad_error_func(void *data, struct mad_stream *stream, struct mad_frame *frame){
	struct _TMP_DATA<CHARTYPE> &tmp_data = *(struct _TMP_DATA<CHARTYPE> *)data;

	if (stream->error){
		if(!MAD_RECOVERABLE(stream->error)){
			tmp_data.error_code = stream->error;
			assert(0);
			return MAD_FLOW_STOP;
		} else{
			//assert(0);
			return MAD_FLOW_CONTINUE;
		}
	}
	return MAD_FLOW_CONTINUE;
}

