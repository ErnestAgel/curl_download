#include<iostream>
#include<memory>
#include <functional>
#include<thread>
#include"Ccurl.h"

using namespace std;


int main(int argc, char **argv)
{
  if(argc > 1)
  {

  }

  unique_ptr<Ccurl> ptr = make_unique<Ccurl>();

  ptr->Init("https://releases.ubuntu.com/20.04/ubuntu-20.04.6-live-server-amd64.iso.zsync", "./test");
  
  ptr->Download_Task();
  
  return 0;
}