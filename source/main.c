#include <nds.h>
#include <nds/disc_io.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <stdio.h>
#include <unistd.h>

#include "libfat_ext/fat_ext.h"
#include "nds_loader_arm9.h"

int die() {
	while(1) swiWaitForVBlank();
	return 0;
}

int main(int argc, char** argv){
	consoleDemoInit();
	iprintf("ttloader technology preview\n");
	if(!fatInitDefault()) {
		iprintf("FAT init fail.\n");
		return die();
	}

	char file[24]="";
	char name[0x1001];

	unsigned long ioType = dldiGetInternal()->ioType;
	iprintf("%08lX\n", ioType);
	iprintf("Setting loader...\n");
	switch(ioType){
		case 0x4f495454: // TTIO
			sprintf(file, "/TTMenu/ttpatch.dat");
			break;
		case 0x46543452: // R4TF
			sprintf(file, "/TTMenu/r4patch.dat");
			break;
		case 0x5344334d: // M3DS
		case 0x53445469: // iTDS
		case 0x495f3452: // R4_I
			sprintf(file, "/TTMenu/m3patch.dat");
			break;
		default:
			iprintf("This cart not compatible.\n");
			return die();
	}
	if(access(file, F_OK) != 0) {
		iprintf("tt/r4/m3patch not found.\n");
		return die();
	}
	iprintf("Done.\n");
	
	if(access("/ttmenu.sys", F_OK) != 0) {
		iprintf("TTMENU.SYS not found. Creating...\n");
		FILE *f = fopen("/ttmenu.sys", "wb");
		fseek(f, 0, SEEK_SET);
        // memdump. Actually just expanding the file seems to crash, but this works totally fine...
        fwrite((void*)0x02400000, 0x400000, 1, f);
		fflush(f);
		fclose(f);
	}

	snprintf(name, 2, "/");
	FILE *f = fopen("/ttmenu.sys", "rb+");
	fseek(f, 0, SEEK_SET);
	fwrite("ttds",1,4,f);

	iprintf("%s\n", argv[1]);
	fseek(f,0x100,SEEK_SET);
	memset(name+1,0,0x1000);
	fatGetAliasPath("fat:/", argv[1],name+1);
	iprintf("%s\n", name);
	fwrite(name,1,0x1000,f);

	iprintf("%s\n", argv[2]);
	memset(name+1,0,0x1000);
	fatGetAliasPath("fat:/", argv[2],name+1);
	iprintf("%s\n", name);
	fwrite(name,1,0x1000,f);

	memset(name+1,0,0x1000);
	//strcpy(name,***ARP***);
	fwrite(name,1,0x1000,f);
	fflush(f);
	fclose(f);

	iprintf("Press START to launch.\n");
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		int pressed = keysDown();
		if(pressed & KEY_START) break;
	}

	int err = runNdsFile(file, 0, NULL);
	iprintf("Start failed, error %d\n", err);
	return die();
}
