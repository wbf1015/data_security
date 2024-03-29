#include <stdio.h>
#include <string.h>
#include "laplace.h"
#include "csvpackage.h"
#include <time.h>
#include <math.h>
extern int rand();
extern void srand(unsigned);
/*
函数功能：	对传入的csv文件进行处理，提取其中数据并生成拉普拉斯分布的噪音进行加噪
输入参数说明：
path		csv文件的存储位置
beta		拉普拉斯分布参数
seed	    长整型指针变量， *seed 为伪随机数的种子
*/
void csv_analysis(char* path, double beta, long int seed)
{
	FILE *original_file = fopen(path,"r+"); //读取指定路径的数据集
	struct Animals * original_data = NULL;
	original_data = csv_parser(original_file);
	int sum=0,i=0;
	double x = 0;
	while(original_data[i].name)  //循环为原始数据集内各条数据去除标识（动物名）、生成拉普拉斯噪音并加噪
	{
		x = laplace_data(beta,&seed); //产生拉普拉斯随机数
		printf("Added noise:%f\t%s %d\t%f\n",x,"Animal",i+1,original_data[i].carrots+x); //此处分别列出了每条具体添加的噪音和加噪的结果。当投入较少预算时，可能会出现负数
		if(original_data[i].carrots+x>=55)
		{
			sum++;
		}
		i++;
	}
	printf("Animals which carrots cost > 55 (Under DP): %d\n",sum); //输出加噪后的数据集中，每日食用胡萝卜大于55的动物个数
}

int csv_analysis_Int_utils(char* path, double beta, long int seed){
	FILE *original_file = fopen(path,"r+"); //读取指定路径的数据集
	struct Animals * original_data = NULL;
	original_data = csv_parser(original_file);
	int sum=0,i=0;
	double x = 0;
	while(original_data[i].name)  //循环为原始数据集内各条数据去除标识（动物名）、生成拉普拉斯噪音并加噪
	{
		x = 0; //产生拉普拉斯随机数
		// printf("Added noise:%f\t%s %d\t%f\n",x,"Animal",i+1,original_data[i].carrots+x); //此处分别列出了每条具体添加的噪音和加噪的结果。当投入较少预算时，可能会出现负数
		if(original_data[i].carrots+x>=55)
		{
			sum++;
		}
		i++;
	}
	double noise = laplace_data(beta,&seed);
	sum = sum+round(noise);
	printf("Animals which carrots cost > 55 (Under DP): %d\n",sum); //输出加噪后的数据集中，每日食用胡萝卜大于55的动物个数
	return sum;
}


/*
函数功能：	对传入的csv文件进行处理，提取其中数据并生成拉普拉斯分布的噪音进行加噪,进行交互式DP
			也就是说可以进行多次查找
输入参数说明：
path		csv文件的存储位置
beta		拉普拉斯分布参数
seed	    长整型指针变量， *seed 为伪随机数的种子
time		想要查找的次数
*/

void csv_analysis_Int(char* path, char* path2, double beta, long int seed,int time){
	double avg_origin = 0;
	double avg_neighbor = 0;
	for(int i=1;i<=time;i++){
		int no_use;
		printf("please input anything for next searching: ");
		scanf("%d", &no_use);
		printf("this is  round %d for searching\n",i);
		avg_origin+=csv_analysis_Int_utils(path, beta*time, rand()%10000+10000+seed);
		printf("==================Using neighbour dataset==================\n");
		int seed_=seed+i;
		avg_neighbor+=csv_analysis_Int_utils(path2, beta*time, rand()%10000+10000+seed);
	}
	printf("avg count for origin dataset is %f\n",avg_origin/(double)time);
	printf("avg count for neighbor dataset is %f\n",avg_neighbor/(double)time);
}

/*
参数表：
seed	    长整型指针变量， *seed为伪随机数的种子
sen			数据集的敏感度
x			用于储存拉普拉斯分布噪音的临时变量
beta		隐私预算，在输入后根据公式转换为拉普拉斯分布参数
*/
void woIntDP(){
	long int seed;
	int sen = 1;  //对于一个单属性的数据集，其敏感度为1
	double beta;
	srand((unsigned)time( NULL )); //生成基于时间的随机种子（srand方法）
	beta = 0;
	printf("Please input laplace epsilon: ");
	scanf("%lf", &beta);
	if(beta<=0 || !beta)//当输入的beta值无效时，默认设定beta值为1
	{
		beta = 1.0;
	} 
	printf("Under privacy budget %f, sanitized original data with fake animal name and laplace noise:\n",beta);
	beta = sen / beta; //拉普拉斯机制下，实际公式的算子beta为敏感度/预算
	seed = rand()%10000+10000; //随机种子产生
	csv_analysis("./zoo.csv",beta,seed); //先调用原始数据集
	printf("==================Using neighbour dataset==================\n");
	seed = rand()%10000+10000; //随机种子更新
	csv_analysis("./zoo_nb.csv",beta,seed); //再调用相邻数据集
}

void IntDP(){
	long int seed;
	int sen = 1;  //对于一个单属性的数据集，其敏感度为1
	double beta;
	int times;
	srand((unsigned)time( NULL )); //生成基于时间的随机种子（srand方法）
	beta = 0;
	printf("Please input laplace epsilon: ");
	scanf("%lf", &beta);
	printf("Please input searching rounds: ");
	scanf("%d", &times);
	if(beta<=0 || !beta)//当输入的beta值无效时，默认设定beta值为1
	{
		beta = 1.0;
	}
	if(times<=0){
		times=10;
	} 
	printf("Under privacy budget %f, sanitized original data with fake animal name and laplace noise:\n",beta);
	beta = sen / beta; //拉普拉斯机制下，实际公式的算子beta为敏感度/预算
	seed = rand()%10000+10000; //随机种子产生
	csv_analysis_Int("./zoo.csv","./zoo_nb.csv",beta,seed,times);
}

int main()
{	
	int c;
	printf("Please input 1 for no interaction else for interaction ");
	scanf("%d", &c);
	if(c==1){
		woIntDP();
	}else{
		IntDP();
	}
	return 0;
}

