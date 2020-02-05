#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>//包含对文件的操作函数
#include <ctime>//设置定时器使用
#include <algorithm>

#define Max 100  //限制字符串最大长度（包括属性名，以及属性的具体值）
#define Maxx 9973 //记录个数为1000,哈希表的长度为记录个数*10以内的最大素数
#define Inf 2010 //设置外排序时每一堆的最大长度

using namespace std;

struct Node { //哈希节点
	char right[Max]; //属性值
	char record[Max];//记录整个记录
	struct Node *next; //采用链地址法解决冲突
} node[Maxx];

struct Point { //快排节点
	char key[Max]; //记录属性值
	char totalstr[Max];//记录整个串
} point[Inf],ppoint[Inf];

char property[Max][Max]; //记录表的各个属性
bool Trag[Maxx];//哈希表的标记变量
bool flag[Max];//标记属性变量
int num; //属性的个数

bool cmp(const struct Point p,const struct Point q) { //sort提供的排序函数
	return strcmp(p.key,q.key)<0;
}

void Handle_Create(const char *Creatstr) { //建表操作
	char *index;
	int pivot=1;
	int numnum=0;
	index=strstr(Creatstr,"("); //匹配字符'('
	for(index=index+1; *index!='\0'; index++) {
		if(*index==',') { //若遇到逗号，说明为一个属性值，此时要记录该属性值
			property[pivot][numnum]='\0';
			//index=strstr(index,",");
			//if(index==NULL) //退出循环
			//	break;
			numnum=0;
			pivot++;
		} else if(*index==')') { //若遇到 ），说明为最后一个字符串，则直接加结尾符退出循环
			property[pivot][numnum]='\0';
			break; 
		} else
			property[pivot][numnum++]=*index;
	}
	num=pivot; //属性的个数
	printf("建表成功\n");
}

void Handle_Insert(const char *Insertstr) { //插入操作
	char *index=strstr(Insertstr,"values"); //匹配字符串"values"
	index=index+6;
	char temp[Max];
	int pivot=0;
	fstream out; //建立流对象
	out.open("d:\\HAPPY\\sqldata.txt",ios::out|ios::app); //关联文件sqldata,并设置打开方式为out|app，在原有文件的末尾插入记录
	for(index=index+2; *index!='\0'; index++) {
		if(*index=='\'' && *(index+1)==',') { //若匹配到字符'\'',并且不是最后一个属性值
			index+=2;
			temp[pivot]='\0';//增加串结尾符
			strcat(temp," ");//在末尾增加空格键
			out << temp; //插入到文件中
			pivot=0;
		} else if(*(index+1)==')') {
			temp[pivot]='\0'; //若为最后一个字符串，则不加空格符，直接加串结尾符
			out << temp;//插入到记录末尾
			break;
		} else
			temp[pivot++]=*index;
	}
	out << endl;//文件下一行
	printf("插入成功\n");
}

