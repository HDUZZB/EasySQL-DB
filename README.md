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
* [开发流程](#开发流程)
* [设计思路](#设计思路)
    * [SQL语句识别器](#SQL语句识别器)
        *  [建表(Create)](#建表(Create))
        *  [插入记录(Insert)](#插入记录(Insert))
        *  [查找记录(Search)](#查找记录(Search))
        *  [修改记录(Update)](#修改记录(Update))
        *  [删除记录(Delete)](#删除记录(Delete))
    * [索引结构](#索引结构)
    * [外排序](#外排序)
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
			out.open("c:\\sqldata.txt", ios::out);
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
![建表](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/1.jpg)

#### 插入记录(Insert)
插入记录实际上就是将新记录信息插入到文件末尾。实现思路如下：首先使用库函数`strstr`匹配用户输入串中的每个属性上的值，依次形成记录各个属性值的字符串，然后将各个字符串顺序插入到文件中，这样就可以实现将一整条记录插入到文件的一行中去了。这里要注意一点：我将一条记录的每个属性值间隔一个空格符，以便清晰和后续操作的实现。同时最后一个属性值没有空格符，而直接是串结尾符。  
***注意：插入语句格式应形如`Insert into Student(sno,sname, ssex,sage,sdept) values('95001','李勇','男','20','CS')`***
![插入记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/2.jpg)

#### 查找记录(Search)
查找记录的思想也很简单，就是顺序读文件中的每条记录，若该记录符合`where`查询语句，则直接输出，否则跳过，读下一条记录。具体实现如下：首先，用`strstr`函数匹配三个字符串，第一：`Select`后紧跟着的最后选择的属性序列字符串；第二：`where`后紧跟着的判别依据属性项；第三：判别依据属性项后紧跟着的相应值。  
***注意：查找语句格式应形如`Select sno,sname,sage from Student where sdept='CS'`***  
以上面的查找语句为例，其中我们要匹配的字符串就是`sno,sname,sage` 和 `sdept` 和 `CS`, 将它们分别标记。则接下来要做的就是顺序读字符串了，在文件中每读一条记录，就判读相应的属性项`sdept`的值是否和要求的值`CS`相同，若相同，则直接输出该条记录，否则读文件中的下条记录。重复上述过程，直至顺序扫描完整个文件。当然，在所有记录输出之前要先输出最后选择的属性序列字符串`sno,sname,sage`。
![查找记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/3.jpg)

#### 修改记录(Update)
修改记录的思想和查询记录的思想非常类似，只不过多了一个辅助文件和要匹配的字符串。思路也是先匹配出要修改的属性项和要修改的值，以及`where`子句后跟着的判断条件所对应的两个字符串。
***注意：修改语句格式应形如`Update Student set sdept='IS' where sage='20'`***  
以上面的修改语句为例，其中要匹配的字符串就是`sdept` 、`IS` 以及 `sage` 和 `20` ，匹配完成后，接下来要做的事情就是顺序读取文件中的每个字符串，判断每条记录是否满足修改条件（即`sage=’20’`）,若满足，则修该相应属性项的值，否则不修改，这里将修改或不修改的记录均存于新创建的辅助文件中，这样，一趟顺序扫描完后，辅助文件中存放的就是修改后的全部记录了，最后将辅助文件拷贝到源文件中（将源文件的内容覆盖）,算法结束。
![修改记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/4.jpg)

#### 删除记录(Delete)
删除记录的思想和修改记录的思想基本一致，而且要相对简单一些，还是简单的描述一下算法实现：首先也是匹配`where`后要删除判别的属性项及其值。接着就是顺序扫描文件，读取文件中的每条记录，判段相应的属性项的值是否满足删除条件，若满足则直接判断下一条记录，否则将该记录写入到辅助文件中，这样，辅助文件存放的就是最终删除后的各记录值，最后将辅助文件中的内容拷贝到`sqldata`中，算法结束。  
***注意：删除语句格式应形如`Delete from Student where sname='李勇'`***
![删除记录](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/5.jpg)

***

### 索引结构
索引结构采用了哈希索引用来快速查询，降低查询时间。建立哈希索引的思路很清晰：首先匹配要建立索引的属性项，然后顺序扫描文件中的每一条记录，将每条记录以该属性项的值作为关键值的来源插入到哈希表中，这样就在相应属性项上建立哈希索引了。哈希索引建立好后，若要依据该属性项查找某个记录，则直接在哈希表中进行查找，由于在求关键值的过程中已经删除了很大一部分不可能匹配的记录，故查找时间明显降低。这里哈希索引直接采用链地址法解决冲突，在计算到哈希表中的位置后，则直接在链表上进行进一步的匹配。
![索引结构](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/6.jpg)
![索引结构](https://github.com/HDUZZB/EasySQL-DB/blob/master/image/7.jpg)

***

### 外排序
外排序的核心就是分堆，将源文件分成多堆，然后依次将合适大小的堆放入内存，用快排等高速排序法排序，然后再写入文件中相应位置，接着对已经有序的序列进行虚拟二路归并操作，将两个有序的序列合成一个有序的序列，不断循环，直至最终合成一个有序的文件。按照上述的思想，这里排序我采用了C++标准库函数`sort`直接对元组按照指定属性值的大小进行排序，将原文件分成两堆，其中一堆处理1000条记录，后面一堆处理1000条以后的记录，分别在内存中排好序后，使用`Merge()`函数进行合并操作，最终合并成一个有序文件。
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
