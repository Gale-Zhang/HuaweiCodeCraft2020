#include <bits/stdc++.h>
using namespace std;

struct Point{
    char x;
    char y;
} p;

int main() {
	// bool flag = false;
 //    timeval start, end; 
 //    gettimeofday(&start, NULL);
	// for(int i = 0; i < 100000; i++) flag = true;
 //    gettimeofday(&end, NULL);
 //    cout<<"load data \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
	// for(int i = 0; i < 100000; i++) flag = !flag; 
 //    gettimeofday(&start, NULL);
 //    cout<<"check circle \t"<<1000000*(start.tv_sec - end.tv_sec) + (start.tv_usec - end.tv_usec)<<endl;
	// for(int i = 0; i < 100000; i++) flag = ~flag; 
 //    gettimeofday(&end, NULL);
 //    cout<<"save answer \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
    Point arr[100];
    memset(arr, 4, 100 * sizeof(Point));
    cout<<(int)arr[0].x<<" "<<(int)arr[0].y<<endl;
    Point t = arr[0];
    t.x = 0;
    t.y = 0;
    cout<<(int)arr[0].x<<" "<<(int)arr[0].y<<endl;
    arr[0].x = 0;
    arr[0].y = 0;
    cout<<(int)arr[0].x<<" "<<(int)arr[0].y<<endl;


    return 0;
}