void Handle_Search(char *Searchstr) { //查询操作
	clock_t start, finish; //建立对象
	double totaltime;
	start = clock(); //clock():Current time of CPU，当前的时间
	memset(flag,0,sizeof(flag)); //初始化为均未标记
	char *index=Searchstr;
	index=index+6;
	char temp[Max],sym[Max];
	int pivot=0;
	//匹配最终要显示的属性
	for(index=index+1; *index!='\0'; index++) {
		if(*index==',' || *index==' ') {
			temp[pivot]='\0';
			for(int i=1; i<=num; i++) //将此次匹配的属性值标记为true
				if(strcmp(property[i],temp)==0) {
					flag[i]=1;
				}
			if(*index==' ')
				break;
			else
				pivot=0;
		} else
			temp[pivot++]=*index;
	}
	index=strstr(index,"where"); //匹配"where"
	index=index+5;
	pivot=0;
	//匹配要比较的属性
	for(index=index+1; *index!='='; index++)
		temp[pivot++]=*index;
	temp[pivot]='\0';
	pivot=0;
	//匹配要匹配的属性的值
	for(index=index+2; *index!='\''; index++)
		sym[pivot++]=*index;
	sym[pivot]='\0';
	ifstream in("d:\\HAPPY\\sqldata.txt");
	string line;
	char check[Max];
	char par[Max];
	pivot=0;
	for(int i=1; i<=num; i++) //输出最终要显示的属性值，作为第一行
		if(flag[i])
			printf("%s ",property[i]);
	printf("\n");
	char Output[Max][Max];
	bool trag;
	int rpivot;
	int Count;
	while(getline(in,line)) { //读取文件中的每行记录
		strcpy(check,line.c_str());//由string类型转化为char *类型
		trag=0;
		rpivot=1;
		Count=1;
		pivot=0;
		for(index=check; ; index++) {
			if(*index==' ' || *index=='\0') {
				par[pivot]='\0';
				if(flag[Count]) { //记录最终要输出的属性值
					strcpy(Output[rpivot],par);
					rpivot++;
				}
				if(strcmp(property[Count],temp)==0) { //判断是否符合where字句
					if(strcmp(sym,par)==0) {
						trag=1;//若符合，说明满足条件，置标记为true
					}
				}
				if(*index=='\0') {
					if(trag) { //若符合，则输出属性值
						for(int i=1; i<rpivot; i++)
							printf("%s ",Output[i]);
						printf("\n");
					}
					break;
				} else {
					Count++;
					pivot=0;
				}
			} else
				par[pivot++]=*index;
		}
	}
	finish=clock();//现在的时间
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC; //现在的时间-设置的初始时间=程序运行的时间，转化为s
	printf("Runtime is: %lf s\n",totaltime);

}

void Handle_Update(const char *Updatestr) { //更新操作
	char *index=strstr(Updatestr,"set");
	index+=4;
	int pivot=0;
	char obstr[Max],obsubstr[Max],sostr[Max],sosubstr[Max];
	for( ; *index!='='; index++) //匹配要设置的属性项
		obstr[pivot++]=*index;
	obstr[pivot]='\0';
	index+=2;
	pivot=0;
	for( ; *index!='\''; index++) //匹配要设置的属性项的值
		obsubstr[pivot++]=*index;
	obsubstr[pivot]='\0';
	pivot=0;
	index+=8;
	for( ; *index!='='; index++) //匹配where字句后的判断的属性名
		sostr[pivot++]=*index;
	sostr[pivot]='\0';
	pivot=0;
	index+=2;
	for( ; *index!='\''; index++) //匹配判断属性项的值
		sosubstr[pivot++]=*index;
	sosubstr[pivot]='\0';
	ifstream in("d:\\HAPPY\\sqldata.txt");//输入流对象，关联文件sqldata
	fstream out;
	out.open("d:\\HAPPY\\sqlhelpdata.txt",ios::out);//关联文件sqlhelpdata
	ofstream oout("d:\\HAPPY\\sqlhelpdata.txt");//输出流对象，关联文件sqlhelpdata
	char finally[Max],check[Max],par[Max];
	int Count;
	string line;
	bool trag;
	while(getline(in,line)) {
		strcpy(check,line.c_str());//将此次匹配的属性值标记为true
		finally[0]='\0';
		Count=1;
		pivot=0;
		trag=0;
		for(index=check; ; index++) {
			if(*index==' ' || *index=='\0') {
				par[pivot]='\0';
				if(strcmp(sostr,property[Count])==0 && strcmp(sosubstr,par)==0)//判断是否为更新的记录
					trag=1;
				if(strcmp(obstr,property[Count])==0)//若为要更新的属性值，则拼接属性要修改的值
					strcat(finally,obsubstr);
				else//否则为原值
					strcat(finally,par);
				if(*index=='\0') {
					if(trag) //若为要修改的值，则将修改后的记录写入到文件中
						oout << finally << endl;
					else//否则将未修改的串写入到文件中
						oout << check << endl;
					break;
				} else {
					strcat(finally," ");
					pivot=0;
					Count++;
				}
			} else
				par[pivot++]=*index;
		}
	}
	ifstream fin("d:\\HAPPY\\sqlhelpdata.txt");
	fstream ftest;
	ftest.open("d:\\HAPPY\\sqldata.txt",ios::out);
	while(getline(fin,line)) //拷贝文件，将文件sqlhelpdata中的内容拷贝到sqldata中
		ftest << line << endl;
	printf("修改成功\n");
}

