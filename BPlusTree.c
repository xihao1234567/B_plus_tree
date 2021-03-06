#include "BPlusTree.h"
#include "stack.h"
#include<math.h>
#include <time.h>
static KeyType Unavailable = INT_MIN;


/* 生成节点并初始化 */



static BPlusTree MallocNewNode()
{

    BPlusTree NewNode;

    int i;

    NewNode = malloc(sizeof(struct BPlusNode));

    
    if (NewNode == NULL)

        exit(EXIT_FAILURE);

   
    i = 0;


    while (i < M + 1){

        NewNode->Key[i] = Unavailable;

        NewNode->Children[i] = NULL;

        i++;

    }

    NewNode->Next = NULL;

    NewNode->KeyNum = 0;

    return NewNode;

}




/* 初始化 */


extern BPlusTree Initialize(){

    BPlusTree T;  

    if (M < (3)){

        printf("M最小等于3！");

        exit(EXIT_FAILURE);

    }


    /* 根结点 */

    T = MallocNewNode();

    return T;


}

 

static Position FindMostLeft(Position P){

    Position Tmp;

    Tmp = P;



    while (Tmp != NULL && Tmp->Children[0] != NULL) {

        Tmp = Tmp->Children[0];

    }


    return Tmp;

}






static Position FindMostRight(Position P)
{


    Position Tmp;

    Tmp = P;

    while (Tmp != NULL && Tmp->Children[Tmp->KeyNum - 1] != NULL) 
	{

        Tmp = Tmp->Children[Tmp->KeyNum - 1];

    }

    

    return Tmp;

}



 

/*************************************************

Function:       FindSibling()

input:          Parent指当前节点的父亲节点，i是指当前节点在双亲节点中的位置索引

Description:    寻找一个兄弟节点，其存储的关键字未满，否则返回NULL

Return:         Position

*************************************************/

static Position FindSibling(Position Parent,int i)
{

    Position Sibling;

    int Limit;

    Limit = M;

    Sibling = NULL;

    

    if (i == 0)
	{

        if (Parent->Children[1]->KeyNum < Limit)

            Sibling = Parent->Children[1];

    }



    else if (Parent->Children[i - 1]->KeyNum < Limit)

        Sibling = Parent->Children[i - 1];

    

    else if (i + 1 < Parent->KeyNum && Parent->Children[i + 1]->KeyNum < Limit)
	{

        Sibling = Parent->Children[i + 1];

    }


    return Sibling;



}



/* 查找兄弟节点，其关键字数大于M/2 ;没有返回NULL*/



static Position FindSiblingKeyNum_M_2(Position Parent,int i,int *j){

    int Limit;

    Position Sibling;

    Sibling = NULL;

    Limit = LIMIT_M_2;


    if (i == 0)
	{

        if (Parent->Children[1]->KeyNum > Limit){

            Sibling = Parent->Children[1];

            *j = 1;

        }

    }

    else
	{

        if (Parent->Children[i - 1]->KeyNum > Limit){

            Sibling = Parent->Children[i - 1];

            *j = i - 1;

        }

        else if (i + 1 < Parent->KeyNum && Parent->Children[i + 1]->KeyNum > Limit){

            Sibling = Parent->Children[i + 1];

            *j = i + 1;

        }
    

    }



    return Sibling;



}



 



/* 当要对X插入Key的时候，i是X在Parent的位置，j是Key要插入的位置



   当要对Parent插入X节点的时候，i是要插入的位置，Key和j的值没有用



 */





/*************************************************

Function:       InsertElement()

input:          isKey指是否在叶子节点插入数据，Parent指当前节点的父亲节点，当前节点X，Key指要插入的值

Description:    当要对X插入Key的时候，i是X在Parent的位置，j是Key要插入的位置；当要对Parent插入X节点的时候，i是要插入的位置，Key和j的值没有用

Return:         Position

*************************************************/



static Position InsertElement(int isKey, Position Parent,Position X,KeyType Key,int i,int j){


//InsertElement(0, Parent, NewNode, Unavailable, i + 1, Unavailable);
    int k;

    

    if (isKey){

        

        /* 插入key */

        k = X->KeyNum - 1;

        

        while (k >= j)
		{

            X->Key[k + 1] = X->Key[k];k--;

        }


        X->Key[j] = Key;


        if (Parent != NULL)

            Parent->Key[i] = X->Key[0];

        X->KeyNum++;

    }else{

        /* 插入节点 */



        /* 对树叶节点进行连接 */



        if (X->Children[0] == NULL||X->Children[0] != NULL)
		{



            if (i > 0)



                Parent->Children[i - 1]->Next = X;



            X->Next = Parent->Children[i];



        }

        k = Parent->KeyNum - 1;

        

        while (k >= i)
		{

            Parent->Children[k + 1] = Parent->Children[k];

            Parent->Key[k + 1] = Parent->Key[k];

            k--;

        }


        Parent->Key[i] = X->Key[0];

        Parent->Children[i] = X;

        Parent->KeyNum++;


    }


    return X;

}



 




