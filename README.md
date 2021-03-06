支持SQL语句的简易内存数据库
===========================
该项目在不使用任何第三方的库的条件下实现了一个无事务，单线程，存在于内存的数据库。可以用于一定量数据的管理。

****
	
|Author|HDUZZB|
|---|---
|E-mail|hduzzb@126.com


****
## 目录
* [系统功能](#系统功能)
* [设计思路](#设计思路)
    * [SQL语句识别器](#SQL语句识别器)
        *  [建表(Create)](#建表(Create))
        *  [插入记录(Insert)](#插入记录(Insert))
        *  [查找记录(Search)](#查找记录(Search))
        *  [修改记录(Update)](#修改记录(Update))
        *  [删除记录(Delete)](#删除记录(Delete))
    * [索引结构](#索引结构)
    * [外排序](#外排序)
* [测试](#测试)
* [总结](#总结)

系统功能
-----------
该数据库主要实现如下功能：1.建表 2.插入记录 3.查询记录 4.删除记录 5.修改记录 6.对表中的记录按某一属性(任意)进行外排序 7.对表的某一属性(任意)建立哈希索引

设计思路
-----------
项目由SQL语句识别器、索引结构和外排序三部分组成。SQL语句识别器用于识别SQL语句，然后根据逻辑，去执行相应的操作。索引结构用来快速查询，降低查询时间。外排序则是用来对数据库中大量的数据进行快速有效的排序。

***

### SQL语句识别器
SQL语句识别器整合了建表、插入记录、查询记录、删除记录、修改记录等功能。主流程就是匹配用户输入的字符串，判断匹配的字符串是要执行哪部分功能，然后调用相应的模块函数去执行。这里给出相关代码：
```cpp
 //Show codes:
   int main(){
	Init();
	char source[Max];
	bool Is_index=0;
	while(gets(source)){ //输入字符串（包括空格符）
		if(strstr(source,"Create")!=NULL){ //若为建表SQL语句，则创建文件sqldata，并调用相关的处理函数
			fstream out;
			out.open("sqldata.txt", ios::out);
			Handle_Create(source);
		}
		else if(strstr(source,"Insert")!=NULL) //若为插入SQL语句，则调用相应的插入处理函数
			Handle_Insert(source);
		else if(strstr(source,"Select")!=NULL && !Is_index) //若为查询SQL语句，则调用相关的查询处理函数
			Handle_Search(source);
		else if(strstr(source,"Select")!=NULL && Is_index)//若为创建哈希表后的查询语句，则调用哈希查询处理函数
			Hash_search(source);
		else if(strstr(source,"Update")!=NULL) //若为修改SQL语句，则调用修改处理函数
			Handle_Update(source);
		else if(strstr(source,"Delete")!=NULL)//若为删除SQL语句，则调用删除处理函数
			Handle_Delete(source);
		else if(strstr(source,"Index")!=NULL){//若为创建哈希索引SQL语句，则调用相关处理函数
			Hand_Index(source);
			Is_index=1;
		}
		else if(strstr(source,"Sort")!=NULL) //若为排序语句，则调用外排序函数
			Handle_Sort(source);
		else if(strstr(source,"sql end")!=NULL) //若为结束语句，则退出循环，结束程序
			break;
                else
                        printf("格式错误\n");
	}
	return 0;
}		

```
这里强调一下，我们对于字符串的匹配是严格按照一定格式的，若不符合相应的格式则会提示格式错误信息。下面分别介绍一下每个功能的具体实现思想：

#### 建表(Create)
建表相对而言较为简单，当用户输入`Create`建表语句时，则调用`void Handle_Create(const char *Creatstr)`函数进行建表，该函数的功能除了建立一个文本文档外，还要对用户输入的字符串进行匹配，逐一匹配串中每个涉及到属性字段的字符串，然后存放于全局变量`char property[Max][Max]`中，该字符串二维数组用于存放表中的每个属性名称。  
具体匹配函数使用C++标准库函数`extern char *strstr(char *str1,char *str2)`，该函数的功能为从字符串`str1`中查找是否有符串`str2`，如果有，从`str1`中的`str2`位置起，返回`str1`的指针，如果没有，返回`null`。  
***注意：建表语句格式应形如`Create Student(sno,sname,ssex,sage,sdept)`***
```cpp
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
```


#### 插入记录(Insert)
插入记录实际上就是将新记录信息插入到文件末尾。实现思路如下：首先使用库函数`strstr`匹配用户输入串中的每个属性上的值，依次形成记录各个属性值的字符串，然后将各个字符串顺序插入到文件中，这样就可以实现将一整条记录插入到文件的一行中去了。这里要注意一点：我将一条记录的每个属性值间隔一个空格符，以便清晰和后续操作的实现。同时最后一个属性值没有空格符，而直接是串结尾符。  
***注意：插入语句格式应形如`Insert into Student(sno,sname, ssex,sage,sdept) values('95001','李勇','男','20','CS')`***
```cpp
void Handle_Insert(const char *Insertstr) { //插入操作
	char *index=strstr(Insertstr,"values"); //匹配字符串"values"
	index=index+6;
	char temp[Max];
	int pivot=0;
	fstream out; //建立流对象
	out.open("sqldata.txt",ios::out|ios::app); //关联文件sqldata,并设置打开方式为out|app，在原有文件的末尾插入记录
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
```


#### 查找记录(Search)
查找记录的思想也很简单，就是顺序读文件中的每条记录，若该记录符合`where`查询语句，则直接输出，否则跳过，读下一条记录。具体实现如下：首先，用`strstr`函数匹配三个字符串，第一：`Select`后紧跟着的最后选择的属性序列字符串；第二：`where`后紧跟着的判别依据属性项；第三：判别依据属性项后紧跟着的相应值。  
***注意：查找语句格式应形如`Select sno,sname,sage from Student where sdept='CS'`***  
以上面的查找语句为例，其中我们要匹配的字符串就是`sno,sname,sage` 和 `sdept` 和 `CS`, 将它们分别标记。则接下来要做的就是顺序读字符串了，在文件中每读一条记录，就判读相应的属性项`sdept`的值是否和要求的值`CS`相同，若相同，则直接输出该条记录，否则读文件中的下条记录。重复上述过程，直至顺序扫描完整个文件。当然，在所有记录输出之前要先输出最后选择的属性序列字符串`sno,sname,sage`。
```cpp
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
	ifstream in("sqldata.txt");
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
```


#### 修改记录(Update)
修改记录的思想和查询记录的思想非常类似，只不过多了一个辅助文件和要匹配的字符串。思路也是先匹配出要修改的属性项和要修改的值，以及`where`子句后跟着的判断条件所对应的两个字符串。
***注意：修改语句格式应形如`Update Student set sdept='IS' where sage='20'`***  
以上面的修改语句为例，其中要匹配的字符串就是`sdept` 、`IS` 以及 `sage` 和 `20` ，匹配完成后，接下来要做的事情就是顺序读取文件中的每个字符串，判断每条记录是否满足修改条件（即`sage=’20’`）,若满足，则修该相应属性项的值，否则不修改，这里将修改或不修改的记录均存于新创建的辅助文件中，这样，一趟顺序扫描完后，辅助文件中存放的就是修改后的全部记录了，最后将辅助文件拷贝到源文件中（将源文件的内容覆盖）,算法结束。
```cpp
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
	ifstream in("sqldata.txt");//输入流对象，关联文件sqldata
	fstream out;
	out.open("sqlhelpdata.txt",ios::out);//关联文件sqlhelpdata
	ofstream oout("sqlhelpdata.txt");//输出流对象，关联文件sqlhelpdata
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
	ifstream fin("sqlhelpdata.txt");
	fstream ftest;
	ftest.open("sqldata.txt",ios::out);
	while(getline(fin,line)) //拷贝文件，将文件sqlhelpdata中的内容拷贝到sqldata中
		ftest << line << endl;
	printf("修改成功\n");
}
```


#### 删除记录(Delete)
删除记录的思想和修改记录的思想基本一致，而且要相对简单一些，还是简单的描述一下算法实现：首先也是匹配`where`后要删除判别的属性项及其值。接着就是顺序扫描文件，读取文件中的每条记录，判段相应的属性项的值是否满足删除条件，若满足则直接判断下一条记录，否则将该记录写入到辅助文件中，这样，辅助文件存放的就是最终删除后的各记录值，最后将辅助文件中的内容拷贝到`sqldata`中，算法结束。  
***注意：删除语句格式应形如`Delete from Student where sname='李勇'`***
```cpp
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
	ifstream in("sqldata.txt");
	fstream out;
	out.open("sqlhelpdata.txt",ios::out);
	ofstream oout("sqlhelpdata.txt");
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
	ifstream fin("sqlhelpdata.txt");
	fstream ftest;
	ftest.open("sqldata.txt", ios::out);
	while(getline(fin,line)) //拷贝文件
		ftest << line << endl;
	printf("删除成功\n");
}
```


***

### 索引结构
索引结构采用了哈希索引用来快速查询，降低查询时间。建立哈希索引的思路很清晰：首先匹配要建立索引的属性项，然后顺序扫描文件中的每一条记录，将每条记录以该属性项的值作为关键值的来源插入到哈希表中，这样就在相应属性项上建立哈希索引了。哈希索引建立好后，若要依据该属性项查找某个记录，则直接在哈希表中进行查找，由于在求关键值的过程中已经删除了很大一部分不可能匹配的记录，故查找时间明显降低。这里哈希索引直接采用链地址法解决冲突，在计算到哈希表中的位置后，则直接在链表上进行进一步的匹配。
```cpp
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
	ifstream in("sqldata.txt");
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
```

***

### 外排序
外排序的核心就是分堆，将源文件分成多堆，然后依次将合适大小的堆放入内存，用快排等高速排序法排序，然后再写入文件中相应位置，接着对已经有序的序列进行虚拟二路归并操作，将两个有序的序列合成一个有序的序列，不断循环，直至最终合成一个有序的文件。按照上述的思想，这里排序我采用了C++标准库函数`sort`直接对元组按照指定属性值的大小进行排序，将原文件分成两堆，其中一堆处理1000条记录，后面一堆处理1000条以后的记录，分别在内存中排好序后，使用`Merge()`函数进行合并操作，最终合并成一个有序文件。
```cpp
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
	out.open("sortdata.txt",ios::out|ios::app);
	ofstream oout("sortdata.txt");
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
	ifstream in("sqldata.txt");
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
```


测试
-----------
### 建表(Create)
![建表](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/1.jpg)

### 插入记录(Insert)
![插入记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/2.jpg)

### 查找记录(Search)
![查找记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/3.jpg)

### 修改记录(Update)
![修改记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/4.jpg)

### 删除记录(Delete)
![删除记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/5.jpg)

### 索引结构
![索引结构](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/6.jpg)
![索引结构](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/7.jpg)

### 外排序
![外排序](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/8.jpg)
![外排序](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/9.jpg)


总结
-----------
在设计这个简单数据库的过程中，我走过了以下几个步骤：  
1.学习：了解数据库中各个概念，弄不清概念接下来就是一头雾水。  
2.实践：从使用数据库开始，在实际项目中使用，才能对各个概念有实际的印象。  
3.借鉴：尝试阅读读现有数据库的源代码(例如 SQLite 的代码）。  
4.准备：根据已有知识，来决定自己要一个用于什么目的的数据库，需要什么特性，如何实现。使用什么语言，自己是不是熟悉这门语言的特性，这些都很重要。  
5.实现：努力写代码去实现它  
当然这个小型数据库还有很多不足的地方，但是通过它我对数据库原理、程序设计和操作系统数据结构的相关知识都有了进一步的理解，以后有机会会将它进一步完善的。
