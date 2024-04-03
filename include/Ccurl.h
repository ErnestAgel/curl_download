#include<iostream>
#include<curl/curl.h>
#include<string>
#include<mutex>
#include<vector>

using namespace std;

#define MaxThread   10


typedef struct 
{
    const char* url;
    char *file_ptr;
    int32_t offset;
    int32_t end;
    pthread_t thid;
    double download_len;    
}st_EasyList;


class Ccurl
{

public:
    Ccurl(/* args */);
    ~Ccurl();
    bool Init(const string url, string filename);
    
    bool Download_Task();

    bool Uploading_Task(const char* server_url);

    static void* Downloading(void* arg);
    // void Downloading(void* arg);
    
    st_EasyList *m_Easy_List[MaxThread + 1];
    

private:

    bool get_Download_FileSize();

    bool File_Init(const char* filename);

    void Destory();

    void* m_easyHandle = nullptr;
    string m_filename;
    string m_url;
    std::mutex m_lock;
    char* m_pTrunck = nullptr;
    int m_fd;
    vector<std::thread> m_threads;
    double m_fileLen;
    // vector<st_EasyList*> m_Easy_List;
};