static Position RemoveElement(int isKey, Position Parent,Position X,int i,int j){


    int k,Limit; 

    if (isKey)
	{

        Limit = X->KeyNum;

        /* 删除key */

        k = j + 1;

        

        while (k < Limit)
		{

            X->Key[k - 1] = X->Key[k];k++;

        }

        X->Key[X->KeyNum - 1] = Unavailable;

        Parent->Key[i] = X->Key[0];

        X->KeyNum--;



    }else{

        /* 删除节点 */

  
        /* 修改树叶节点的链接 */

        if (X->Children[0] == NULL && i > 0){

            Parent->Children[i - 1]->Next = Parent->Children[i + 1];

        }

        Limit = Parent->KeyNum;

        k = i + 1;  

        while (k < Limit){

            Parent->Children[k - 1] = Parent->Children[k];

            Parent->Key[k - 1] = Parent->Key[k];

            k++;

        }    

        Parent->Children[Parent->KeyNum - 1] = NULL;

        Parent->Key[Parent->KeyNum - 1] = Unavailable;
       
        Parent->KeyNum--;



    }


    return X;



}



 
/* Src和Dst是两个相邻的节点，i是Src在Parent中的位置；



 将Src的元素移动到Dst中 ,n是移动元素的个数*/



static Position MoveElement(Position Src,Position Dst,Position Parent,int i,int n){


//MoveElement(T, Sibling, Parent, i, 1);
    KeyType TmpKey;

    Position Child;

    int j,SrcInFront;

    SrcInFront = 0;
    

    if (Src->Key[0] < Dst->Key[0])

        SrcInFront = 1;



    j = 0;

    /* 节点Src在Dst前面 */

    

    if (SrcInFront){



        if (Src->Children[0] != NULL){



            while (j < n) {

                Child = Src->Children[Src->KeyNum - 1];

                RemoveElement(0, Src, Child, Src->KeyNum - 1, Unavailable);

                InsertElement(0, Dst, Child, Unavailable, 0, Unavailable);

                j++;

            }



        }else{



            while (j < n) {

                TmpKey = Src->Key[Src->KeyNum -1];

                RemoveElement(1, Parent, Src, i, Src->KeyNum - 1);

                InsertElement(1, Parent, Dst, TmpKey, i + 1, 0);

                j++;

            }



        }

       

        Parent->Key[i + 1] = Dst->Key[0];

        /* 将树叶节点重新连接 */



        if (Src->KeyNum > 0)

            FindMostRight(Src)->Next = FindMostLeft(Dst);

    }else{

        if (Src->Children[0] != NULL){

            while (j < n) {

                Child = Src->Children[0];

                RemoveElement(0, Src, Child, 0, Unavailable);

                InsertElement(0, Dst, Child, Unavailable, Dst->KeyNum, Unavailable);

                j++;

            }



        }

		else

		{

            

            while (j < n) {

                TmpKey = Src->Key[0];

                RemoveElement(1, Parent, Src, i, 0);

                InsertElement(1, Parent, Dst, TmpKey, i - 1, Dst->KeyNum);

                j++;

            }

        }
     

        Parent->Key[i] = Src->Key[0];

        if (Src->KeyNum > 0)

            FindMostRight(Dst)->Next = FindMostLeft(Src);

    

    }

    

    return Parent;

}





/*************************************************

Function:       SplitNode()

input:          双亲节点Parent,X当前节点，i是X在双亲节点中的位置

Description:    分裂节点

Return:         BPlusTree

*************************************************/

static BPlusTree SplitNode(Position Parent,Position X,int i){

    int j,k,Limit;

    Position NewNode;

    NewNode = MallocNewNode();

    k = 0;

    j = X->KeyNum / 2;

    Limit = X->KeyNum;

    /* 分裂节点 */

    while (j < Limit){

        if (X->Children[0] != NULL){

            NewNode->Children[k] = X->Children[j];

            X->Children[j] = NULL;

        }



        NewNode->Key[k] = X->Key[j];

        X->Key[j] = Unavailable;

        NewNode->KeyNum++;X->KeyNum--;

        j++;k++;



    }

    

/*向父亲节点插入数据*/

    if (Parent != NULL)

        InsertElement(0, Parent, NewNode, Unavailable, i + 1, Unavailable);


    else
	{


        /* 如果是X是根，那么创建新的根并返回 */

        Parent = MallocNewNode();

        InsertElement(0, Parent, X, Unavailable, 0, Unavailable);

        InsertElement(0, Parent, NewNode, Unavailable, 1, Unavailable);

        return Parent;

    }

    return X;
}





