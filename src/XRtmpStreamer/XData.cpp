#include "XData.h"
#include <stdlib.h>
#include<string.h>

//ȫ�ֺ�������ȡ��ǰʱ�䣬ʹ��ffmpeg�Ļ�ȡʱ��Ľӿ�
long long GetCurTime()
{
	return av_gettime();
}

void XData::Drop()
{
	if (data)
		delete data;
	data = 0;
	size = 0;
}

XData::XData(char *data, int size, long long p)
{
	this->data = new char[size];//���·���ռ䣬���ⲿ���ݿ��������ݰ���װ����
	memcpy(this->data, data, size);
	this->size = size;
	this->pts = p;
}

XData::XData()
{
}


XData::~XData()
{
}