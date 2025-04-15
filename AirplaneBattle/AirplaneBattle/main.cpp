#include<stdio.h>
#include<easyx.h>

int main() {

	//1.创建图形界面窗口
	initgraph(480, 700);//px像素：宽×高（按照图片大小）

	//2.贴图
	//定义变量
	IMAGE img;
	
	//赋值
	loadimage(&img, "images/bk.png");//加载图片：容器，路径
	//输出
	putimage(0, 0, &img);//输出图片：坐标（从左上角0，0开始），容器

	system("pause");
	return 0;
}