/* 合并节点,X少于M/2关键字，S有大于或等于M/2个关键字*/


static Position MergeNode(Position Parent, Position X,Position S,int i){



    int Limit;


    /* S的关键字数目大于M/2 */

    if (S->KeyNum > LIMIT_M_2)
	{

        /* 从S中移动一个元素到X中 */

        MoveElement(S, X, Parent, i,1);


    }

	else

	{

        /* 将X全部元素移动到S中，并把X删除 */

        Limit = X->KeyNum;

        MoveElement(X,S, Parent, i,Limit);

        RemoveElement(0, Parent, X, i, Unavailable);

        free(X);

        X = NULL;

    }

    return Parent;

}



/*************************************************

Function:       RecursiveInsert()

input:          树的节点T，要插入的值key,i是T在双亲节点的位置，T的双亲节点Parent

Description:    插入元素

Return:         BPlusTree

*************************************************/


static BPlusTree RecursiveInsert(BPlusTree T,KeyType Key,int i,BPlusTree Parent){

    int j,Limit,start,end,temp;
    long  c1,c2;
    Position Sibling;

    /* 查找分支 */
	//printf("x=%d\n",T->KeyNum);

    j = 0;
    start=0;
	if(T->KeyNum==0)
		end=0;

	else

		end=(T->KeyNum)-1;

	if(Key<T->Key[0])
	{

		j=0;

	}
	else if(Key>T->Key[end])

	{

		j=end;

	}
	else
	{
		
		while(1)
		{

			temp=(start+end)/2;
			if(start>=end-1)
		
			{
				j=start;
				break;
			
			}

			if(Key==T->Key[temp])
				return T;
			if(Key > T->Key[temp])
				start=temp;
			else
			{
				end=temp;
			}

	
	}
	}
	if (T->Children[0] == NULL && Key>T->Key[0]&&T->KeyNum!=0) j++;

/*
    while (j < T->KeyNum && Key >= T->Key[j])
	{

        //重复值不插入 
		

        if (Key == T->Key[j])

            return T;

        j++;

    }


    if (j != 0 && T->Children[0] != NULL) j--;

*/

    /* 树叶 */



    if (T->Children[0] == NULL)
	{
		//printf("j=%d\n",j);
			
	    c1 = clock();
		

		
		T = InsertElement(1, Parent, T, Key, i, j);
		c2 = clock();
	//	printf("\nmove用时： %lf秒\n",difftime(c2,c1)/1000);


	}

    /* 内部节点 */

    else

        T->Children[j] = RecursiveInsert(T->Children[j], Key, j, T);


    /* 调整节点 */

    Limit = M;

 
    if (T->KeyNum > Limit){

        /* 根 */

        if (Parent == NULL)
		{

            /* 分裂节点 */

            T = SplitNode(Parent, T, i);

        }

        else
		{

            Sibling = FindSibling(Parent, i);

            if (Sibling != NULL)
			{

                /* 将T的一个元素（Key或者Child）移动的Sibing中 */

                MoveElement(T, Sibling, Parent, i, 1);

            }
			else
			{

                /* 分裂节点 */

                T = SplitNode(Parent, T, i);

            }

        }

    }


    if (Parent != NULL)

        Parent->Key[i] = T->Key[0];

    return T;



}



 



/* 插入 */



extern BPlusTree Insert(BPlusTree T,KeyType Key)
{

    return RecursiveInsert(T, Key, 0, NULL);


}


