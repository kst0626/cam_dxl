#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "dxl.hpp"
using namespace std;
bool ctrl_c_pressed = false;
void ctrlc_handler(int){ ctrl_c_pressed = true; }
int main(void)
{
  Dxl mx;
  struct timeval start,end1;
  double time1;
  int vel1 = 0,vel2 = 0;
  signal(SIGINT, ctrlc_handler);
  if(!mx.open()) { cout << "dynamixel open error"<<endl; return -1; }
  string src = “nvarguscamerasrc sensor-id=0 ! \
    video/x-raw(memory:NVMM), width=(int)640, height=(int)360, \
    format=(string)NV12 ! nvvidconv flip-method=0 ! video/x-raw, \
    width=(int)640, height=(int)360, format=(string)BGRx ! \
    videoconvert ! video/x-raw, format=(string)BGR ! appsink”;
  
  VideoCapture source(src, CAP_GSTREAMER);
  if (!source.isOpened()){ cout << “Camera error" << endl; return -1; }

  string dst1 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
    nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
    h264parse ! rtph264pay pt=96 ! \
    udpsink host=203.234.58.X port=8001 sync=false";
  VideoWriter writer1(dst1, 0, (double)30, Size(640, 360), true);
  if (!writer1.isOpened()) { cerr << "Writer open failed!" << endl; return -1;}
  while(true)
  {
    gettimeofday(&start,NULL);
    if (mx.kbhit()) //키보드입력 체크
    {
    char c = mx.getch(); //키입력 받기
    switch(c)
    {
      case 's': vel1 = 0; vel2 = 0; break; //정지
      case ’f’: vel1 = 50; vel2 = -50; break; //전진
      case ‘b': vel1 = -50; vel2 = 50; break; //후진
      case 'l': vel1 = -50; vel2 = -50; break; //좌회전
      case 'r': vel1 = 50; vel2 = 50; break; //우회전
      default : vel1 = 0; vel2 = 0; break; //정지
      }
    mx.setVelocity(vel1,vel2);
    }
    
    if (ctrl_c_pressed) break; //Ctrl+c입력시 탈출
    usleep(20*1000);
    gettimeofday(&end1,NULL);
    time1 =end1.tv_sec-start.tv_sec
    +(end1.tv_usec-start.tv_usec)/1000000.0;
    cout <<"vel1:"<<vel1<<','<<"vel2:"<<vel2<<
    “,time:"<<time1<< endl;
  }
  mx.close(); // 장치닫기
  return 0;
}
