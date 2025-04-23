#include<stdio.h>
#include<easyx.h>
#include<conio.h>
#include<time.h>
#define MAX 10	//定时器最大个数

enum My {
	WIDTH = 480,	//宽度
	HEIGHT = 700,	//高度
	BULLET_NUM = 30,//子弹数量
	BIG,			//大敌机
	SMALL,			//小敌机
	ENEMY_NUM = 5,  //敌机数量
};

struct plane {//飞机结构体（自己、敌机、子弹）
	int x;//位置x
	int y;//位置y
	bool live;//是否存活
	int hp;//血量
	int type;//标记敌机大小
	int width;//宽
	int height;//高
};

plane MyPlane;//自己飞机
plane bullet[BULLET_NUM];//子弹
plane enemy[ENEMY_NUM];//敌机

//加载图片
IMAGE bk;//背景
IMAGE img_me[2];//玩家飞机
IMAGE img_bullet[2];//子弹
IMAGE img_enemy[2][2];//敌机
void Load();
void drawAlpha(IMAGE* picture, int  picture_x, int picture_y);// 载入PNG图并去透明部分
bool Timer(int ms, int id);//定时器
void init_game();//初始化游戏
void Draw();//绘制游戏
void Move();//飞机移动
void create_bullet();//创建子弹
void move_bullet();//子弹移动
void enemyHP(int i);//敌机血条
void create_enemy();//创建敌机
void move_enemy();//敌机移动
void play_game();//打飞机
void plane_collision();//飞机碰撞检测
void start_menu();//菜单、难度选择
void Difficulty();//难度设置
void game_loop();//游戏循环

//全局变量设置
int score = 0;//游戏得分
int difficulty = 1;//游戏难度
int PLANE_SPEED = 2,//飞机移动速度
BULLET_SPEED = 3,//子弹速度
BULLET_GEN = 600,//子弹生成速度
ENEMY_SPEED = 1,//敌机速度
ENEMY_GEN = 500;//敌机生成速度 
bool isPaused = false;//暂停
bool restartGame = false;//重新开始

int main() {

	//1.创建图形界面窗口
	initgraph(480, 700);//px像素：宽×高（按照图片大小）
	
	//2.贴图
	//定义变量、赋值
	//输出
	Draw();
	
	while (1) {
		FlushBatchDraw();//刷新
		init_game();
		start_menu();//菜单
		restartGame = false;
		isPaused = false;
		//3.游戏循环
		game_loop();
		if (!restartGame) break;     // 如果不是按R重启，说明是正常退出
		EndBatchDraw();
	}
	//system("pause");
	return 0;
}

//具体实现函数
void Load() {
	loadimage(&bk, "images/bk.png");//加载图片：容器，路径，宽，高
	loadimage(&img_me[0], "images/me1.png", 102, 126);//加载图片：容器，路径，宽，高
	loadimage(&img_me[1], "images/me2.png", 102, 126);//加载图片：容器，路径，宽，高
	loadimage(&img_bullet[0], "images/bullet1.png", 5, 11);//加载图片：容器，路径，宽，高
	loadimage(&img_bullet[1], "images/bullet2.png", 5, 11);//加载图片：容器，路径，宽，高
	loadimage(&img_enemy[0][0], "images/enemy1.png", 57, 43);//加载图片：容器，路径，宽，高
	loadimage(&img_enemy[0][1], "images/enemy1_down1.png", 57, 43);//加载图片：容器，路径，宽，高
	loadimage(&img_enemy[1][0], "images/enemy2.png", 69, 99);//加载图片：容器，路径，宽，高
	loadimage(&img_enemy[1][1], "images/enemy2_down2.png", 69, 99);//加载图片：容器，路径，宽，高
}
// 载入PNG图并去透明部分
void drawAlpha(IMAGE* picture, int  picture_x, int picture_y) //x为载入图片的X坐标，y为Y坐标
{

	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++) {
		for (int ix = 0; ix < picture_width; ix++) {
			int srcX = ix + iy * picture_width;
			int sa = ((src[srcX] & 0xff000000) >> 24);
			int sr = ((src[srcX] & 0xff0000) >> 16);
			int sg = ((src[srcX] & 0xff00) >> 8);
			int sb = src[srcX] & 0xff;

			// === 包括这里的 wrap-around 实现 ===
			int dst_ix = (ix + picture_x + graphWidth) % graphWidth;
			int dst_iy = (iy + picture_y + graphHeight) % graphHeight;
			int dstX = dst_ix + dst_iy * graphWidth;

			int dr = ((dst[dstX] & 0xff0000) >> 16);
			int dg = ((dst[dstX] & 0xff00) >> 8);
			int db = dst[dstX] & 0xff;

			draw[dstX] =
				((sr * sa / 255 + dr * (255 - sa) / 255) << 16) |
				((sg * sa / 255 + dg * (255 - sa) / 255) << 8) |
				(sb * sa / 255 + db * (255 - sa) / 255);
		}
	}

}
//定时器
bool Timer(int ms, int id) {
	static DWORD t[MAX];
	if (clock() - t[id] > ms) {
		t[id] = clock();
		return true;
	}
	return false;
}
//初始化游戏
void init_game() {
	Load();//加载图片
	//设置飞机位置在底部中心
	MyPlane.width = 102;
	MyPlane.height = 126;
	MyPlane.x = WIDTH / 2 - MyPlane.width / 2;
	MyPlane.y = HEIGHT - MyPlane.height;
	MyPlane.live = true;
	//初始化子弹
	for (int i = 0; i < BULLET_NUM; i++) {
		bullet[i].x = 0;
		bullet[i].y = 0;
		bullet[i].live = true;
	}
}

