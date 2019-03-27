#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "../../include/msp_cmn.h"
#include "../../include/qivw.h"
#include "../../include/msp_errors.h"

#define IVW_AUDIO_FILE_NAME "audio/awake.pcm"
#define FRAME_LEN	640 //16k采样率的16bit音频，一帧的大小为640B, 时长20ms


void sleep_ms(int ms)
{
	usleep(ms * 1000);
}

int cb_ivw_msg_proc( const char *sessionID, int msg, int param1, int param2, const void *info, void *userData )
{
	if (MSP_IVW_MSG_ERROR == msg) //唤醒出错消息
	{
		printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
	}
	else if (MSP_IVW_MSG_WAKEUP == msg) //唤醒成功消息
	{
		printf("\n\nMSP_IVW_MSG_WAKEUP result = %s\n\n", info);
	}
	return 0;
}

void run_ivw(const char *grammar_list, const char* audio_filename ,  const char* session_begin_params)
{
	const char *session_id = NULL;
	int err_code = MSP_SUCCESS;
	FILE *f_aud = NULL;
	long audio_size = 0;
	long real_read = 0;
	long audio_count = 0;
	int count = 0;
	int audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	char *audio_buffer=NULL;
	char sse_hints[128];
	if (NULL == audio_filename)
	{
		printf("params error\n");
		return;
	}

	f_aud=fopen(audio_filename, "rb");
	if (NULL == f_aud)
	{
		printf("audio file open failed! \n");
		return;
	}
	fseek(f_aud, 0, SEEK_END);
	audio_size = ftell(f_aud);
	fseek(f_aud, 0, SEEK_SET);
	audio_buffer = (char *)malloc(audio_size);
	if (NULL == audio_buffer)
	{
		printf("malloc failed! \n");
		goto exit;
	}
	real_read = fread((void *)audio_buffer, 1, audio_size, f_aud);
	if (real_read != audio_size)
	{
		printf("read audio file failed!\n");
		goto exit;
	}

	session_id=QIVWSessionBegin(grammar_list, session_begin_params, &err_code);
	if (err_code != MSP_SUCCESS)
	{
		printf("QIVWSessionBegin failed! error code:%d\n",err_code);
		goto exit;
	}

	err_code = QIVWRegisterNotify(session_id, cb_ivw_msg_proc,NULL);
	if (err_code != MSP_SUCCESS)
	{
		snprintf(sse_hints, sizeof(sse_hints), "QIVWRegisterNotify errorCode=%d", err_code);
		printf("QIVWRegisterNotify failed! error code:%d\n",err_code);
		goto exit;
	}
	while(1)
	{
		long len = 10*FRAME_LEN; //16k音频，10帧 （时长200ms）
		audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if(audio_size <= len)
		{
			len = audio_size;
			audio_stat = MSP_AUDIO_SAMPLE_LAST; //最后一块
		}
		if (0 == audio_count)
		{
			audio_stat = MSP_AUDIO_SAMPLE_FIRST;
		}

		printf("csid=%s,count=%d,aus=%d\n",session_id, count++, audio_stat);
		err_code = QIVWAudioWrite(session_id, (const void *)&audio_buffer[audio_count], len, audio_stat);
		if (MSP_SUCCESS != err_code)
		{
			printf("QIVWAudioWrite failed! error code:%d\n",err_code);
			snprintf(sse_hints, sizeof(sse_hints), "QIVWAudioWrite errorCode=%d", err_code);
			goto exit;
		}
		if (MSP_AUDIO_SAMPLE_LAST == audio_stat)
		{
			break;
		}
		audio_count += len;
		audio_size -= len;

		sleep_ms(200); //模拟人说话时间间隙，10帧的音频时长为200ms
	}
	snprintf(sse_hints, sizeof(sse_hints), "success");

exit:
	if (NULL != session_id)
	{
		QIVWSessionEnd(session_id, sse_hints);
	}
	if (NULL != f_aud)
	{
		fclose(f_aud);
	}
	if (NULL != audio_buffer)
	{
		free(audio_buffer);
	}
}


int main(int argc, char* argv[])
{
	int         ret       = MSP_SUCCESS;
	const char *lgi_param = "appid = 5c865702,work_dir = .";
	const char *ssb_param = "ivw_threshold=0:1450,sst=wakeup,ivw_res_path =fo|res/ivw/wakeupresource.jet";

	ret = MSPLogin(NULL, NULL, lgi_param);
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
		goto exit ;//登录失败，退出登录
	}
	printf("\n###############################################################################################################\n");
	printf("## 请注意，唤醒语音需要根据唤醒词内容自行录制并重命名为宏IVW_AUDIO_FILE_NAME所指定名称，存放在bin/audio文件里##\n");
	printf("###############################################################################################################\n\n");
	run_ivw(NULL, IVW_AUDIO_FILE_NAME, ssb_param); 

	sleep_ms(2000);
exit:
	printf("按任意键退出 ...\n");
	getchar();
	MSPLogout(); //退出登录
	return 0;
}
