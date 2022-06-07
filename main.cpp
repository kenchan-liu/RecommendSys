#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include<Windows.h>
using namespace std;

struct Item
{
	int IId;
	double Score;
};

//查看train文件，得到用户总数
int NumOfUser = 19835;
//查看train文件，得到item的最大值
int Items[19835];
//test文件的目标
int TestContent[19835][6];
//原始数据矩阵
Item** Data = new Item * [NumOfUser];
//由原始数据转化的皮尔逊矩阵
Item** PearSun = new Item * [NumOfUser];
double u = 0;


//item id的比对函数，在排序中会用到
bool compare(Item a, Item b) {return a.IId < b.IId;}


//用户x对所有产品打分的平均值
double AverageScoOfUserOnAllItem(int UserID)
{
	double Scores = 0;
	for (int i = 0; i < Items[UserID]; i++) {
		Scores += Data[UserID][i].Score;
	}
	double Average = Scores / Items[UserID];
	return Average;
}

//产品i的所有评分的平均值
double AverageScoOfItem(int ItemID)
{
	double Scores = 0;
	int Itemcount = 0;
	for (int i = 0; i < NumOfUser; i++) {
		for (int j = 0; j < Items[i]; j++) {
			if(Data[i][j].IId==ItemID){
				Scores += Data[i][j].Score;
				Itemcount++;
			}
		}
	}
	double Average = Scores / Itemcount;
	return Average;
}

//求所有Item平均值
double AverageScoOfAllItem() {
	double Scores = 0;
	int count = 0;
	for (int i = 0; i < NumOfUser; i++){
		for (int j = 0; j < Items[i]; j++)
		{
			Scores += Data[i][j].Score;
			count++;
		}
	}
	double Average = Scores / count;
	return Average;
}
void Data2Pear()
{
	for (int i = 0; i < NumOfUser; i++)
	{
		double avguser = AverageScoOfUserOnAllItem(i);
		PearSun[i] = new Item[Items[i]];
		for (int j = 0; j < Items[i]; j++) {
			PearSun[i][j].IId = Data[i][j].IId;
			//数值就是Sco减去用户对于某个Item的平均得分
			PearSun[i][j].Score = Data[i][j].Score - avguser;
		}
	}
}

//计算用户A和用户B的相似度
double SimOfAB(int UserA, int UserB)
{
	double guodushuzu[10000];//用来存放一些要使用的数据
	int count = 0;
	if (Items[UserA] <= Items[UserB])
	{
		for (int i = 0; i < Items[UserA]; i++)
		{
			//如果在皮尔森矩阵中发现了对应的ID，就要计算相应的值
			for (int q = 0; q < Items[UserB]; q++) {
				if (PearSun[UserA][i].IId == PearSun[UserB][q].IId) {
					guodushuzu[count] = PearSun[UserA][i].Score * PearSun[UserB][q].Score;
					count++;
				}
			}
		}
	}
	if (Items[UserA] > Items[UserB]) {
		for (int i = 0; i < Items[UserB]; i++) {
			for (int q = 0; q < Items[UserA]; q++) {
				//如果在皮尔森矩阵中发现了对应的ID，就要计算相应的值
				if (PearSun[UserB][i].IId == PearSun[UserA][q].IId) {
					guodushuzu[count] = PearSun[UserB][i].Score * PearSun[UserA][q].Score;
					count++;
				}
			}
		}
	}
	double PearSenguodu = 0;//作为分子
	double SumUserA = 0;
	double SumUserB = 0;
	for (int i = 0; i < count; i++)
		PearSenguodu += guodushuzu[i];
	for (int i = 0; i < Items[UserA]; i++)
		SumUserA += PearSun[UserA][i].Score * PearSun[UserA][i].Score;
	for (int i = 0; i < Items[UserB]; i++)
		SumUserB += PearSun[UserB][i].Score * PearSun[UserB][i].Score;
	double guodu = sqrt(SumUserA * SumUserB);//作为分母
	double pearson = PearSenguodu / guodu;
	return pearson;
}

//查找m在R中的位置，若不存在则返回-99
int Search(Item** R, int Uid, int n, int m){
	int low = 0, high = n - 1, mid;
	while (low <= high)
	{
		if (R[Uid][low].IId == m)
			return low;
		if (R[Uid][high].IId == m)
			return high;
		mid = low + ((high - low) / 2);
		if (R[Uid][mid].IId == m)
			return mid;
		if (R[Uid][mid].IId < m)
			low = mid + 1;
		else
			high = mid - 1;
	}
	if (low > high)
		return -99;
	return -99;
}
//最后产品中发生了Influence的情况的得分
double Influence(int uid, int iid, double u)
{
	double SumOfAll = 0;
	double SumOfPear = 0;
	for (int i = 0; i < NumOfUser; i++) {
		//如果在Data矩阵中找到了某个Item，就要进行下一步考量
		int j = Search(Data, i, Items[i], iid);
		if (j != -99) {
			double a = SimOfAB(uid, i);
			if (a > 0) //如果两用户是相似用户
			{
				SumOfAll += a;
				SumOfPear += a * (Data[i][j].Score - (AverageScoOfUserOnAllItem(i) + AverageScoOfItem(Data[i][j].IId) - u));
			}
		}
	}
	if (SumOfAll != 0 && SumOfPear != 0) {
		double rx = SumOfPear / SumOfAll;
		return rx;
	}
	else {
		//达到了阙值
		double rx = 999;
		return rx;
	}
}

