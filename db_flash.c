#include "include.h"
#include "db_flash.h"
S_Data_Map data_map;
S_Ram_Page ram_page;

void set_mem(unsigned char* data,unsigned char value,unsigned short length)//设置一段内存值为value
{
	unsigned short tmp_length = length;
	unsigned char* tmp_data = data;
	while(tmp_length --)
		{
			*(tmp_data ++) = value;
		}
}

void db_mem_copy(char* from_addr,char* to_addr,unsigned short length)
{
	char* tmp_from_addr;
	char* tmp_to_addr;
	tmp_from_addr = from_addr;
	tmp_to_addr = to_addr;
	while(length --)
		{
			*(tmp_to_addr ++) = *(tmp_from_addr ++);
		}
}


unsigned char is_data_changed(unsigned char page,unsigned index)
{
	if(data_map.data_info[page][index].is_this_data_change)
		{
			return 1;
		}
	return 0;
}
void set_data_change(unsigned char page,unsigned index,unsigned char enable)
{
	data_map.data_info[page][index].is_this_data_change= enable;
}

unsigned char is_data_effect(unsigned char page,unsigned index)//判断页中指定数据块是否有效
{
	if(data_map.data_info[page][index].is_this_data_effect)
		{
			return 1;
		}
	return 0;
}

void set_data_effect(unsigned char page,unsigned index,unsigned char enable)
{
	data_map.data_info[page][index].is_this_data_effect= enable;
}

unsigned char is_data_delet(unsigned char page,unsigned index)//判断页中指定数据块是否被删除了
{
	if(data_map.data_info[page][index].is_this_data_deleted)
		{
			return 1;
		}
	return 0;
}
void set_data_delete(unsigned char page,unsigned index,unsigned char enable)
{
	data_map.data_info[page][index].is_this_data_deleted= enable;
}


void updata_data_map(void)//将data_map写入flash
{
	write_flash(DB_CONFIG_PAGE_ADDRESS,(char*)&data_map,sizeof(data_map));
}

void init_data_map(void)//初始化data_map ,data_map用于查找数据等
{
	read_flash(DB_CONFIG_PAGE_ADDRESS,(char*)&data_map,sizeof(data_map));
	if(data_map.this_data_effect != EFFECT)
		{
			set_mem((unsigned char*)&data_map,0,sizeof(data_map));
			data_map.this_data_effect = EFFECT;
			updata_data_map();
		}
}

void reinit_data_map(void)
{
	set_mem((unsigned char*)&data_map,0,sizeof(data_map));
	data_map.this_data_effect = EFFECT;
	updata_data_map();
}



unsigned char set_cell_size(unsigned char page,unsigned short cell_size,unsigned char force)//设置页的每个数据块大小
{
	if(page > DB_DATA_PAGE_NUMBER)
		{
			return 2;
		}
	if((data_map.cell_size[page]) && (0 == force))//不是强制更改
		{
			return 3;
		}
	data_map.cell_size[page] = cell_size;
	return 1;
}

unsigned char is_page_full(unsigned char page)//检测指定的页是否存满
{
	if(0 == data_map.cell_size[page])
		{
			return 2;//未初始化或初始化错误
		}
	if((data_map.save_num[page] < (data_map.total_save_num[page]))||(data_map.delet_num[page]>0))
		{
			return 1;//页未满
		}
	return 0;//页满
}

unsigned char creat_database(E_Save_Data_Type data_type,unsigned short cell_size)//创建一个数据表
{
	unsigned char loopx;
	if((db_type_null == data_type)||(0 == cell_size))
		{
			return 3;//参数错误创建失败
		}
	for(loopx =0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if((data_map.data_type[loopx] == data_type)&&(1 == is_page_full(loopx)))
				{
					return 0;//此种类型的表已经创建了,并且不满
				}
		}
	for(loopx =0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if(data_map.data_type[loopx] == db_type_null)
				{
					data_map.data_type[loopx] = data_type;
					data_map.cell_size[loopx] =cell_size;
					data_map.total_save_num[loopx] = (DB_PAGE_SIZE/cell_size) > DATA_INFO_MAX_NUM ? DATA_INFO_MAX_NUM : (DB_PAGE_SIZE/cell_size);
					data_map.page_point[0] = loopx;
					return 1;//创建了一个新的表
				}
		}
	return 2;//表满可以增加数据表来结局这个问题
}