void Handle_Delete(const char *Deletestr) { //删除操作
	char *index=strstr(Deletestr,"where");
	index+=6;
	char sym[Max];
	int pivot=0;
	for(; *index!='='; index++) //匹配where后要选择的属性
		sym[pivot++]=*index;
	sym[pivot]='\0';
	char result[Max];
	pivot=0;
	for(index+=2; *index!='\''; index++) //匹配要选择的属性值
		result[pivot++]=*index;
	result[pivot]='\0';
	ifstream in("d:\\HAPPY\\sqldata.txt");
	fstream out;
	out.open("d:\\HAPPY\\sqlhelpdata.txt",ios::out);
	ofstream oout("d:\\HAPPY\\sqlhelpdata.txt");
	char finally[Max],check[Max],par[Max];
	int Count;
	string line;
	while(getline(in,line)) {
		strcpy(check,line.c_str());
		finally[0]='\0';
		Count=1;
		pivot=0;
		for(index=check; ; index++) {
			if(*index==' ' || *index=='\0') {
				par[pivot]='\0';
				strcat(finally,par);
				if(strcmp(property[Count],sym)==0)//若为要删除的记录，则直接退出循环
					if(strcmp(result,par)==0)
						break;
				if(*index=='\0') {
					oout << finally << endl; //否则将原记录写入文件中
					break;
				} else {
					strcat(finally," ");
					Count++;
					pivot=0;
				}
			} else
				par[pivot++]=*index;
		}
	}
	ifstream fin("d:\\HAPPY\\sqlhelpdata.txt");
	fstream ftest;
	ftest.open("d:\\HAPPY\\sqldata.txt", ios::out);
	while(getline(fin,line)) //拷贝文件
		ftest << line << endl;
	printf("删除成功\n");
}
void Insert_hash(char *hashstr,char *totalstr) { //插入到哈希表中
	int sum=0;
	char *index=hashstr;
	for( ; *index!='\0'; index++)
		sum+=(*index-'0'); //以字符串中的每个字符的ASCLL码值之和作为关键字
	int key=sum%Maxx; //取得哈希表中的位置（下标）
	if(!Trag[key]) { //若在该哈希表位置还没有插入串
		Trag[key]=1;//设置该哈希表位置为已插入串
		strcpy(node[key].record,totalstr);//插入串
		strcpy(node[key].right,hashstr);
		node[key].next=NULL;//并设next指针为NULL
	} else { //若在该位置存在串，则采用链地址法解决冲突
		struct Node *pivot=&node[key]; //这里采用尾插法插入记录，便于后续查找的一致性
		for( ; pivot->next!=NULL; pivot=pivot->next); //找到最后一个节点
		struct Node *temp=(struct Node*)malloc(sizeof(struct Node));//动态申请一个节点，插入到最后
		strcpy(temp->record,totalstr);
		strcpy(temp->right,hashstr);
		temp->next=NULL;
		pivot->next=temp;
	}
}
void Hand_Index(const char *Indexstr) { //创建哈希索引
	char *index;
	index=strstr(Indexstr,"(");
	index+=1;
	char sym[Max];
	int pivot=0;
	for( ; *index!=')'; index++) //匹配要创建索引的属性项
		sym[pivot++]=*index;
	sym[pivot]='\0';
	char par[Max];
	char check[Max];
	string line;
	int Count;
	ifstream in("d:\\HAPPY\\sqldata.txt");
	memset(Trag,0,sizeof(Trag));
	while(getline(in,line)) { //顺序读取每条记录
		strcpy(check,line.c_str());
		pivot=0;
		Count=1;
		for(index=check; ; index++) {
			if(*index==' ' || *index=='\0') {
				par[pivot]='\0';
				if(strcmp(property[Count],sym)==0) { //若为要建立索引的属性项，则直接插入到哈希表中
					Insert_hash(par,check);
					break;
				} else {
					pivot=0;
					Count++;
				}
			} else
				par[pivot++]=*index;
		}
	}
	printf("创建索引成功\n");
}
void Hash_search(char *Hash_searchstr) {
	clock_t start, finish; //建立类对象
	double totaltime;
	start = clock(); //clock():Current time of CPU，设置初始时间
	memset(flag,0,sizeof(flag));//初始化为未标记
	char *index=Hash_searchstr;
	index=index+6;
	char temp[Max],sym[Max];
	int pivot=0;
	for(index=index+1; *index!='\0'; index++) {
		if(*index==',' || *index==' ') {
			temp[pivot]='\0';
			for(int i=1; i<=num; i++)
				if(strcmp(property[i],temp)==0) { //若为要最终输出的属性串，则设置相应标记为true
					flag[i]=1;
				}
			if(*index==' ')
				break;
			else
				pivot=0;
		} else
			temp[pivot++]=*index;
	}
	index=strstr(index,"where");
	index=index+5;
	pivot=0;
	for(index=index+1; *index!='='; index++)
		temp[pivot++]=*index; //匹配要判断的属性串
	temp[pivot]='\0';
	pivot=0;
	for(index=index+2; *index!='\''; index++)
		sym[pivot++]=*index;//匹配判断的属性值
	sym[pivot]='\0';
	for(int i=1; i<=num; i++)
		if(flag[i])
			printf("%s ",property[i]);//输出最终要显示的属性值
	printf("\n");
	int sum=0;
	for(index=sym; *index!='\0'; index++)
		sum+=(*index-'0'); //求哈希表的关键值
	int key=sum%Maxx;//求哈希表中的位置=key%prime
	if(!Trag[key]) //若该位置不存在字符串，则说明要查找的记录一定不存在
		printf("wrong search\n");
	else { //否则在链表中查找
		struct Node *piv=&node[key];
		for( ; piv!=NULL; piv=piv->next) { //顺序扫描链表，对每一个哈希节点进行判断
			if(strcmp(piv->right,sym)==0) { //若匹配成功，则进行相应处理，并输出
				char Output[Max][Max];
				char par[Max];
				int rpivot=1;
				int Count=1;
				pivot=0;
				for(char *index=piv->record; ; index++) {
					if(*index==' ' || *index=='\0') {
						par[pivot]='\0';
						if(flag[Count]) { //将要输出的属性字符串拼接起来
							strcpy(Output[rpivot],par);
							rpivot++;
						}
						if(*index=='\0') { //最终输出
							for(int i=1; i<rpivot; i++)
								printf("%s ",Output[i]);
							printf("\n");
							break;
						} else {
							Count++;
							pivot=0;
						}
					} else
						par[pivot++]=*index;
				}
			}
		}
	}
	finish=clock();//设置此时的时间
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC;//程序运行时间，即哈希查找时间
	printf("Runtime is: %lf s\n",totaltime);
}

