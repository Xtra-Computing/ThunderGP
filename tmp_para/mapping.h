
#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY

const cumem_lut_t mapping_item[] =
{
	{
		.cu_id  = 0,
		.mem_id = BANK_NAME(0),
		.he_attr_id = ATTR_PL_DDR0,
		.interface_id = 0,
	},
	{
		.cu_id  = 1,
		.mem_id = BANK_NAME(1),
		.he_attr_id = ATTR_PL_DDR1,
		.interface_id = 1,
	},
	{
		.cu_id  = 2,
		.mem_id = BANK_NAME(2),
		.he_attr_id = ATTR_PL_DDR2,
		.interface_id = 2,
	},
	{
		.cu_id  = 3,
		.mem_id = BANK_NAME(3),
		.he_attr_id = ATTR_PL_DDR3,
		.interface_id = 3,
	},
};