unsigned short cell_size_in_page(unsigned char page)//找出指定页的数据块大小
{
	return data_map.cell_size[page];
}

unsigned char dummy_address_write(unsigned int address,char* data,unsigned short len)//虚拟地址写最终写到ram中
{
	if((DB_DATA_PAGE_ADRESS(ram_page.copy_page_number+1) - address) > DB_PAGE_SIZE)
		{
			return 0;//要写的地址不在ram页范围内
		}
	if((DB_DATA_PAGE_ADRESS(ram_page.copy_page_number+1) - address) < len)
		{
			return 2;//要写的数据将会溢出
		}
	unsigned short tmp_len = len;
	unsigned int offset = address - DB_DATA_PAGE_ADRESS(ram_page.copy_page_number);
	char* tmp_data = data;
	unsigned char* write_address = (ram_page.data + offset);
	while(tmp_len)
		{
			*(write_address ++) = *(tmp_data ++);
			tmp_len --;
		}
	ram_page.is_this_data_change = 1;
	return 1;
}

unsigned char dummy_address_read(unsigned address,char* data,unsigned short len)
{
  return 1;
}


unsigned char save_ram_page_to_flash(void)//将ram缓存区的数据写入flash
{
	if(ram_page.is_this_data_change == 0)
		{return 2;}
	unsigned char reslut;
	reslut = write_flash(DB_DATA_PAGE_ADRESS(ram_page.copy_page_number),(char*)(ram_page.data),DB_PAGE_SIZE);
	if(reslut)
		{
			ram_page.is_this_data_change = 0;
		}
	return reslut;
}
unsigned char read_flash_page_to_ram(void)//将flash存储的数据载入到ram中
{
	unsigned char reslut;
	reslut = read_flash(DB_DATA_PAGE_ADRESS(ram_page.copy_page_number),(char*)(ram_page.data),DB_PAGE_SIZE);
	if(reslut)
		{
			ram_page.is_this_data_change = 0;
		}
	return reslut;
}


	
unsigned char load_flash_to_ram_page(unsigned char page_num)//将指定页的数据存储到ram中
{
	if((ram_page.is_this_data_effect)&&(ram_page.is_this_data_change)&&(ram_page.copy_page_number != page_num))
		{
			save_ram_page_to_flash();
			read_flash_page_to_ram();
		}
	if((ram_page.is_this_data_effect)&&(ram_page.copy_page_number == page_num))
		{
			return 2;
		}
	read_flash_page_to_ram();
	ram_page.copy_page_number = page_num;
	ram_page.is_this_data_effect = 1;
	return 1;
}

unsigned char find_data_type(E_Save_Data_Type data_type)//找出指定数据类型的保存页
{
	unsigned char loopx;
	unsigned char count = 0;
	for(loopx = 0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if(data_map.data_type[loopx] == data_type)
				{
					data_map.page_point[count] = loopx;
					count ++;
				}
		}
	return count;
}

unsigned short get_1st_delet_point(unsigned char page)
{
	unsigned short loopx;
	for(loopx = 0 ; loopx < data_map.save_num[page] ; loopx ++)
		{
			if(1 == is_data_delet(page,loopx))
				{					
					return loopx;
				}
		}
	return 0XFFFF;//未找到删除的点
}

unsigned short data_save_num_in_page(unsigned char page)//指定页中数据块的保存数量实际数量可能会小于返回值,这个值就是表创建以来保存过得最大值
{
	return data_map.save_num[page];
}

unsigned short data_delet_num_in_page(unsigned char page)//指定页中数据块已经删除的个数
{
	return data_map.delet_num[page];
}

