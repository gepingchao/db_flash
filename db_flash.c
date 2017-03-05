#include "db_flash.h"

S_Data_Map data_map;
S_Ram_Page ram_page;

unsigned char set_cell_size(unsigned char page,unsigned short cell_size,unsigned char force)
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

unsigned char creat_database(E_Save_Data_Type data_type,unsigned short cell_size)
{
	unsigned char loopx;
	for(loopx =0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if(data_map.data_type[loopx] == db_type_null)
				{
					data_map.data_type[loopx] = data_type;
					data_map.cell_size[loopx] =cell_size;
					data_map.page_point[0] = loopx;
					return 1;
				}
		}
	return 2;//表满可以增加数据表来结局这个问题
}

void save_ram_page_to_flash(void)
{
	//save_flsh(ram_page.copy_page_number);
}
unsigned short cell_size_in_page(unsigned char page)
{
	return data_map.cell_size[page];
}
	
unsigned char load_flash_to_ram_page(unsigned char page_num)
{
	if(ram_page.is_this_data_effect&ram_page.is_this_data_change)
		{
			save_ram_page_to_flash();
		}
	//if(0 == read_flash(page_num))
		//{
			//return 2;
		//}
	ram_page.copy_page_number = page_num;
	ram_page.is_this_data_effect = 1;
	return 1;
}

unsigned char find_data_type(E_Save_Data_Type data_type)
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

unsigned char is_data_effect(unsigned char page,unsigned index)
{
	if(data_map.data_info[page][index].is_this_data_effect)
		{
			return 1;
		}
	return 0;
}
unsigned char is_data_delet(unsigned char page,unsigned index)
{
	if(data_map.data_info[page][index].is_this_data_deleted)
		{
			return 1;
		}
	return 0;
}

unsigned short data_save_num_in_page(unsigned char page)
{
	return data_map.save_num[page];
}

unsigned short data_delet_num_in_page(unsigned char page)
{
	return data_map.delet_num[page];
}

unsigned char compare_data(P_S_Seek_Require req,P_S_Seek_Result res)
{
	return 1;
}

unsigned char find_data_in_page(unsigned char page,P_S_Seek_Require req,P_S_Seek_Result res)
{
	unsigned char seek_count;
	unsigned short loopx;
	unsigned short tmp_save_num;
	unsigned int start_cmp_address;
	start_cmp_address = DB_DATA_PAGE_ADRESS(page) + (req->start_compare_offset)*cell_size_in_page(page);
	seek_count = req->expect_seek_num;
	while(seek_count)
		{
			tmp_save_num = data_save_num_in_page(page);
			for(loopx = 0 ; loopx < tmp_save_num ; loopx ++)
				{
					if(is_data_effect(page,loopx))
						{
							if(1 ==compare_data(req,res))
								{
									seek_count --;
									if(0 == seek_count){return;}
								}
             
						}
					else
						{
							continue;
						}
				}
		}
	return 1;
}


unsigned char seek_data(P_S_Seek_Require req,P_S_Seek_Result res)
{
	unsigned char loopx;
	loopx = find_data_type(req->data_type);
	while(loopx)
		{
			find_data_in_page(data_map.page_point[loopx],req,res);
			loopx --;
		}
	return 1;
}

