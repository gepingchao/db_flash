#ifndef __DB_FLASH__
#define __DB_FLASH__

/*******************************************
配置数据库
********************************************/
#define DB_START_PAGE	30
#define DB_CONFIG_SIZE	1
#define DB_PAGE_NUMBER	6
#define DB_DEFAULT_CELL_SIZE	20

//#define DB_CELL_SIZE	(sizeof())	
#ifndef DB_CELL_SIZE
#define DB_CELL_SIZE 	DB_DEFAULT_CELL_SIZE	
#endif


/**************************************

***************************************/
#define DB_TOTAL_SIZE		(DB_PAGE_NUMBER+DB_CONFIG_SIZE)
#define DB_CONFIG_PAGE		DB_START_PAGE
#define DB_DATA_START_PAGE	(DB_START_PAGE+1)
#define DB_DATA_END_PAGE	(DB_START_PAGE+1+DB_PAGE_NUMBER)
#define DB_PAGE_SIZE			FLASH_PAGE_SIZE
#define DB_CELL_NUM_PER_PAGE	(DB_PAGE_SIZE/DB_CELL_SIZE)

#define offsetof(struct_type, name) (size_t) ((char*)&((struct_type *)0)->name-(char *)((struct_type *)0))




/*************************************
FLAGS 标志位设置
*************************************/
#define FLAG_THIS_DATA_EFFECT		((uint16_t)0x0001)  
#define FLAG_IS_THIS_DATA_CHANGED	((uint16_t)0x0002)  
#define FLAG_3	((uint16_t)0x0004)  
#define FLAG_4	((uint16_t)0x0008)  
#define FLAG_5	((uint16_t)0x0010)  
#define FLAG_6	((uint16_t)0x0020)  
#define FLAG_7	((uint16_t)0x0040)  
#define FLAG_8	((uint16_t)0x0080)  
#define FLAG_9	((uint16_t)0x0100)  
#define FLAG_10	((uint16_t)0x0200) 
#define FLAG_11	((uint16_t)0x0400)  
#define FLAG_12	((uint16_t)0x0800)  
#define FLAG_13	((uint16_t)0x1000)  
#define FLAG_14	((uint16_t)0x2000)  
#define FLAG_15	((uint16_t)0x4000)  
#define FLAG_16	((uint16_t)0x8000)  
#define FLAG_All	((uint16_t)0xFFFF)  

#define SET_FLAG_DISABLE(flags,flag_type)	((flags) &(~(flag_type)))
#define SET_FLAG_ENABLE(flags,flag_type)	((flags)|(flag_type))
#define CHECK_FLAG(flags,flag_type)			((flags)&(flag_type))


/******************************************
结构体
******************************************/
typedef struct
{
	//unsigned char flags;
	unsigned char is_this_data_effect:1;
	unsigned char is_this_data_change:1;
	unsigned short index;
}S_Data_Info,*P_S_Data_Info;
#define DATA_INFO_MAX_NUM	(FLASH_PAGE_SIZE/sizeof(S_Data_Info))

typedef struct
{
	unsigned short point;
	unsigned short save_num;
	unsigned short delet_num;
	S_Data_Info data[DATA_INFO_MAX_NUM];
}S_Data_Map,*p_S_Data_Map;

typedef struct
{
	unsigned char is_this_data_effect:1;
	unsigned char is_this_data_change:1;
	unsigned char copy_page_number;
	unsigned char data[FLASH_PAGE_SIZE];
}S_Ram_Page,*P_S_Ram_Page;


typedef char(*compare_operat)(void* model,void* data,unsigned char compare_length);

typedef struct
{
	unsigned char equal_count;
	unsigned char point;
	unsigned char equal_id[20];
}S_Seek_Result,*P_S_Seek_Result;

typedef struct
{
	unsigned char page;
	unsigned char expect_seek_num;//期望查找的个数,当查找到的个数达到期望值时立即停止查找

	unsigned char flags_length;
	unsigned char flags_offset;
	unsigned char flags_effect_num;
	unsigned char compare_member_length;
	unsigned short compare_member_offset;
	
	unsigned short cell_size;
	unsigned short start_compare_offset;

	
	compare_operat cmp_operat;
	
}S_Seek_Require,*P_S_Seek_Require;
/********************************************
使用到的全局变量
********************************************/
extern S_Data_Map data_map;
extern S_Ram_Page ram_page;
#endif