void get_value(void* address,unsigned char data_length,long* value)//获取指定地址上的值的有符号类型值
{
	
		switch(data_length)
		{
			case 1:
				*value = *(char*)address;
				break;

			case 2:
				*value = *(short*)address;
				break;

			case 4:
				*value = *(int*)address;
				break;

			case 8:
				*value = *(long*)address;
				break;
		}
}

void get_unsigned_value(void* address,unsigned char data_length,long* value)//获取指定地址上的值的无符号类型值
{
	
		switch(data_length)
		{
			case 1:
				*value = *(unsigned char*)address;
				break;

			case 2:
				*value = *(unsigned short*)address;
				break;

			case 4:
				*value = *(unsigned int*)address;
				break;

			case 8:
				*value = *(long*)address;
				break;
		}
}

E_Compare_Reslut math_compare (long model_value,void* compare_address,unsigned char data_length,unsigned char is_data_signed)//比较指定地址上的数与model的大小关系
{
	long compare_value = 0;
	if(1 == is_data_signed)
		{
			get_value(compare_address,data_length,&compare_value);
		}
	else
		{
			get_unsigned_value(compare_address,data_length,&compare_value);
		}
	if(model_value < compare_value)
		{
			return compare_greater;
		}
	if(model_value == compare_value)
		{
			return compare_equel;
		}
	if(model_value > compare_value)
		{
			return compare_less;
		}
	return compare_not_equel;
}


E_Compare_Reslut string_compare(unsigned char* model,unsigned char* compare_str,unsigned char compare_str_length)//按字节比较指定地址与model的关系
{
	unsigned char loopx = 0;
	loopx = compare_str_length;
	unsigned char* tmp_model = model;
	unsigned char* tmp_compare_str = compare_str;
	while(loopx)
		{
			if(*tmp_model == *tmp_compare_str)
				{
					tmp_model ++;
					tmp_compare_str ++;
				}
			else
				{
					return compare_not_equel;
				}
			loopx --;
		}
	return compare_equel;
}

unsigned char compare_data(void* compare,P_S_Seek_Require req,P_S_Seek_Result res)//比较
{
	long effect_value = 0;
	long compare_value = req->compare_value;
	get_unsigned_value((void*)((int)compare+(req->effect_flags_offset)),(req->effect_flags_length),&effect_value);
	if((req->effect_flags_value) != effect_value)
		{
			return 0;
		}

	if(1 == req->is_compare_member_is_string)
		{
			if((req->compare_type) == string_compare((unsigned char*)(req->string_model),(unsigned char*)((int)compare+(req->compare_member_offset)),(req->compare_member_length)))
				{
					if(res != NULL)
						{
							//res->equal_count ++;
							//res->result[res->point] = compare;
							//res->point ++;
						}
					return 1;
				}
			else
				{
					return 0;
				}
		}	

	if((req->compare_type) == math_compare(compare_value,(void*)((int)compare+(req->compare_member_offset)),(req->compare_member_length),(req->is_compare_member_signed)))
		{
			if(res != NULL)
				{
					//res->equal_count ++;
					//res->result[res->point] = compare;
					//res->point ++;
				}
			return 1;
		}
	else
		{
			return 0;
		}
	
}

unsigned char find_data_in_page(unsigned char page,P_S_Seek_Require req,P_S_Seek_Result res)
{
	unsigned char seek_count;
	unsigned short loopx;
	unsigned short tmp_save_num;
	void* compare_address;
	seek_count = req->expect_seek_num;
	while(seek_count)
		{
			tmp_save_num = data_save_num_in_page(page);
			for(loopx = 0 ; loopx < tmp_save_num ; loopx ++)
				{
					if(is_data_effect(page,loopx))
						{
							if((ram_page.copy_page_number == page)&&(ram_page.is_this_data_effect == 1))//如果此页加载到内存则首先到ram中查找
								{
									compare_address = (void*)(ram_page.data+ (req->start_compare_offset+ loopx)*(data_map.cell_size[page]));
								}
							else
								{
									compare_address = (void*)(DB_DATA_PAGE_ADRESS(page) + (req->start_compare_offset+ loopx)*(data_map.cell_size[page]));
								}
							if(1 ==compare_data(compare_address,req,res))//符合查找条件
								{
								if(res != NULL)
									{
										res->result[res->point] = compare_address;
										res->equal_id[res->point].page = page;
										res->equal_id[res->point].index = loopx;
										res->equal_count ++;
										res->point ++;
										if((res->equal_count) ==  (req->expect_seek_num))
											{
												return 2;//找到的符合条件的数据达到期望个数
											}
									}
									seek_count --;
									if(0 == seek_count)
										{
											return 2;//找到的符合条件的数据达到期望个数
										}
								}
             
						}
					else
						{
							continue;
						}
				}
			if(seek_count == req->expect_seek_num)
				{
					return 0;//未找到符合条件的数据
				}
			return 1;//找到符合条件的数据个数小于期望查找的个数
		}
    return 3;//期望查找的个数为0  参数无效
}


