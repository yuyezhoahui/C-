#include <iostream>
#include <graphics.h>
#include <string>
#include <vector>
#include "head.h"

int idx_current_anim = 0;//储存当前动画的帧索引

const int PLAYER_ANIM_NUM = 6;//定义动画帧总数为常量6



const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;




//动画类
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


class Player
{
public:
	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/paimon_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/paimon_right_%d.png"), 6, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
			case WM_KEYDOWN:
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
				break;

			case WM_KEYUP:
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
				break;
		}
	
	}

	void Move()
	{
		//玩家移动
		//确保斜着移动的速度与水平竖直的移动速度一致
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_pos.x += (int)(PLAYER_SPEED * normalized_x);
			player_pos.y += (int)(PLAYER_SPEED * normalized_y);

		}

		//校准玩家位置//确保玩家不会超出屏幕
		if (player_pos.x < 0) player_pos.x = 0;
		if (player_pos.y < 0) player_pos.y = 0;
		if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT) player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT;

	}

	void Draw(int delta)
	{
		int pos_shadow_x = player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;//判断玩家动画是否面向左侧
		int dir_x = is_move_right - is_move_left;
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
			anim_left->Play(player_pos.x, player_pos.y, delta);
		}
		else
		{
			anim_right->Play(player_pos.x, player_pos.y, delta);
		}
	}

	const POINT& GetPosition() const
	{
		return player_pos;
	}


private:

	int PLAYER_SPEED = 5;
	const int PLAYER_WIDTH = 80; //玩家宽度
	const int PLAYER_HEIGHT = 80;//玩家高度
	const int SHADOW_WIDTH = 32; //阴影宽度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT player_pos = { 500,500 };//玩家位置
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

};

class Enemy
{
public:
	Enemy()
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/boar_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/boar_right_%d.png"), 6, 45);


		// 敌人生成边界
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//将敌人随机放置在地图边缘处
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch(edge)
		{
			case SpawnEdge::Up:
				position.x = rand() % WINDOW_WIDTH;
				position.y = -FRAME_HEIGHT;
				break;
			case SpawnEdge::Down:
				position.x = rand() % WINDOW_WIDTH;
				position.y = WINDOW_HEIGHT;
				break;
			case SpawnEdge::Left:
				position.x = -FRAME_WIDTH;
				position.y = rand() % WINDOW_HEIGHT;
				break;
			case SpawnEdge::Right:
				position.x = WINDOW_WIDTH;
				position.y = rand() % WINDOW_HEIGHT;
				break;
			default:
				break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		return false;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		return false;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y*dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
		if (dir_x < 0)
		{
			facing_left = true;
		}
		if (dir_x > 0)
		{
			facing_left = false;
		}
	}

	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + FRAME_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
		{
			anim_left->Play(position.x,position.y,delta);
		}
		else
		{
			anim_right->Play(position.x,position.y,delta);
		}
	}


	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

private:
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;//敌人宽度
	const int FRAME_HEIGHT = 80;//敌人高度
	const int SHADOW_WIDTH = 48;//阴影宽度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = {0,0};
	bool facing_left = false;
};

class Bullet
{
public:
	POINT position = {0,0};

public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setlinecolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}

private:
	const int RADIUS = 10;
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

//生成敌人
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
	{
		enemy_list.push_back(new Enemy());
	}
}




int main()
{
	initgraph(1280,720);//绘制窗口大小

	bool running = true;

	ExMessage msg;//输入列表
	IMAGE img_background;
	Player player;
	std::vector<Enemy*> enemy_list;//敌人列表

	
	loadimage(&img_background,_T("img/background.png"));//加载图片位置
	
	//LoadAnimation();


	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();//动态延时功能

		while (peekmessage(&msg))
		{
			player.ProcessEvent(msg);
		}

		player.Move();

		TryGenerateEnemy(enemy_list);
		for (Enemy* enemy : enemy_list)
		{
			enemy->Move(player);
		}


		cleardevice();

		putimage(0, 0, &img_background);//放置背景图片
		player.Draw(1000/144);
		for (Enemy* enemy : enemy_list)
		{
			enemy->Draw(1000/144);
		}

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