void Merge(const struct Point *str1,int begin,int end,const struct Point *str2,int top,int bottom) {
	fstream out;
	out.open("d:\\HAPPY\\sortdata.txt",ios::out|ios::app);
	ofstream oout("d:\\HAPPY\\sortdata.txt");
	int pivot=begin,index=top;
	while(pivot<=end && index<=bottom) { //比较两个元素中关键值的大小
		if(strcmp(str1[pivot].key,str2[index].key)<0) { //将关键值小的一方记录写入到文件中
			oout << str1[pivot].totalstr << endl;
			pivot++;
		} else {
			oout << str2[index].totalstr << endl;
			index++;
		}
	}
	while(pivot<=end) { //处理剩余的记录,写入到文件中
		oout << str1[pivot].totalstr << endl;
		pivot++;
	}
	while(index<=bottom) { //处理剩余的记录，写入到文件中
		oout << str2[index].totalstr << endl;
		index++;
	}
}

void Handle_Sort(const char *Sortstr) {
	char *index=strstr(Sortstr,"Sort"); //匹配字符串"Sort"
	index+=6;
	char key[Max];
	int pivot=0;
	for( ; *index!='\''; index++)
		key[pivot++]=*index; //匹配排序依据属性名
	key[pivot]='\0';
	ifstream in("d:\\HAPPY\\sqldata.txt");
	string line;
	char check[Max],par[Max];
	int Count;
	int pnum=0,ppnum=0;
	bool ttrag=0; //设置标记变量，用来标记是在第一次快排序中，或者是在第二次快排序中
	while(getline(in,line)) { //顺序读取sqldata文件中的每一行记录
		strcpy(check,line.c_str());
		Count=1;
		pivot=0;
		if(!ttrag) { //若为第一次快排序
			for(index=check; ; index++) {
				if(*index==' ' || *index=='\0') {
					par[pivot]='\0';
					if(strcmp(property[Count],key)==0) { //如匹配到排序依据的属性项
						if(pnum>1000) { //若第一次快排序的元素个数已经满1000
							ttrag=1; //则替换到第二次快排序
							strcpy(ppoint[ppnum].key,par); //加入到第二次快排的数组中
							strcpy(ppoint[ppnum].totalstr,check);
							ppnum++;
							break;
						} else { //若第一次快排序的元素个数还未满1000，则直接加入到第一次快排序的数组中
							strcpy(point[pnum].key,par);
							strcpy(point[pnum].totalstr,check);
							pnum++;
							break;
						}
					} else {
						Count++;
						pivot=0;
					}
				} else
					par[pivot++]=*index;
			}
		} else { //若为第二次快排序
			for(index=check; ; index++) {
				if(*index==' ' || *index=='\0') {
					if(strcmp(property[Count],key)==0) {
						par[pivot]='\0'; //将元素加入到第二次快排序的数组中
						strcpy(ppoint[ppnum].key,par);
						strcpy(ppoint[ppnum].totalstr,check);
						ppnum++;
						break;
					} else {
						Count++;
						pivot=0;
					}
				} else
					par[pivot++]=*index;
			}
		}
	}
	sort(point,point+pnum,cmp); //进行第一次快排序
	sort(ppoint,ppoint+ppnum,cmp);//进行第二次快排序
	Merge(point,0,pnum-1,ppoint,0,ppnum-1);//将两次排序的结果合并成一个有序序列，并拷贝到文件中
	printf("排序成功\n");
}