unsigned char seek_data(P_S_Seek_Require req,P_S_Seek_Result res)//seek
{
	unsigned char loopx;
	loopx = find_data_type(req->data_type);
	while(loopx)
		{
			if(2 == find_data_in_page(data_map.page_point[loopx],req,res))
				{
					break;
				}
			loopx --;
		}
	return res->equal_count;
}

void insert_data_to_ram_page_index(unsigned char page,unsigned char index,void* data)
{
	if(index > (DB_PAGE_SIZE / data_map.cell_size[page]))
		{
			return ;
		}
	
	load_flash_to_ram_page(page);
	
	unsigned char* insert_addr;
	insert_addr = ram_page.data + (index*data_map.cell_size[page]);
	db_mem_copy((char*)data, (char*)insert_addr, data_map.cell_size[page]);
}

void insert_data_to_ram_page(unsigned char page,void* data)
{	
	unsigned short insert_point = 0;
	if(data_map.delet_num[page]>0)//删除过数据需要在删除的位置添加数据
		{
			insert_point = get_1st_delet_point(page);
			if(insert_point > (DB_PAGE_SIZE / data_map.cell_size[page]))
				{					
				}
			else
				{
					insert_data_to_ram_page_index(page,insert_point,data);			
					set_data_delete(page,insert_point,0);
					set_data_effect(page,insert_point,1);
					data_map.delet_num[page] --;
					ram_page.is_this_data_change = 1;
					ram_page.is_this_data_effect = EFFECT;
					return;
				}
		}
	
	if(1 != is_page_full(page))//此页不能添加
		{
			return;
		}
	insert_point = data_map.save_num[page]++;
	insert_data_to_ram_page_index(page,insert_point,data);
	set_data_delete(page,insert_point,0);
	set_data_effect(page,insert_point,1);
	ram_page.is_this_data_change = 1;
	ram_page.is_this_data_effect = EFFECT;
	
	return;
}

unsigned char save_data(P_S_Seek_Require req,P_S_Seek_Result res,void* data)
{
	unsigned char page_num,loopx;
	//S_Seek_Result res = {0};
	req->expect_seek_num =1;
	page_num = find_data_type(req->data_type);
	loopx = page_num;
	while(loopx)
		{
			if(2 == find_data_in_page(data_map.page_point[loopx],req,res))//找到符合条件的数据 数据更改
				{
					//insert_data_to_ram_page(res->equal_id[0].page,data);
					insert_data_to_ram_page_index(res->equal_id[0].page,res->equal_id[0].index,data);//插入
					return 2;//数据更改
					//dummy_address_write((unsigned int)(res.result[0]),data,data_map.cell_size[loopx]);
				}
			loopx --;
		}
	for(loopx = 0 ; loopx < page_num ; loopx++)
		{
			if(1 == is_page_full(data_map.page_point[loopx]))
				{
					insert_data_to_ram_page(data_map.page_point[loopx],data);
					return 1;//在已存在的页里插入数据
				}
		}
	if(3 == creat_database(req->data_type,req->data_length))
		{
			return 4;//参数错误保存失败
		}
	page_num = find_data_type(req->data_type);
	for(loopx = 0 ; loopx < page_num ; loopx++)
		{
			if(1 == is_page_full(data_map.page_point[loopx]))
				{
					insert_data_to_ram_page(data_map.page_point[loopx],data);
					return 3;//新建了一个表存数据
				}
		}
	return 0;//插入失败
}

