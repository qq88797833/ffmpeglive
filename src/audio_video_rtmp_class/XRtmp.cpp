#include "XRtmp.h"
#include <iostream>
#include <string>

using namespace std;

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavutil/time.h>
}

#pragma comment(lib, "avformat.lib")

class CXRtmp : public XRtmp
{
public:
	void Close()
	{
		if (ic)
		{
			avformat_close_input(&ic);
			vs = NULL;
		}

		vc = NULL;
		url = "";
	}

	bool Init(const char *url)
	{
		///5 �����װ������Ƶ������ 
		//a ���������װ��������
		int ret = avformat_alloc_output_context2(&ic, 0, "flv", url);
		this->url = url;

		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			cout << buf;

			return false;
		}

		return true;
	}

	bool AddStream(const AVCodecContext *c)
	{
		if (!c) return false;
		
		//b ������Ƶ��, ����
		AVStream *st = avformat_new_stream(ic, NULL);
		if (!st)
		{
			cout<<"avformat_new_stream failed!"<<endl;
			return false;
		}

		st->codecpar->codec_tag = 0;//��������Ϊ0���������
		//�ӱ��������Ʋ���
		avcodec_parameters_from_context(st->codecpar, c);

		av_dump_format(ic, 0, url.c_str(), 1);

		if (c->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vc = c;
			vs = st;
		}
		else if (c->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ac = c;
			as = st;
		}

		return true;
	}

	bool SendHead()
	{
		///��rtmp���������IO
		int ret = avio_open(&ic->pb, url.c_str(), AVIO_FLAG_WRITE);
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			cout << buf << endl;

			return false;
		}

		/*д���װͷ ��һ�����ܻὫ������AVStream�е�timebase���ĵ���
		��˺���ʹ��ʱ,timebase��Ҫ���´�AVStream�л�ȡ,ʹ�øĺ��timebase���������ȷ�ģ�
		����ʹ��֮ǰ���õ�timebase
		*/
		ret = avformat_write_header(ic, NULL);
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			cout << buf << endl;

			return false;
		}

		return true;
	}

	bool SendFrame(AVPacket *pack)
	{
		if (!pack || pack->size <= 0 || !pack->data) return false;
		AVRational stime;
		AVRational dtime;

		//�ж���Ƶ������Ƶ
		if (vs && vc && pack->stream_index == vs->index)
		{
			stime = vc->time_base;
			dtime = vs->time_base;
		}
		else if (as && ac && pack->stream_index == as->index)
		{
			stime = ac->time_base;
			dtime = as->time_base;
		}
		else
		{
			return false;
		}
		//����
		/*
		�����������timebase���б仯��������ʹ��֡�ʽ��м���ġ�����ʱҪ��Ϊʹ������ʱ��timebase���½��м���
		*/
		pack->pts = av_rescale_q(pack->pts, stime, dtime);
		pack->dts = av_rescale_q(pack->dts, stime, dtime);
		pack->duration = av_rescale_q(pack->duration, stime, dtime);
		/*��������ڲ��л������򣬻����P֡B֡����ͬʱ�����Զ��ͷ�packet�Ŀռ䣬���ܳɹ����
		�����av_write_frame
		*/
		int ret = av_interleaved_write_frame(ic, pack);
		if (ret == 0)
		{
			cout << "#" << flush;
			return true;
		}
	}
private:
	//rtmp flv��װ��
	AVFormatContext *ic = NULL;

	//��Ƶ������
	const AVCodecContext *vc = NULL;

	//��Ƶ������
	const AVCodecContext *ac = NULL;

	//��Ƶ��
	AVStream *vs = NULL;
	//��Ƶ��
	AVStream *as = NULL;

	string url = "";
};

//������������
XRtmp *XRtmp::Get(unsigned char index)
{
	static CXRtmp cxr[255];
	static bool isFirst = true;
	if (isFirst)
	{
		//ע�����еķ�װ��
		av_register_all();

		//ע�����е�����Э��
		avformat_network_init();

		isFirst = false;
	}

	return &cxr[index];
}

XRtmp::XRtmp()
{
}


XRtmp::~XRtmp()
{
}