static BPlusTree RecursiveRemove(BPlusTree T,KeyType Key,int i,BPlusTree Parent)
{

    int j,NeedAdjust;
    Position Sibling,Tmp;
    Sibling = NULL;

    /* 查找分支 */

    j = 0;



    while (j < T->KeyNum && Key >= T->Key[j])
	{

        if (Key == T->Key[j])

            break;

        j++;

    }




    if (T->Children[0] == NULL)
	{

        /* 没找到 */


        if (Key != T->Key[j] || j == T->KeyNum)

            return T;

    }else

        if (j == T->KeyNum || Key < T->Key[j]) j--;



    



    



    



    /* 树叶 */



    if (T->Children[0] == NULL)
	{


        T = RemoveElement(1, Parent, T, i, j);



    }

	else
	{

        T->Children[j] = RecursiveRemove(T->Children[j], Key, j, T);



    }

    NeedAdjust = 0;

    /* 树的根或者是一片树叶，或者其儿子数在2到M之间 */


    if (Parent == NULL && T->Children[0] != NULL && T->KeyNum < 2)

        NeedAdjust = 1;


    /* 除根外，所有非树叶节点的儿子数在[M/2]到M之间。(符号[]表示向上取整) */


    else if (Parent != NULL && T->KeyNum < LIMIT_M_2)


        NeedAdjust = 1;


    /* 调整节点 */

    if (NeedAdjust){

        /* 根 */


        if (Parent == NULL){

            if(T->Children[0] != NULL && T->KeyNum < 2){

                Tmp = T;



                T = T->Children[0];

                free(Tmp);

                return T;


            }
 
        }else{

            /* 查找兄弟节点，其关键字数目大于M/2 */


            Sibling = FindSiblingKeyNum_M_2(Parent, i,&j);

            if (Sibling != NULL){

                MoveElement(Sibling, T, Parent, j, 1);

            }
		
	    else{

                if (i == 0)

                    Sibling = Parent->Children[1];

                else

                    Sibling = Parent->Children[i - 1]; 



                Parent = MergeNode(Parent, T, Sibling, i);

                T = Parent->Children[i];

            }

        }

    }



    return T;


}




/* 删除 */



extern BPlusTree Remove(BPlusTree T,KeyType Key){

    return RecursiveRemove(T, Key, 0, NULL);

}



 

/* 销毁 */

extern BPlusTree Destroy(BPlusTree T){

    int i,j;

    if (T != NULL){

        i = 0;

        while (i < T->KeyNum + 1){

            Destroy(T->Children[i]);i++;

        }


        printf("Destroy:(");

        j = 0;

        while (j < T->KeyNum)/*  T->Key[i] != Unavailable*/

            printf("%d:",T->Key[j++]);

        printf(") ");
        free(T);
        T = NULL;



    }

    return T;



}


static void RecursiveTravel(BPlusTree T,int Level){

    int i;

    if (T != NULL){

        printf("  ");

        printf("[Level:%d]-->",Level);

        printf("(");


        i = 0;

        while (i < T->KeyNum)/*  T->Key[i] != Unavailable*/

            printf("%d:",T->Key[i++]);


        printf(")");

        Level++;

        i = 0;

        while (i <= T->KeyNum) {


            RecursiveTravel(T->Children[i], Level);


            i++;



        }
        
    }

}



/* 遍历 */



extern void Travel(BPlusTree T)
{

    RecursiveTravel(T, 0);

    printf("\n");


}



 



/* 遍历树叶节点的数据 */



extern void TravelData(BPlusTree T){

    Position Tmp;
    int i;
    if (T == NULL)

        return ;

    printf("All Data:");


    Tmp = T;


    while (Tmp->Children[0] != NULL)

        Tmp = Tmp->Children[0];



    /* 第一片树叶 */


    while (Tmp != NULL){

        i = 0;

        while (i < Tmp->KeyNum)

            printf(" %d",Tmp->Key[i++]);

        Tmp = Tmp->Next;

    }

}


extern void serialize(BPlusTree T){
	FILE *fp;
	int i;
	BPlusTree firstchildren;
	if((fp=fopen("C://Users//Administrator.000//Desktop//test.txt","w+"))==NULL){
		printf("Cannot open file strike any key exit!");
	}


	while(T!=NULL)
	{
		firstchildren=T->Children[0];
		while(T!=NULL&&T->KeyNum!=0)
		{
		    i=0;
			fprintf(fp,"%ld%c",T->KeyNum,'\n');
		//	printf("keynum=%d\n",T->KeyNum);
			
			
	    	while(i<T->KeyNum)
			{

			    fprintf(fp,"%ld%c",T->Key[i],'\n');
				//printf("num=%d\n",T->Key[i]);
			    i++;
			}
			T=T->Next;
		}
		
		T=firstchildren;
	}
	fflush(fp);
	fclose(fp);


}