void delete_data_page_index(unsigned char page,unsigned short index)
{
	set_data_delete(page,index,1);
	set_data_effect(page,index,0);
	set_data_change(page,index,1);
	data_map.delet_num[page]++;
}

unsigned char delete_data(P_S_Seek_Require req,P_S_Seek_Result res)
{
	unsigned char page_num,loopx;
	unsigned char delete_loop;
	//S_Seek_Result res = {0};
	if(0 == req->expect_seek_num)//如果未赋值,则默认为1
		{
			req->expect_seek_num = 1;
		}
	page_num = find_data_type(req->data_type);
	loopx = page_num;
	while(loopx)//全部页都要找一遍
		{
			if(2 == find_data_in_page(data_map.page_point[loopx],req,res))//数据更改
				{
					for(delete_loop = 0 ; delete_loop < res->equal_count ; delete_loop ++)
						{
							delete_data_page_index(res->equal_id[delete_loop].page,res->equal_id[delete_loop].index);
						}
					return 2;//删除个数到达期望查找个数
				}
			loopx --;
		}
	if(res->equal_count > 0)
		{
			for(delete_loop = 0 ; delete_loop < res->equal_count ; delete_loop ++)
				{
					delete_data_page_index(res->equal_id[delete_loop].page,res->equal_id[delete_loop].index);
				}	
			return 1;//已经全部删除,但是删除的个数还没达到期望个数
		}
	return 0;//未找到符合的数据
}


unsigned char db_commit(void)
{
	updata_data_map();
	save_ram_page_to_flash();
	return 1;
}






/*

测试函数
*/



void db_test(void)
{	
	init_data_map();
	S_DB_Demo demo = {0};		
	S_Seek_Require req = {0};
	S_Seek_Result res = {0};
	
	creat_database(db_type_user_define_1,sizeof(demo));
	
	unsigned char loopx = 0;
	for(;loopx<10;loopx++)
		{
			set_mem((unsigned char*)&demo,0,sizeof(S_DB_Demo));
			set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));
			demo.data_type = db_type_user_define_1;
			demo.is_this_data_effect = EFFECT;
			demo.primary_key = 1;
			demo.user_data_3 = loopx;

			req.data_type = db_type_user_define_1;
			req.effect_flags_length = 1;
			req.effect_flags_value = EFFECT;
			req.effect_flags_offset = offsetof(S_DB_Demo,is_this_data_effect);
			
			
			req.compare_member_offset = offsetof(S_DB_Demo,primary_key);
			req.compare_value = 1;
			//req.compare_member_length = sizeof(demo.primary_key);
			//如果不指定比较的数的长度,那么从flash中取出的数据将会是初始化的0 必定不等于compare_value的1 则此种情况下必定会添加到db尾部慎用  !!
			//同理如果compare_value=0 则必定符合条件,此时相当于从flash头开始添加慎用!!!
	
			req.primary_key_value = 1;
			req.compare_type = compare_equel;
			save_data(&req,&res,&demo);//add10个数据1~10
		}
	db_commit();
	/////////////////////////////////////////////////////////////
	set_mem((unsigned char*)&res,0,sizeof(S_Seek_Result));
	set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));
	req.expect_seek_num = 20;
	req.data_type = db_type_user_define_1;
	
	req.effect_flags_length = 1;
	req.effect_flags_value = EFFECT;
	req.effect_flags_offset = offsetof(S_DB_Demo,is_this_data_effect);
	
	//req.compare_member_offset = offsetof(S_DB_Demo,is_this_data_effect);
	req.compare_member_offset = offsetof(S_DB_Demo,primary_key);
	req.compare_value = 1;
	req.compare_member_length = sizeof(demo.primary_key);
	
	req.primary_key_value = 1;
	req.compare_type = compare_equel;
	seek_data(&req, &res);//查找demo中primary_key为1的数据

	