void Init() { //初始化界面信息
	printf("=======================================================================\n");
	printf("hello!\n");
	printf("welcome to the EasySQL-DB\n");
	printf("EasySQL-DB：使用须知\n");
	printf("Create：若为建表SQL语句，则创建文件sqldata，并调用相关的处理函数\n");
	printf("Insert：若为插入SQL语句，则调用相应的插入处理函数\n");
	printf("Select：若为查询SQL语句，则调用相关的查询处理函数\n");
	printf("Update：若为修改SQL语句，则调用修改处理函数\n");
	printf("Delete：若为删除SQL语句，则调用删除处理函数\n");
	printf("Index：若为创建哈希索引SQL语句，则调用相关处理函数\n");
	printf("Sort：若为排序语句，则调用外排序函数\n");
	printf("sql end：若为结束语句，则退出循环，结束程序\n");
	printf("=======================================================================\n");
}
int main() {
	Init();
	char source[Max];
	bool Is_index=0;
	while(gets(source)) { //输入字符串（包括空格符）
		if(strstr(source,"Create")!=NULL) { //若为建表SQL语句，则创建文件sqldata，并调用相关的处理函数
			fstream out;
			out.open("d:\\HAPPY\\sqldata.txt", ios::out);
			Handle_Create(source);
		} else if(strstr(source,"Insert")!=NULL) //若为插入SQL语句，则调用相应的插入处理函数
			Handle_Insert(source);
		else if(strstr(source,"Select")!=NULL && !Is_index) //若为查询SQL语句，则调用相关的查询处理函数
			Handle_Search(source);
		else if(strstr(source,"Select")!=NULL && Is_index)//若为创建哈希表后的查询语句，则调用哈希查询处理函数
			Hash_search(source);
		else if(strstr(source,"Update")!=NULL) //若为修改SQL语句，则调用修改处理函数
			Handle_Update(source);
		else if(strstr(source,"Delete")!=NULL)//若为删除SQL语句，则调用删除处理函数
			Handle_Delete(source);
		else if(strstr(source,"Index")!=NULL) { //若为创建哈希索引SQL语句，则调用相关处理函数
			Hand_Index(source);
			Is_index=1;
		} else if(strstr(source,"Sort")!=NULL) //若为排序语句，则调用外排序函数
			Handle_Sort(source);
		else if(strstr(source,"sql end")!=NULL) //若为结束语句，则退出循环，结束程序
			break;
		else
			printf("格式错误\n");
	}
	return 0;
}
