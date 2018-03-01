---
layout:     post
title: git的简单操作
description: 记录我从零开始学git的一些命令
category: blog
---
---
layout:     post
title: 看《剑指offer》的记录
description: 记录书中学到的一些东西
category: blog
---
<p>
  对于第一章面试相关的技巧，结合自身之前面试的几次经验，记录几个比较重要的：  
  1.电话。投递简历后一般HR都是电话联系。可以记录一些有用的信息，然后让HR发一个邮件给你。例如联系方式、何时何地面试，有什么需要准备的等等.  
  2.准备问题。面试会有HR面，技术面，老大面等等。提前想好针对谁可以问什么问题，以及面试过程中记住对方说的可以针对性的问。切忌问错人。我组长之前面试一个人回来后和我说他还问加不加班，印象不好。虽然我对这一点不予置评，但可见还是需要准备好问题的.  
  3.自我介绍。面试者可能之前都不知道有要面试你这个任务，所以会让你简述一下自己。此时对于技术面可以加入项目、学习工作经历。切忌说完全不懂的，因为可能技术面会深入的问一两个问题  
  4.书中讲介绍项目可以用“项目背景-完成任务-为了完成任务做了什么怎么做的-贡献”。此外还需要准备这个项目中遇到了什么大问题、怎么解决的、学到了什么。  
  5.“精通”是个很傻缺的词汇。
  6.必须准备的答案：自我介绍、项目介绍、为什么跳槽。时刻注意解决问题要有清晰的思路。  
  
</p>

<p>
  第二章开始就是一些基本知识了。这里记录一下自己思考、参考后写出的面试题C语言的答案
</p>
<pre>
//面试题1、2不适合C语言答题。因此从3开始
//第三题的二位数组是行从小到大排序，列从小到大排序。
//从第一行的最后一列开始匹配，当前值大于查找值，可以排除当前列。因此列自减
//当前值小于查找值，可以排除当前行。因此行自增
//如果相等，则找到了。
//别忘了对边界条件进行判定
#include &lt;stdio.h&gt;
#include &lt;string.h&gt;

int arry[5][5] =
{
    {1,3,4,7,8},
    {2,5,6,9,11},
    {10,90,93,101,231},
    {25,57,59,99,103},
    {60,63,89,333,777}
};


int find(int str[5][5], int rows, int columns, int number)
{
    int r = 0;                      //行
    int c = 0;                      //列
    if(str == NULL && rows > 0 && columns > 0)
        return -1;

    r = 0;                          //行从0开始
    c = columns - 1;                //列从最后一列开始
    while(r<rows && c >= 0)
    {
        if(str[r][c] == number)
            return 1;
        else if(str[r][c] < number)
        {
            r++;
        }
        else if(str[r][c] > number)
        {
            c--;
        }
    }
    return 0;
}

int main()
{
    int ret = find(arry, 5, 5, 6);
    printf("%d\n", ret);
    return 0;
}
</pre>


<pre>
//第四题是查找替换空格。把整个字符串的空格替换成%20
//算法有两种。
//1.遇到一个空格就把后面的全部往后推3个。然后加入%20
//2.先扫描一遍有多少空格。然后结果字符串长度是本身长度+空格数*2。从后往前，遇到空格就在最后加'0','2','%'.普通字符直接复制到最后
//1简单。但建议用2，2需要两个指针，指向原字符串的最后，以及结果字符串的最后。然后往前遍历
#include &lt;stdio.h&gt;
#include &lt;string.h&gt;

char string[1000] = "dsaf fdsfj ewq ck;lskffds sf s  sdfs sdf   fsdf   ";

int replaceblank(char *str)
{
    unsigned int u32blankcnt = 0;
    int i = 0;
    for(i = 0; i < strlen(str) + 1; i++ )
    {
        if(str[i] == ' ')
            u32blankcnt++;
    }
    unsigned int u32strlen = strlen(str);
    unsigned int u32retlen = u32strlen + u32blankcnt*2;
    char *pcurstr = &(str[0]);
    char *pretstr = &(str[0]);
    int j = 0;
    for(i = u32strlen, j = u32retlen; i >=0; i--)
    {
        if(pcurstr[i] != ' ')
        {
            pretstr[j--] = pcurstr[i];

        }
        else
        {
            pretstr[j--] = '0';
            pretstr[j--] = '2';
            pretstr[j--] = '%';
        }
    }
}


int main()
{
    replaceblank(string);
    printf("%s\n", string);
    return 0;
}

</pre>


<p>
  暂时看到这里。后续继续更新  
</p>