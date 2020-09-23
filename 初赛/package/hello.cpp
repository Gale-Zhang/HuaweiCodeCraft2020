#include <bits/stdc++.h>

using namespace std;

int main() {

    timeval start, end; 
    int a = 5, b = 10;
    gettimeofday(&start, NULL);

    //memset(buffer, 10, sizeof(int) * 280000);
    for(int i = 0; i < 120000; i++) bool t = a == b;

    gettimeofday(&end, NULL);
    cout<<"time elpased \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
	return 0;
}