//绘制游戏
void Draw() {
	putimage(0, 0, &bk);//背景图片：坐标（从左上角0，0开始），容器
	//我的飞机图片：坐标（从左上角0，0开始），容器
	drawAlpha(&img_me[0], MyPlane.x, MyPlane.y);
	//子弹
	for (int i = 0; i < BULLET_NUM; i++) {
		if (bullet[i].live) {
			drawAlpha(&img_bullet[0], bullet[i].x, bullet[i].y);
		}
	}
	//敌机
	for (int i = 0; i < ENEMY_NUM; i++) {
		if (enemy[i].live) {
			if (enemy[i].type == BIG) {//大飞机
				drawAlpha(&img_enemy[1][0], enemy[i].x, enemy[i].y);
			}
			else {//小飞机
				drawAlpha(&img_enemy[0][0], enemy[i].x, enemy[i].y);
			}
		}
	}
	//显示分数
	char scoreText[50];//存文字
	settextcolor(WHITE);//颜色
	settextstyle(20, 0, _T("黑体"));//大小和字体
	sprintf_s(scoreText, sizeof(scoreText), "得分：%d", score);
	outtextxy(10, 10, scoreText);
}

//飞机移动
void Move() {
	//window API
	// 处理键盘输入（持续检测）
	if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
		MyPlane.y -= PLANE_SPEED;
	if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
		MyPlane.y += PLANE_SPEED;
	if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
		MyPlane.x -= PLANE_SPEED;
	if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
		MyPlane.x += PLANE_SPEED;

	// ESC 键退出
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)return;
	Sleep(10); // 稍微延迟一下，控制帧率
}

//创建子弹
void create_bullet() {
	for (int i = 0; i < BULLET_NUM; i++) {
		if (!bullet[i].live) {
			bullet[i].x = MyPlane.x + (51 - 2.5);//从飞机中心射出
			bullet[i].y = MyPlane.y;
			bullet[i].live = true;
			break;
		}
	}
}
//子弹移动
void move_bullet() {
	for (int i = 0; i < BULLET_NUM; i++) {
		if (bullet[i].live) {
			bullet[i].y -= BULLET_SPEED;
		}
		if (bullet[i].y < 0) {//子弹超出范围消失
			bullet[i].live = false;
		}
	}
}
//敌机血条
void enemyHP(int i) {
	int flag = rand() % 10;
	if (flag > 0 && flag < 2) {
		enemy[i].type = BIG;
		enemy[i].hp = 3;
		enemy[i].width = 69;
		enemy[i].height = 99;
	}
	else {
		enemy[i].type = SMALL;
		enemy[i].hp = 1;
		enemy[i].width = 57;
		enemy[i].height = 43;
	}
}