/////////////////////////////////////////////////////////////////////////
	set_mem((unsigned char*)&res,0,sizeof(S_Seek_Result));
	set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));
	
	req.expect_seek_num = 20;
	req.data_type = db_type_user_define_1;
	req.effect_flags_length = 1;
	req.effect_flags_value = EFFECT;
	req.effect_flags_offset = offsetof(S_DB_Demo,is_this_data_effect);
	
	req.compare_member_offset = offsetof(S_DB_Demo,user_data_3);
	req.compare_value = 4;
	req.compare_member_length = sizeof(demo.user_data_3);
	req.primary_key_value = 1;
	req.compare_type = compare_greater;
	seek_data(&req, &res);//查找demo中user_data_3大于4的数据
//////////////////////////////////////////////////////////////////////////	
	set_mem((unsigned char*)&res,0,sizeof(S_Seek_Result));
	set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));

	req.expect_seek_num = 3;
	req.data_type = db_type_user_define_1;
	req.effect_flags_length = 1;
	req.effect_flags_value = EFFECT;
	req.effect_flags_offset = offsetof(S_DB_Demo,is_this_data_effect);
	
	req.compare_member_offset = offsetof(S_DB_Demo,primary_key);
	req.compare_value = 1;
	req.compare_member_length = sizeof(demo.primary_key);
	req.primary_key_value = 1;
	req.compare_type = compare_equel;
	delete_data(&req, &res);//删除前3个数据
//////////////////////////////////////////////////////////////////////////		
	set_mem((unsigned char*)&res,0,sizeof(S_Seek_Result));
	set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));


	for(loopx = 0;loopx<5;loopx++)
		{
			set_mem((unsigned char*)&demo,0,sizeof(S_DB_Demo));
			set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));
			demo.data_type = db_type_user_define_1;
			demo.is_this_data_effect = EFFECT;
			demo.primary_key = 1;
			demo.user_data_3 = loopx+12;

			req.data_type = db_type_user_define_1;
			req.effect_flags_length = 1;
			req.effect_flags_value = EFFECT;
			req.effect_flags_offset = offsetof(S_DB_Demo,is_this_data_effect);
			
			
			req.compare_member_offset = offsetof(S_DB_Demo,primary_key);
			req.compare_value = 1;
			//req.compare_member_length = sizeof(demo.primary_key);//如果不指定比较的数的长度,那么从flash中取出的数据将会是初始化的0 必定不等于1 则此种情况下必定会添加到db尾部慎用  !!
			
	
			req.primary_key_value = 1;
			req.compare_type = compare_equel;
			save_data(&req,&res,&demo);//add 5个数据12 13 14 15 16
		}    	
	db_commit();//save只是将数据写到ram中,需要同步,seek从flash中查找,如果没有提交的话则刚更改的不能被查询到

//////////////////////////////////////////////////////////////////////////	
	set_mem((unsigned char*)&res,0,sizeof(S_Seek_Result));
	set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));

	req.expect_seek_num = 20;
	req.data_type = db_type_user_define_1;
	req.effect_flags_length = 1;
	req.effect_flags_value = EFFECT;
	req.effect_flags_offset = offsetof(S_DB_Demo,is_this_data_effect);
	
	req.compare_member_offset = offsetof(S_DB_Demo,user_data_3);
	req.compare_value = 12;
	req.compare_member_length = sizeof(demo.user_data_3);
	req.primary_key_value = 1;
	req.compare_type = compare_greater;
	seek_data(&req, &res);//查找demo中user_data_3大于12的数据
	
//////////////////////////////////////////////////////////////////////////		
	db_commit();
	set_mem((unsigned char*)&res,0,sizeof(S_Seek_Result));
	set_mem((unsigned char*)&req,0,sizeof(S_Seek_Require));
	

}


void deinit_seek_require(P_S_Seek_Require req)
{
	req->compare_member_length = 1;
	req->effect_flags_length = 1;
	req->effect_flags_value = EFFECT;
	req->expect_seek_num = 1;
}

























