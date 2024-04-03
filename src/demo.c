
// gcc -o multi_download multi_download.c -lcurl

#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

struct fileInfo {
	const char *url;
	char *fileptr;
	int offset; //  start 
	int end;   // end
	pthread_t thid;
	double download; // 
};

#define THREAD_NUM		10


struct fileInfo **pInfoTable;
double downloadFileLength = 0;


// fwrite(fp, size, count, buf);
// libcurl

// 1.  fwrite/write
// mmap 

size_t writeFunc(void *ptr, size_t size, size_t memb, void *userdata) {
	// --> ptr
	struct fileInfo *info = (struct fileInfo *)userdata;
	//printf("writeFunc: %ld\n", size * memb);

	memcpy(info->fileptr + info->offset, ptr, size * memb);
	info->offset += size * memb;
	

	return size * memb;
}

// 
int progressFunc(void *userdata, double totalDownload, double nowDownload, 
				double totalUpload, double nowUpload) {

	int percent = 0;
	static int print = 1;
	struct fileInfo *info = (struct fileInfo*)userdata;
	info->download = nowDownload;
	// save 
	
	if (totalDownload > 0) {

		int i = 0;
		double allDownload = 0;
		for (i = 0;i <= THREAD_NUM;i ++) {
			allDownload += pInfoTable[i]->download;
		}
		
		percent = (int)(allDownload / downloadFileLength * 100);
	}

	if (percent == print) {
		printf("thid:%ld percent: %d%%\n",info->thid, percent);
		print += 1;
	}

	return 0;
}

//
double getDownloadFileLength(const char *url) {

	CURL *curl = curl_easy_init();

	printf("url: %s\n", url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36");
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	
// 111
	CURLcode res = curl_easy_perform(curl);
	if (res == CURLE_OK) {
		printf("downloadFileLength success\n");
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLength);
	} else {
		printf("downloadFileLength error\n");
		downloadFileLength = -1;
	}
// 2222
	curl_easy_cleanup(curl);

	return downloadFileLength;
}


// curl
// 0 - 11
void *worker(void *arg) {

	struct fileInfo *info = (struct fileInfo*)arg;

	char range[64] = {0};
	snprintf(range, 64, "%d-%d", info->offset, info->end);

	printf("threadid: %ld, download from: %d to: %d\n", info->thid, info->offset, info->end);
	// curl --> 
	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, info->url); // url
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc); // save
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, info); 
	
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // progress
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunc);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, info);
	
	curl_easy_setopt(curl, CURLOPT_RANGE, range);
	
	// http range 
// 111
// multicurl 
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		printf("res %d\n", res);
	}
// 2222
	curl_easy_cleanup(curl);

	return NULL;
}


// https://releases.ubuntu.com/22.04/ubuntu-22.04.2-live-server-amd64.iso.zsync
// ubuntu.zsync
int download(const char *url, const char *filename) {

	long fileLength = getDownloadFileLength(url);
	printf("downloadFileLength: %ld\n", fileLength);
	

	// write
	int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // 
	if (fd == -1) {
		return -1;
	}

	if (-1 == lseek(fd, fileLength-1, SEEK_SET)) {
		perror("lseek");
		close(fd);
		return -1;
	}
	if (1 != write(fd, "", 1)) {
		perror("write");
		close(fd);
		return -1;
	}

	char *fileptr = (char *)mmap(NULL, fileLength, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
	if (fileptr == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return -1;
	}
// fileLength 2014, 11

// 0 99
// 100 199
// 200 299
// 300 399
// .....
// 1000 1023
	// thread arg
	int i = 0;
	long partSize = fileLength / THREAD_NUM;
	struct fileInfo *info[THREAD_NUM+1] = {NULL};
	
	for (i = 0;i <= THREAD_NUM;i ++) {

		info[i] = (struct fileInfo*)malloc(sizeof(struct fileInfo));
		
		info[i]->offset = i * partSize;
		if (i < THREAD_NUM) {
			info[i]->end = (i+1) * partSize - 1;
		} else {
			info[i]->end = fileLength - 1;
		}
		info[i]->fileptr = fileptr;
		info[i]->url = url;
		info[i]->download = 0;
	}
	pInfoTable = info;

	//pthread_t thid[THREAD_NUM+1] = {0};
	for (i = 0;i <= THREAD_NUM;i ++) {
		pthread_create(&(info[i]->thid), NULL, worker, info[i]);
	}

	for (i = 0;i <= THREAD_NUM;i ++) {
		pthread_join(info[i]->thid, NULL);
	}
	
	for (i = 0;i <= THREAD_NUM;i ++) {		
		free(info[i]);
	}
	munmap(fileptr, fileLength);
	close(fd);
	
	
	return 0;
}

// multi_download  https://releases.ubuntu.com/22.04/ubuntu-22.04.2-live-server-amd64.iso.zsync  ubuntu.zsync
int main(int argc, const char *argv[]) {
	

	return download("https://releases.ubuntu.com/20.04/ubuntu-20.04.6-live-server-amd64.iso.zsync", "ubuntu.zsync");

}




