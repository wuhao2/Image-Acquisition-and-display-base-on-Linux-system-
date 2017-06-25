#include "image_manager.h"

#include <config.h>
#include <fb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>




// ����洢ͼƬ��Ϣ������

image_info_t images[MAX_IMAGE_CNT];
unsigned int image_count = 0;			// ����images�еļ���index


// images���鱾���ǿյģ�Ȼ������ʼ��ʱ��ȥһ������Լ���õ�Ŀ¼(imageĿ¼)��ȥ
// �ݹ�������е��ļ������ļ��У����ҽ����е�ͼƬ��ʽ�ռ���������¼��images������
// ����������image�����оͼ�¼�����е�ͼƬ��Ȼ����ʾͼƬ�߼�������ȥ���ͼƬ����
// �ó���Ӧ��ͼƬ����ʾ����
// path��Ҫȥ�������ļ��е�pathname


int scan_image(const char *path)
{
	// �ڱ������еݹ����path�ļ��У�����������ͼƬ��䵽iamges������ȥ
	DIR *dir;
	struct dirent *ptr;
	char base[1000];

	if ((dir = opendir(path)) == NULL)
	{
		perror("Open dir error...");
		exit(1);
	}

	// readdir����ÿ����һ�ξͻ᷵��opendir�򿪵�basepathĿ¼�µ�һ���ļ���ֱ��
	// basepathĿ¼�������ļ���������֮�󣬾ͻ᷵��NULL
	while ((ptr = readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name, ".")==0 || strcmp(ptr->d_name, "..")==0)    ///current dir OR parrent dir
			continue;

		debug("d_name = %s.\n", ptr->d_name);
		debug("d_type = %d, DT_REG = %d, DT_DIR = %d, DT_UNKNOWN = %d.\n", 
			ptr->d_type, DT_REG, DT_DIR, DT_UNKNOWN);
		switch (ptr->d_type)
		{
			case DT_REG:			// ��ͨ�ļ�
				printf("d_name:%s/%s\n", path, ptr->d_name);
				break;
			case DT_DIR:			// �ļ���
				memset(base,'\0',sizeof(base));
				strcpy(base,path);
				strcat(base,"/");
				strcat(base,ptr->d_name);
				scan_image(base);
				break;
			case DT_UNKNOWN:		// ��ʶ����ļ���ʽ
				printf("unknown file type.\n");
				break;
			default:
				break;
		}
	}
}



int scan_image2(const char *path)
{
	// �ڱ������еݹ����path�ļ��У�����������ͼƬ��䵽iamges������ȥ
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	struct stat sta;

	if ((dir = opendir(path)) == NULL)
	{
		perror("Open dir error...");
		exit(1);
	}

	// readdir����ÿ����һ�ξͻ᷵��opendir�򿪵�basepathĿ¼�µ�һ���ļ���ֱ��
	// basepathĿ¼�������ļ���������֮�󣬾ͻ᷵��NULL
	while ((ptr = readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name, ".")==0 || strcmp(ptr->d_name, "..")==0)    ///current dir OR parrent dir
			continue;

		// ��lstat����ȡ�ļ����Բ��ж��ļ�����
		memset(base,'\0',sizeof(base));
		strcpy(base,path);
		strcat(base,"/");
		strcat(base,ptr->d_name);
		lstat(base, &sta);

		if (S_ISREG(sta.st_mode))
		{
			//printf("regular file.\n");
			//printf("d_name:%s/%s\n", path, ptr->d_name);
			// �������ͨ�ļ�����Ҫ��������д���:
			// ����˼·���� ���ж��Ƿ�������֪��ĳ��ͼƬ��ʽ���������ŵ�images������
			// �����������������
			if (!is_bmp(base))
			{
				strcpy(images[image_count].pathname, base);
				images[image_count].type = IMAGE_TYPE_BMP;
			}
			if (!is_jpg(base))
			{
				strcpy(images[image_count].pathname, base);
				images[image_count].type = IMAGE_TYPE_JPG;
			}
			if (!is_png(base))
			{
				strcpy(images[image_count].pathname, base);
				images[image_count].type = IMAGE_TYPE_PNG;
			}		
			image_count++;
			
		}
		if (S_ISDIR(sta.st_mode))
		{
			//printf("directory.\n");
			//printf("d_name:%s/%s\n", path, ptr->d_name);
			scan_image2(base);
		}
	}
}



void print_images(void)
{
	int i;

	printf("iamge_index = %d.\n", image_count);
	for (i=0; i<image_count; i++)
	{
		printf("images[i].pathname = %s,		type = %d.\n", images[i].pathname, images[i].type);
	}
}


void show_images(void)
{
	int i;

	for (i=0; i<image_count; i++)
	{
		switch (images[i].type)
		{
			case IMAGE_TYPE_BMP:
				display_bmp(images[i].pathname);		break;
			case IMAGE_TYPE_JPG:
				display_jpg(images[i].pathname);		break;
			case IMAGE_TYPE_PNG:
				display_png(images[i].pathname);		break;
			default:
				break;
		}
		sleep(2);
	}
}


static void show_image(int index)
{
	debug("index = %d.\n", index);
	switch (images[index].type)
	{
		case IMAGE_TYPE_BMP:
			display_bmp(images[index].pathname);		break;
		case IMAGE_TYPE_JPG:
			display_jpg(images[index].pathname);		break;
		case IMAGE_TYPE_PNG:
			display_png(images[index].pathname);		break;
		default:
			break;
	}	
}


// ������ʵ��ͨ������������ͼƬ��ҳ��ʾ
int ts_updown(void)
{
	// ��һ��: �������Ĵ����������
	int fd = -1, ret = -1;
	struct input_event ev;
	int i = 0;					// ������¼��ǰ��ʾ���ǵڼ���ͼƬ
		
	fd = open(DEVICE_TOUCHSCREEN, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return -1;
	}
		
	while (1)
	{
		memset(&ev, 0, sizeof(struct input_event));
		ret = read(fd, &ev, sizeof(struct input_event));
		if (ret != sizeof(struct input_event))
		{
			perror("read");
			close(fd);
			return -1;
		}

		// �ڶ���: ���ݴ�����������ҳ
		if ((ev.type == EV_ABS) && (ev.code == ABS_X))
		{
			// ȷ�������x����
			if ((ev.value >= 0) && (ev.value < TOUCH_WIDTH))
			{
				// �Ϸ�ҳ
				if (i-- <= 1)
				{
					i = image_count;
					debug("i=%d.\n", i);
				}
				
			}
			else if ((ev.value > (WIDTH - TOUCH_WIDTH)) && (ev.value <= WIDTH))
			{
				// �·�ҳ			
				if (i++ >= image_count)
				{
					i = 1;
					debug("i=%d.\n", i);
				}
			}
			else
			{
				// ����ҳ
			}
			show_image(i - 1);
		}
		
			
/*
		printf("-------------------------\n");
		printf("type: %hd\n", ev.type);
		printf("code: %hd\n", ev.code);
		printf("value: %d\n", ev.value);
		printf("\n");
		*/
	}	
	close(fd);

	return 0;
	
}











