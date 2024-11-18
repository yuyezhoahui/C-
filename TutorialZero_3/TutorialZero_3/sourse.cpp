#include <iostream>
#include <graphics.h>
#include <string>
#include <vector>
#include "head.h"

int idx_current_anim = 0;//储存当前动画的帧索引

const int PLAYER_ANIM_NUM = 6;//定义动画帧总数为常量6

const int PLAYER_WIDTH = 80; //玩家宽度
const int PLAYER_HEIGHT = 80;//玩家高度
const int SHADOW_WIDTH = 32; //阴影宽度

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;

POINT player_pos = { 500,500 };//玩家位置
int PLAYER_SPEED = 5;

class Animation
{
public:
	Animation(LPCTSTR path,int num,int interval)
	{
		interval_ms = interval;

		TCHAR path_file[256];
		for (size_t i = 0; i < num ;i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame,path_file);
			frame_list.push_back(frame);
		}
	}

	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}

	void Play(int x,int y,int delta /*表示距离上一次调用函数时间过去了多久*/)
	{
		timer += delta;

		if (timer >= interval_ms)
		{

			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;

			

		}
		putimage_alpha(x, y, frame_list[idx_frame]);
		
	}

private:
	int timer = 0;//动画计时器
	int idx_frame = 0;//动画帧索引
	int interval_ms = 0;//帧间隔
	std::vector<IMAGE*> frame_list;
};


#pragma comment(lib, "MSIMG32.LIB")

inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

//void LoadAnimation()
//{
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
//	{
//		std::wstring path = L"img/paimon_left_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_left[i], path.c_str());
//	}
//
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
//	{
//		std::wstring path = L"img/paimon_left_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_right[i], path.c_str());
//	}
//
//}

Animation anim_left_player(_T("img/paimon_left_%d.png"), 6, 45);
Animation anim_right_player(_T("img/paimon_right_%d.png"), 6, 45);

void DrawPlayer(int delta, int dir_x)
{
	
	int pos_shadow_x = player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
	int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
	putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

	static bool facing_left = false;//判断玩家动画是否面向左侧
	if (dir_x < 0)
	{
		facing_left = true;
	}
	else if (dir_x > 0)
	{
		facing_left = false;
	}

	if (facing_left)
	{
		anim_left_player.Play(player_pos.x,player_pos.y,delta);
	}
	else
	{
		anim_right_player.Play(player_pos.x, player_pos.y, delta);
	}
}

int main()
{
	initgraph(1280,720);//绘制窗口大小

	bool running = true;

	ExMessage msg;//输入列表
	IMAGE img_background;

	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;



	loadimage(&img_shadow, _T("img/shadow_player.png"));
	loadimage(&img_background,_T("img/background.png"));//加载图片位置
	
	//LoadAnimation();


	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();//动态延时功能

		while (peekmessage(&msg))
		{
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.vkcode)
				{
				case VK_UP:
					is_move_up = true;
					break;
				case VK_DOWN:
					is_move_down = true;
					break;
				case VK_LEFT:
					is_move_left = true;
					break;
				case VK_RIGHT:
					is_move_right = true;
					break;
				}
			}
			else if (msg.message == WM_KEYUP)
			{
				switch (msg.vkcode)
				{
				case VK_UP:
					is_move_up = false;
					break;
				case VK_DOWN:
					is_move_down = false;
					break;
				case VK_LEFT:
					is_move_left = false;
					break;
				case VK_RIGHT:
					is_move_right = false;
					break;
				}
			}
		}


		//玩家移动
		if (is_move_up)
		{
			player_pos.y -= PLAYER_SPEED;
		}
		if (is_move_down)
		{
			player_pos.y += PLAYER_SPEED;
		}
		if (is_move_left)
		{
			player_pos.x -= PLAYER_SPEED;
		}
		if (is_move_right)
		{
			player_pos.x += PLAYER_SPEED;
		}



		//static int counter = 0;//记录当前动画帧一共播放了几个游戏帧
		//if (++counter % 5 == 0)//每5个游戏帧播放一个动画帧
		//{
		//	idx_current_anim++;
		//}

		////动画循环播放
		//idx_current_anim = idx_current_anim % PLAYER_ANIM_NUM;

		cleardevice();

		putimage(0, 0, &img_background);//放置背景图片
		//putimage_alpha(player_pos.x, player_pos.y, &img_player_left[idx_current_anim]);//派蒙序列动画
		DrawPlayer(1000 / 144, is_move_right - is_move_left);

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}

	}

	EndBatchDraw();

	return 0;

}