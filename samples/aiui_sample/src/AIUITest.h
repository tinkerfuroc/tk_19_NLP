/*
* AIUIAgentTest.h
*
*  Created on: 2017年3月9日
*      Author: hj
*/

#ifndef AIUIAGENTTEST_H_
#define AIUIAGENTTEST_H_

#include "aiui/AIUI.h"


#include <string>
#include <iostream>

#include "FileUtil.h"

#include <unistd.h>
#include <pthread.h>

#define TEST_ROOT_DIR "./AIUI/"

//配置文件打的路径，里面是客户端设置的参数
#define CFG_FILE_PATH "./AIUI/cfg/aiui.cfg"

//测试音频的路径
#define TEST_AUDIO_PATH "./AIUI/audio/test.pcm"

#define GRAMMAR_FILE_PATH "./AIUI/asr/call.bnf"

#define LOG_DIR "./AIUI/log"


using namespace aiui;
using namespace std;


//写测试音频线程
class WriteAudioThread
{
private:
	IAIUIAgent* mAgent;

	string mAudioPath;

	bool mRepeat;

	bool mRun;

	FileUtil::DataFileHelper* mFileHelper;

  pthread_t thread_id;
  bool thread_created;

private:
	bool threadLoop();

  static void* thread_proc(void * paramptr);

public:
	WriteAudioThread(IAIUIAgent* agent, const string& audioPath, bool repeat);

	~WriteAudioThread();

	void stopRun();

	bool run();

};


//监听器，SDK通过onEvent（）回调抛出状态，结果等信息
class TestListener : public IAIUIListener
{
private:
	FileUtil::DataFileHelper* mTtsFileHelper;

public:
	void onEvent(const IAIUIEvent& event) const;

	TestListener();

	~TestListener();
};


//测试主类
class AIUITester
{
private:
	IAIUIAgent* agent;

	TestListener listener;

	WriteAudioThread * writeThread;

	string encode(const unsigned char* bytes_to_encode, unsigned int in_len);

public:
	AIUITester() ;

	~AIUITester();
private:

	void showIntroduction(bool detail);
	//创建AIUI 代理，通过AIUI代理与SDK发送消息通信
	void createAgent();

	//唤醒接口
	void wakeup();

	//开始AIUI，调用stop()之后需要调用此接口才可以与AIUI继续交互
	void start();

	//停止AIUI
	void stop();

	//写音频接口，参数表示是否重复写
	void write(bool repeat);

	void stopWriteThread();

	void reset();

	//写文本接口
	void writeText();

	void syncSchema();

	void querySyncStatus();

	void buildGrammar();

	void updateLocalLexicon();

	//开始tts
	void startTts();

	//暂停tts
	void pauseTts();

	//继续上次的tts
	void resumeTts();

	//取消本次tts
	void cancelTts();

	void destory();

public:
	void readCmd();

	void test();
};



#endif /* AIUIAGENTTEST_H_ */
