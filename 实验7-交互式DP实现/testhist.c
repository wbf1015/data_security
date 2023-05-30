#include <stdio.h>
#include <string.h>
#include "laplace.h"
#include "csvpackage.h"
#include <time.h>
#include <stdbool.h>

extern int rand();
extern void srand(unsigned);
double Avg[5]={0.0};
double Avg_neighbor[5]={0.0};
int globalTime=0;
/*
函数功能：	对传入的csv文件进行处理，提取其中数据并生成拉普拉斯分布的噪音进行加噪
输入参数说明：
path		csv文件的存储位置
beta		拉普拉斯分布参数
seed	    长整型指针变量， *seed 为伪随机数的种子
*/
int csv_analysis(char* path, double beta, long int seed,bool isprint)
{
	FILE *original_file = fopen(path,"r+"); //读取指定路径的数据集
	struct Histobuckets * original_data = NULL;
	original_data = hb_csv_parser(original_file);
	int sum=0,i=0;
	double x = 0;
	
	while(original_data[i].bucket)  //循环为原始数据集内各桶数据生成拉普拉斯噪音并加噪
	{
		x = laplace_data(beta,&seed); //产生拉普拉斯随机数
		printf("Added noise:%f\t%s\t%f\n",x,original_data[i].bucket,original_data[i].count+x); //此处分别列出了每条具体添加的噪音和加噪的结果。当投入较少预算时，可能会出现负数
		if(path=="./medicaldata.csv"){
			Avg[i]+=original_data[i].count+x;
			i++;
		}else{
			Avg_neighbor[i]+=original_data[i].count+x;
			i++;
		}
		
    }
	if(isprint){
		printf("==========AVG VALUE=========\n");
		if(path=="./medicaldata.csv"){
			for(int i=0;i<5;i++){
				printf("%s\torigin_data=%d\tavg_data=%f\n",original_data[i].bucket,original_data[i].count,Avg[i]/(double)globalTime);
		}
		}else{
			for(int i=0;i<5;i++){
				printf("%s\torigin_data=%d\tavg_data=%f\n",original_data[i].bucket,original_data[i].count,Avg_neighbor[i]/(double)globalTime);
			}
		}
	}
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
	for(int i=1;i<time;i++){
		int no_use;
		printf("please input anything for next searching:");
		scanf("%d", &no_use);
		printf("this is  round %d for searching\n",i);
		csv_analysis(path, beta*time, rand()%10000+10000+seed,false);
		printf("==================Using neighbour dataset==================\n");
		int seed_=seed+1;
		csv_analysis(path2, beta*time, rand()%10000+10000+seed_,false);
	}
	int no_use;
		printf("please input anything for next searching:");
		scanf("%d", &no_use);
		printf("this is  round %d for searching\n",time);
		csv_analysis(path, beta*time, rand()%10000+10000+seed,true);
		printf("==================Using neighbour dataset==================\n");
		int seed_=seed+1;
		csv_analysis(path2, beta*time, rand()%10000+10000+seed_,true);
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
	double x, beta;
	srand((unsigned)time( NULL )); //生成基于时间的随机种子（srand方法）
	beta = 0;
	printf("Please input laplace epsilon:");
	scanf("%lf", &beta);
	if(beta<=0 || !beta) //当输入的beta值无效时，默认设定beta值为1
	{
		beta = 1.0;
	} 
	printf("Under privacy budget %f, sanitized original bucket with laplace noise:\n",beta);
	beta = sen / beta; //拉普拉斯机制下，实际公式的算子beta为敏感度/预算
	seed = rand()%10000+10000; //随机种子产生
	csv_analysis("./medicaldata.csv",beta,seed,false); //先调用原始数据集
    printf("==================Using neighbour dataset==================\n");
    seed = rand()%10000+10000; //随机种子更新
    csv_analysis("./md_nb.csv",beta,seed,false); //再调用相邻数据集
}

void IntDP(){
	long int seed;
	int sen = 1;  //对于一个单属性的数据集，其敏感度为1
	double beta;
	int times;
	srand((unsigned)time( NULL )); //生成基于时间的随机种子（srand方法）
	beta = 0;
	printf("Please input laplace epsilon:");
	scanf("%lf", &beta);
	printf("Please input searching rounds:");
	scanf("%d", &times);
	globalTime=times;
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
	csv_analysis_Int("./medicaldata.csv","./md_nb.csv",beta,seed,times);
}


int main()
{
	int c;
	printf("Please input 1 for no interaction else for interaction");
	scanf("%d", &c);
	if(c==1){
		woIntDP();
	}else{
		IntDP();
	}
	return 0;
}