//创建敌机
void create_enemy() {
	for (int i = 0; i < ENEMY_NUM; i++) {
		if (!enemy[i].live) {
			enemy[i].x = rand() % (WIDTH - 51);//从随机位置出现
			enemy[i].y = 0;//从屏幕上方发出
			enemy[i].live = true;
			enemyHP(i);//设置敌机血量和类型
			break;
		}
	}
}
//敌机移动
void move_enemy() {
	for (int i = 0; i < ENEMY_NUM; i++) {
		if (enemy[i].live) {
			enemy[i].y += ENEMY_SPEED;
		}
		if (enemy[i].y > HEIGHT) {
			enemy[i].live = false;
		}
	}
}
//打飞机
void play_game() {
	for (int i = 0; i < ENEMY_NUM; i++) {
		if (!enemy[i].live) {//敌机不存在
			continue;
		}
		for (int j = 0; j < BULLET_NUM; j++) {
			if (!bullet[j].live) {//子弹不存在
				continue;
			}
			//敌机、子弹均存在，进行碰撞检测
			if (bullet[j].x > enemy[i].x && bullet[j].x<enemy[i].x + enemy[i].width &&
				bullet[j].y>enemy[i].y && bullet[j].y < enemy[i].y + enemy[i].height) {
				bullet[j].live = false;//消除子弹
				enemy[i].hp--;//扣一滴血
				//加分
				if (enemy[i].type == BIG) {
					score += 20;
				}
				else {
					score += 10;
				}
			}
			if (enemy[i].hp == 0) {
				enemy[i].live = false;//血条为0，清除敌机
			}
		}
	}
}
//飞机碰撞检测
void plane_collision() {
	for (int i = 0; i < ENEMY_NUM; i++) {
		if (!enemy[i].live)continue;
		if (MyPlane.x + MyPlane.width >= enemy[i].x && MyPlane.x <= enemy[i].x + enemy[i].width &&
			MyPlane.y + MyPlane.height >= enemy[i].y && MyPlane.y <= enemy[i].y + enemy[i].height) {
			MyPlane.live = false;
			exit(0);//退出游戏
		}
	}
}
void Difficulty() {
	if (difficulty == 1) {//简单
		BULLET_SPEED = 5;
		BULLET_GEN = 500;
		ENEMY_SPEED = 1;
		ENEMY_GEN = 400;
	}
	else if (difficulty == 2) {//困难
		BULLET_SPEED = 10;
		BULLET_GEN = 250;
		ENEMY_SPEED = 5;
		ENEMY_GEN = 200;
	}
	else if (difficulty == 3) {//地狱
		BULLET_SPEED = 30;
		BULLET_GEN = 100;
		ENEMY_SPEED = 10;
		ENEMY_GEN = 100;
	}
}
//菜单、难度选择
void start_menu() {
	cleardevice();
	settextstyle(40, 0, "黑体");
	settextcolor(WHITE);

	outtextxy(200, 100, "飞机大战");
	settextstyle(20, 0, "黑体");

	outtextxy(200, 200, "按 1 键：简单模式");
	outtextxy(200, 250, "按 2 键：困难模式");
	outtextxy(200, 300, "按 3 键：地狱模式");

	while (1) {
		if (_kbhit()) {
			char ch = _getch();
			if (ch == '1') {
				difficulty = 1;
				break;
			}
			else if (ch == '2') {
				difficulty = 2;
				break;
			}
			else if (ch == '3') {
				difficulty = 3;
				break;
			}
		}
		Difficulty();//设置子弹和敌机属性为对应难度等级
		Sleep(50); // 减少CPU占用
	}
}
void game_loop() {
	BeginBatchDraw();  // 开启双缓冲

	while (1) {
		// 按键判断：暂停或重新开始
		if (_kbhit()) {
			char ch = _getch();
			if (ch == 'p' || ch == 'P') {
				isPaused = !isPaused;
			}
			else if (ch == 'r' || ch == 'R') {
				restartGame = true;
				break;
			}
		}

		if (isPaused) continue;

		Draw();
		FlushBatchDraw();
		Move();

		if (Timer(BULLET_GEN, 0)) {
			create_bullet();
		}
		move_bullet();

		if (Timer(ENEMY_GEN, 1)) {
			create_enemy();
		}
		if (Timer(30, 2)) {
			move_enemy();
		}

		play_game();
		plane_collision();

		Sleep(10); // 控制帧率
	}

	EndBatchDraw(); // 结束双缓冲
}