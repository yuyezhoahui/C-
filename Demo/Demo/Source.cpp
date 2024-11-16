#include <graphics.h>

#include <iostream>

int main()
{

	initgraph(1280, 720);//显示窗口

	int x = 300;
	int y = 300;

	BeginBatchDraw();//新建缓冲区

	while (true)
	{
		ExMessage msg;//消息队列
		while (peekmessage(&msg))//获取消息队列中的消息
		{
			if (msg.message == WM_MOUSEMOVE)
			{
				//Z在这里处理移动逻辑
				x = msg.x;
				y = msg.y;
			}
		}
		cleardevice();//清空
		solidcircle(x,y,100);//画圆
		FlushBatchDraw();//切换缓冲区
	}
	EndBatchDraw();//双缓冲相关操作

	



	return 0;
}