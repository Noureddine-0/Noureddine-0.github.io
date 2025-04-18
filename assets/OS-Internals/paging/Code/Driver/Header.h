#pragma once
//#include <ntddk.h>


#pragma warning(disable:4201)

typedef unsigned long long UINT64;

typedef union _CR0 {
	UINT64 flags;

	struct {

		UINT64 pe : 1;
		UINT64 mp : 1;
		UINT64 em : 1;
		UINT64 ts : 1;
		UINT64 et : 1;
		UINT64 ne : 1;
		UINT64 reserved1 : 10;
		UINT64 wp : 1;
		UINT64 reserved2 : 1;
		UINT64 am : 1;
		UINT64 reserved3 : 10;
		UINT64 nw : 1;
		UINT64 cd : 1;
		UINT64 pg : 1;
		UINT64 reserved4 : 32;
	};
}CR0;

typedef union {
	UINT64 flags;
	struct {
		UINT64 ignored1 : 3;
		UINT64 pwt : 1;
		UINT64 pcd : 1;
		UINT64 ignored2 : 7;
		UINT64 pml4_paddr : 40;
		UINT64 reserved1 : 12;
	};
}CR3;


typedef union {
	UINT64 flags;
	struct {
		UINT64 vme : 1;
		UINT64 pvi : 1;
		UINT64 tsd : 1;
		UINT64 de : 1;
		UINT64 pse : 1;
		UINT64 pae : 1;
		UINT64 mce : 1;
		UINT64 pge : 1;
		UINT64 pce : 1;
		UINT64 osfxsr : 1;
		UINT64 osxmmexcpt : 1;
		UINT64 umip : 1;
		UINT64 la57 : 1;
		UINT64 vmxe : 1;
		UINT64 smxe : 1;
		UINT64 reserved1 : 1;
		UINT64 fsgsbase : 1;
		UINT64 pcide : 1;
		UINT64 osxsave : 1;
		UINT64 reserved2 : 1;
		UINT64 smep : 1;
		UINT64 smap : 1;
		UINT64 pke : 1;
		UINT64 cet : 1;
		UINT64 pks : 1;
		UINT64 reserved3 : 39;
	};
}CR4;

typedef union {
	UINT64 Address;
	struct {
		UINT64 offset : 12;
		UINT64 pt_index : 9;
		UINT64 pd_index : 9;
		UINT64 pdpt_index : 9;
		UINT64 pml4_index : 9;
		UINT64 sign_extension : 16;
	};
}_VirtualAddress;

typedef union {
	UINT64 flags;
	struct {
		UINT64 p : 1;
		UINT64 w : 1;
		UINT64 us : 1;
		UINT64 pwt : 1;
		UINT64 pcd : 1;
		UINT64 a : 1;
		UINT64 ignored1 : 1;
		UINT64 rsvd : 1;
		UINT64 ignored2 : 4;
		UINT64 pdpt : 40;
		UINT64 ignored3 : 11;
		UINT64 nx : 1;
	};
}PML4E_64;

typedef union _PDPTE_64 {
	UINT64 flags;
	struct {
		UINT64 p : 1;
		UINT64 w : 1;
		UINT64 us : 1;
		UINT64 pwt : 1;
		UINT64 pcd : 1;
		UINT64 a : 1;
		UINT64 ignored1 : 1;
		UINT64 ps : 1;
		UINT64 ignored3 : 4;
		UINT64 pd : 40;
		UINT64 ignored5 : 11;
		UINT64 nx : 1;
	};
} PDPTE_64;


typedef union {
	UINT64 flags;
	struct {
		UINT64 p : 1;
		UINT64 w : 1;
		UINT64 us : 1;
		UINT64 pwt : 1;
		UINT64 pcd : 1;
		UINT64 a : 1;
		UINT64 ignored1 : 1;
		UINT64 ps : 1;
		UINT64 ignored3 : 4;
		UINT64 pt : 40;
		UINT64 ignored5 : 11;
		UINT64 nx : 1;
	};
}PDE_64;

typedef union {
	UINT64 flags;
	struct {
		UINT64 p : 1;
		UINT64 r : 1;
		UINT64 us : 1;
		UINT64 pwt : 1;
		UINT64 pcd : 1;
		UINT64 a : 1;
		UINT64 d : 1;
		UINT64 pat : 1;
		UINT64 g : 1;
		UINT64 ignored1 : 3;
		UINT64 pf : 40;
		UINT64 ignored2 : 7;
		UINT64 pk : 4;
		UINT64 nx : 1;
	};
}PTE_64;
typedef _VirtualAddress VIRTUAL_ADDRESS;


#define PML4_TABLE_PHYSICAL_ADDRESS_MASK                             0x000FFFFFFFFFF000
#define PDPT_TABLE_PHYSICAL_ADDRESS_MASK                             0x000FFFFFFFFFF000
#define PD_TABLE_PHYSICAL_ADDRESS_MASK                               0x000FFFFFFFFFF000
#define PT_TABLE_PHYSICAL_ADDRESS_MASK                               0x000FFFFFFFFFF000


#define PDPTE_1GB_64_PAGE_FRAME_NUMBER_FLAG                          0x000FFFFFC0000000
#define PDE_2MB_64_PAGE_FRAME_NUMBER_FLAG                            0x000FFFFFFFE00000


#pragma warning(default:4201)

#define TRANSLATOR 0x8000
#define IOCTL_TRANSLATOR_TRANSLATE_ADDRESS CTL_CODE(TRANSLATOR,\
	0x800,METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef struct _INPUT {
	UINT64 vAddress;
}INPUT, * PINPUT;

typedef struct _OUTPUT {
	UINT64 pAddress;
}OUTPUT, * POUTPUT;
