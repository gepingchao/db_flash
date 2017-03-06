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
			return 2;//未初始化
		}
	if(data_map.save_num[page] < (DB_PAGE_SIZE / data_map.cell_size[page]))
		{
			return 1;//页未满
		}
	return 0;//页满
}

unsigned char creat_database(E_Save_Data_Type data_type,unsigned short cell_size)//创建一个数据表
{
	unsigned char loopx;
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
	if(ram_page.copy_page_number == page_num)
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
					data_map.page_point[count] = data_type;
					count ++;
				}
		}
	return count;
}

unsigned char is_data_effect(unsigned char page,unsigned index)//判断页中指定数据块是否有效
{
	if(data_map.data_info[page][index].is_this_data_effect)
		{
			return 1;
		}
	return 0;
}
unsigned char is_data_delet(unsigned char page,unsigned index)//判断页中指定数据块是否被删除了
{
	if(data_map.data_info[page][index].is_this_data_deleted)
		{
			return 1;
		}
	return 0;
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
							res->equal_count ++;
							res->result[res->point] = compare;
							res->point ++;
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
					res->equal_count ++;
					res->result[res->point] = compare;
					res->point ++;
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
	void* compare_ddress;
	seek_count = req->expect_seek_num;
	while(seek_count)
		{
			tmp_save_num = data_save_num_in_page(page);
			for(loopx = 0 ; loopx < tmp_save_num ; loopx ++)
				{
					if(is_data_effect(page,loopx))
						{
							compare_ddress = (void*)(DB_DATA_PAGE_ADRESS(page) + (req->compare_member_offset + loopx)*(data_map.cell_size[page]));
							if(1 ==compare_data(compare_ddress,req,res))
								{
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
			return 0;//未找到符合条件的数据
		}
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
	return 1;
}

void insert_data_to_page(unsigned char page)
{	
	load_flash_to_ram_page(page);
}

unsigned char save_data(P_S_Seek_Require req,void* data)
{
	unsigned char page_num,loopx;
	S_Seek_Result res;
	req->expect_seek_num =1;
	page_num = find_data_type(req->data_type);
	loopx = page_num;
	while(loopx)
		{
			if(2 == find_data_in_page(data_map.page_point[loopx],req,&res))
				{
					load_flash_to_ram_page(loopx);//将查找的的数据所在页载入内存
					dummy_address_write((unsigned int)(res.result[0]),data,data_map.cell_size[loopx]);
				}
			loopx --;
		}
	for(loopx = 0 ; loopx < page_num ; loopx++)
		{
			
		}
	return 1;
}












/*

测试函数
*/



void db_test(void)
{
	S_DB_Demo demo;
	demo.data_type = db_type_user_define_1;
	demo.is_this_data_effect = EFFECT;
	demo.primary_key = 1;
	demo.user_data_3 = 0x88;
	init_data_map();
	creat_database(demo.data_type,sizeof(demo));

	S_Seek_Require req;
	req.data_type = demo.data_type;
	req.effect_flags_length = 1;
	req.compare_member_offset = offsetof(S_DB_Demo,is_this_data_effect);
	req.compare_value = EFFECT;
	req.compare_member_offset = offsetof(S_DB_Demo,primary_key);
	req.compare_type = compare_equel;
	save_data(&req,&demo);
}


























