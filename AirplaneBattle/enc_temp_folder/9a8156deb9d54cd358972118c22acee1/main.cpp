#include<stdio.h>
#include<easyx.h>

int main() {

	//1.����ͼ�ν��洰��
	initgraph(480, 700);//�����

	//2.��ͼ
	//�������
	IMAGE img;
	loadimage(&img,"images/bk.png");//����ͼƬ��������·��
	putimage(0,0,&img);//���ͼƬ�����꣨�����Ͻ�0��0��ʼ��������
	//��ֵ

	//���


	system("pause");
	return 0;
}