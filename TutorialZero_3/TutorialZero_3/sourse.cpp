#include <iostream>
#include <graphics.h>
#include <string>
#include <vector>
#include "head.h"

bool running = true;

bool is_game_started = false;

int idx_current_anim = 0;//储存当前动画的帧索引

const int PLAYER_ANIM_NUM = 6;//定义动画帧总数为常量6



const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BUTTON_WIDTH = 193;
const int BUTTON_HEIGHT = 75;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;



//表示动画所使用的图集
class Atlas
{
public:
	Atlas(LPCTSTR path, int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}

public:
	std::vector<IMAGE*> frame_list;
};

Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;


//动画类
class Animation
{
public:
	Animation(Atlas* atlas,int interval)
	{
		anim_atlas = atlas;
		interval_ms = interval;
		
	}

	~Animation() = default;
	

	void Play(int x,int y,int delta /*表示距离上一次调用函数时间过去了多久*/)
	{
		timer += delta;

		if (timer >= interval_ms)
		{

			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;

			

		}
		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
		
	}

private:
	int timer = 0;//动画计时器
	int idx_frame = 0;//动画帧索引
	int interval_ms = 0;//帧间隔
	
private:
	Atlas* anim_atlas;
};

//按钮类
class Button
{
public:
	Button(RECT rect,LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	//事件处理部分
	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x,msg.y))
			{
				status = Status::Hovered;
			}
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Idle;
			}
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Pushed;
			}
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
			{
				OnClick();
			}
			break;
		default:
			break;
		}
		

			
	}

	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}




	~Button()
	{

	}

protected:
	virtual void OnClick() = 0;

private:
	enum class Status
	{
		Idle = 0,
		Hovered,
		Pushed
	};

private:
	RECT region;//描述自己的位置与大小
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;//按钮状态枚举

private:
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

//开始游戏按钮
class StartGameButton : public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		: Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
	{

	}

	~StartGameButton()
	{

	}

protected:
	void OnClick()
	{
		is_game_started = true;

		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);//博放音乐
	}
};

//结束游戏按钮
class QuitGameButton : public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		: Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
	{

	}

	~QuitGameButton()
	{

	}

protected:
	void OnClick()
	{
		running = false;
	}
};

class Bullet
{
public:
	POINT position = { 0,0 };

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

class Player
{
public:
	const int PLAYER_WIDTH = 80; //玩家宽度
	const int PLAYER_HEIGHT = 80;//玩家高度

public:
	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(atlas_player_left, 45);
		anim_right = new Animation(atlas_player_right, 45);
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
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);


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
		//将子弹等效为点，判断点是否在敌人所在的矩形中
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;


		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		//将敌人位置等效为一个点，判断敌人的点是否在玩家的矩形中
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
		POINT player_pos = player.GetPosition();

		bool is_overlap_x = check_position.x >=player_pos.x && check_position.x <= player_pos.x + player.PLAYER_WIDTH;
		bool is_overlap_y = check_position.y >= player_pos.y && check_position.y <= player_pos.y + player.PLAYER_HEIGHT;

		return is_overlap_x && is_overlap_y;
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


	void Hurt()
	{
		alive = false;
	}

	bool CheckAlive()
	{
		return alive;
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
	bool alive = true;
};

void DrawPlayerScore(int score)
{
	static TCHAR text[64];
	_stprintf_s(text,_T("SCORE:%d"),score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255,85,185));
	outtextxy(10,10,text);
}

#pragma comment(lib, "MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

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

//更新子弹位置
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	const double RADIAL_SPEED = 0.0045;//径向波动速度
	const double TANGENT_SPEED = 0.0055;//切向波动速度
	double radian_interval = 2 * 3.14159 / bullet_list.size(); //子弹之间的弧度
	POINT player_pos = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;//当前子弹所在弧度
		bullet_list[i].position.x = player_pos.x + player.PLAYER_WIDTH / 2 + (int)(radius* sin(radian));
		bullet_list[i].position.y = player_pos.y + player.PLAYER_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}




int main()
{
	initgraph(1280,720);//绘制窗口大小


	atlas_player_left = new Atlas(_T("img/paimon_left_%d.png"), 6);
	atlas_player_right = new Atlas(_T("img/paimon_right_%d.png"), 6);
	atlas_enemy_left = new Atlas(_T("img/boar_left_%d.png"), 6);
	atlas_enemy_right = new Atlas(_T("img/boar_right_%d.png"), 6);


	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);//加载音乐
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);

	

	

	ExMessage msg;//输入列表
	IMAGE img_background;
	IMAGE img_menu;
	Player player;
	int score = 0;//记录玩家得分
	std::vector<Enemy*> enemy_list;//敌人列表
	std::vector<Bullet> bullet_list(3);//子弹列表

	RECT region_btn_start_game, region_btn_quit_game;//开始按钮与退出按钮的位置
	//设置开始按钮与结束游戏按钮
	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
		_T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	loadimage(&img_menu, _T("img/menu.png"));
	loadimage(&img_background,_T("img/background.png"));//加载图片位置
	
	//LoadAnimation();


	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();//动态延时功能

		while (peekmessage(&msg))
		{
			if (is_game_started)
			{
				player.ProcessEvent(msg);
			}
			else
			{
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}

		if (is_game_started)
		{

			player.Move();

			UpdateBullets(bullet_list, player);

			TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
			{
				enemy->Move(player);
			}

			//检测敌人与玩家的碰撞
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(player))
				{
					static TCHAR text[128];
					_stprintf_s(text, _T("Final Score: %d"), score);
					MessageBox(GetHWnd(), _T("You Lose"), _T("Game Over"), MB_OK);
					running = false;
					break;
				}

			}

			//检测子弹与敌人的碰撞
			for (Enemy* enemy : enemy_list)
			{
				for (const Bullet& bullet : bullet_list)
				{
					if (enemy->CheckBulletCollision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);//博放音乐
						enemy->Hurt();
						score++;
					}
				}
			}

			//移除生命值归零的敌人
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					std::swap(enemy_list[i], enemy_list.back());//与对末尾的元素交换位置
					enemy_list.pop_back();//讲最后一个元素从vector中弹出
					delete enemy;
				}

			}
		}


		cleardevice();

		if (is_game_started)
		{
			putimage(0, 0, &img_background);//放置背景图片
			player.Draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
			{
				enemy->Draw(1000 / 144);
			}
			for (const Bullet& bullet : bullet_list)
			{
				bullet.Draw();
			}

			DrawPlayerScore(score);//绘制玩家得分
		}
		else
		{
			putimage(0,0,&img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}

		

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}

	}

	delete atlas_player_left;
	delete atlas_player_right;
	delete atlas_enemy_left;
	delete atlas_enemy_right;

	EndBatchDraw();

	return 0;

}