//item的最终得分
double LastScore(int UserID, int ItemID, double u)
{
	double LastScore = 0;
	double r = Influence(UserID, ItemID, u);
	if (r == 999){
		//如果达到了阙值，就用平均分代替
		LastScore = AverageScoOfUserOnAllItem(UserID);
		return LastScore;
	}
	else{
		LastScore = r + AverageScoOfUserOnAllItem(UserID) + AverageScoOfItem(ItemID) - u;
		//数据发生错误时进行矫正
		if (LastScore > 100)
			LastScore = 100;
		if (LastScore < 0)
			LastScore = 0;
		return LastScore;
	}
}

//读入测试数据
void TestIn()
{
	ifstream infile;
	infile.open("test.txt");
	assert(infile.is_open());
	string s;
	string blank = " ";
	string line = "|";
	int uid = -1;
	int r = -1;
	int i = 0;
	while (getline(infile, s)) {
		int LocLine = s.find(line);
		if (s == "")
			break;
		if (LocLine != -1) {
			r++;
			i = 0;
		}
		else {
			TestContent[r][i] = stoi(s);
			i++;
		}
	}
	infile.close();
}

//写入结果文档（result.txt）
//使用多线程，实现cpu的多核利用
void WriteFinal()
{
	fstream file1;
	file1.open("result3.txt");
	for (int i = 0; i < 6000; i++)
	{
		file1 << i << "|" << "6" << endl << TestContent[i][0] << "  " << int(LastScore(i, TestContent[i][0], u)) << endl;
		for (int j = 1; j < 6; j++) {
			file1 << TestContent[i][j] << "  " << int(LastScore(i, TestContent[i][j], u)) << endl;
		}
	}
	file1.close();
}

void WriteFinal1()
{
	fstream file1;
	file1.open("result4.txt");
	for (int i = 6000; i < 12000; i++)
	{
		file1 << i << "|" << "6" << endl << TestContent[i][0] << "  " << int(LastScore(i, TestContent[i][0], u)) << endl;
		for (int j = 1; j < 6; j++) {
			file1 << TestContent[i][j] << "  " << int(LastScore(i, TestContent[i][j], u)) << endl;
		}
	}
	file1.close();
}
void WriteFinal2()
{
	fstream file1;
	file1.open("result5.txt");
	for (int i = 12000; i < 19835; i++)
	{
		file1 << i << "|" << "6" << endl << TestContent[i][0] << "  " << int(LastScore(i, TestContent[i][0], u)) << endl;
		for (int j = 1; j < 6; j++) {
			file1 << TestContent[i][j] << "  " << int(LastScore(i, TestContent[i][j], u)) << endl;
		}
	}
	file1.close();
}

int main()
{
	HANDLE hThread;
	DWORD ThreadID;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestIn, NULL, 0, &ThreadID);
	//录入数据到原始矩阵

	//首先打开train.txt

	ifstream infile1;
	infile1.open("train.txt");
	string s;
	//判断标准有两个，一个就是userid和itemid中间有个|
	//还有就是item和评分之间有空格
	string blank = " ";
	string l = "|";
	//user是从i=0开始的
	int uid = -1;
	int NumOfItem = 0;
	int c = 0;
	while (getline(infile1, s))
	{
		int line = s.find(l);
		if (s == "")
			break;
		if (line != -1) {
			uid++;
			c = 0;
			int len = s.length();
			string ss = s.substr(line + 1, len - line);
			NumOfItem = stoi(ss);
			Items[uid] = NumOfItem;
			Data[uid] = new Item[NumOfItem];
		}
		else {
			int f = s.find(blank);
			string SIID = s.substr(0, f);
			int leng = s.length();
			string sco = s.substr(f + 2, leng - f);
			double score = stof(sco);
			int IID = stoi(SIID);
			struct Item MS = { IID,score };
			Data[uid][c] = MS;
			c++;
		}
	}
	infile1.close();
	cout << "train.txt读取完毕，存入data矩阵中" << endl;
	u = AverageScoOfAllItem();
	cout <<"total average:"<< u;
	for (int i = 0; i < NumOfUser; i++) {
		sort(Data[i], Data[i] + Items[i], compare);
	}
	Data2Pear();

	WriteFinal();
	HANDLE writethread1;
	DWORD ThreadID1;
	writethread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WriteFinal1, NULL, 0, &ThreadID1);
	HANDLE writethread2;
	DWORD ThreadID2;
	writethread2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WriteFinal2, NULL, 0, &ThreadID2);
	return 0;
}