extern BPlusTree deserialize()
{
	FILE *fp;
	int ch,keynum,num,i,j,k,flag;
	BPlusTree Root,T,node,first,former;
	Stack *s;
	s=malloc(sizeof(*s));
	init(s);
    T = Initialize(); 
	Root=T;  //根节点
	flag=0;
	if((fp=fopen("C://Users//Administrator.000//Desktop//test.txt","r"))==NULL)
	{

		printf("Cannot open file strike any key exit!");
	
	}

    
	
	while(!feof(fp))
	{

		fscanf(fp,"%d",&keynum);
		printf("keynum=%d",keynum);
		node=MallocNewNode();
		j=0;
	//	c=fgetc(fp);

		while(j<keynum)
		{

	     	if(!feof(fp))
				fscanf(fp,"%d",&num);
			T->Key[j]=num;
			T->KeyNum++;
			j++;
		
		}
		break;
		//T=T->Children[0];
	}

	while(!feof(fp))
	{

		k=0;
	    while(T!=NULL&&!feof(fp))
		{

            fscanf(fp,"%d",&keynum);
	     	node=MallocNewNode();
			j=0;
			while(j<keynum&&!feof(fp))
			{

	     		fscanf(fp,"%d",&num);
				node->Key[j]=num;
				//printf("num1=%d\n",num);
				node->KeyNum++;
				j++;
			}

		    T->Children[k]=node;
			if(k!=0)
				T->Children[k-1]->Next=node;
			if(k==0&&flag==1)
			{

				former->Children[former->KeyNum-1]->Next=node;
			
			}

			if(k==0&&flag==0)
			{

				first=node;
			
			}
			k++;
			if(k==T->KeyNum)
			{
			
				former=T;
				T=T->Next;
				flag=1;
			    k=0;
			
			}
		
		}

		T=first;
		flag=0;			
		
	}

	return Root;

	

}


/*
  extern BPlusTree deserialize()
{
	FILE *fp;
	int ch,keynum,num,i,j,k,flag;
	BPlusTree Root,T,node,first,former;
	Stack *s;
	s=malloc(sizeof(*s));
	init(s);
    T = Initialize(); 
	Root=T;  //根节点
	flag=0;
	if((fp=fopen("C://Users//Administrator.000//Desktop//test.txt","r"))==NULL)
	{
		printf("Cannot open file strike any key exit!");
	}
    ch=fgetc(fp);
	
	while(ch!=EOF)
	{
		
		
		while(ch!=','&&ch!=EOF)
		{
			
			Push(s,ch);
			ch=fgetc(fp);
		}
		num=0;
		i=0;
		while(!Empty(s))
		{
			num=num+(Top(s)-48)*(int)pow(10,i);
			Pop(s);
			i++;
		}
		keynum=num;
		node=MallocNewNode();
		j=0;
	//	c=fgetc(fp);

		while(j<keynum)
		{
	     	
			ch=fgetc(fp);
			while(ch!=','&&ch!=EOF)
			{
				Push(s,ch);
				ch=fgetc(fp);	
			}
			num=0;
			i=0;	
			while(!Empty(s))
			{
				num=num+(Top(s)-48)*(int)pow(10,i);
				Pop(s);
				i++;
			}
			T->Key[j]=num;
			T->KeyNum++;
			j++;
		}
		break;
		//T=T->Children[0];
	}
	while(ch!=EOF)
	{
		k=0;
	    while(T!=NULL)
		{
			ch=fgetc(fp);
			while(ch!=','&&ch!=EOF)
			{
				Push(s,ch);
				ch=fgetc(fp);
			}
	    	num=0;
	    	i=0;
			while(!Empty(s))
			{
				num=num+(Top(s)-48)*(int)pow(10,i);
				Pop(s);
				//printf("num=%d\n",num);
				i++;
			}
			keynum=num;
	     	node=MallocNewNode();
			j=0;
			while(j<keynum)
			{
	     		num=0;
				i=0;
				ch=fgetc(fp);
				while(ch!=','&&ch!=EOF)
				{
					Push(s,ch);
					ch=fgetc(fp);
				}
		
			
				while(!Empty(s))
				{
				num=num+(Top(s)-48)*(int)pow(10,i);
				Pop(s);
				i++;
				}
				node->Key[j]=num;
				//printf("num1=%d\n",num);
				node->KeyNum++;
				j++;
			}
		    T->Children[k]=node;
			if(k!=0)
				T->Children[k-1]->Next=node;
			if(k==0&&flag==1)
			{
				former->Children[former->KeyNum-1]->Next=node;
			}
			if(k==0&&flag==0)
			{
				first=node;
			}
			k++;
			if(k==T->KeyNum)
			{
				former=T;
				T=T->Next;
				flag=1;
			    k=0;
			}
		}
		T=first;
		flag=0;			
		}

	return Root;

	

}
*/





