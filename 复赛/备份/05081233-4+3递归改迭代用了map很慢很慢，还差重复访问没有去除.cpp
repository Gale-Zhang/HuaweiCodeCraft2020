#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#define TNUM 8
#define VNUM 2000000

struct Line {
    int point;
    int weight;
};

struct Graph {
    int lnum;   //边数
    int pnum;   //点数
    int list[VNUM];
    int id[VNUM];
    Line out[VNUM][256];    //出边
    Line in[VNUM][256];     //入边
    char odr[VNUM]; //出度
    char idr[VNUM]; //入度
    char strlen[VNUM];  //str长度
    char str[VNUM][11]; //映射：索引 -> str
} graph;

struct AnswerBuffer{
    char *ans[TNUM][5];
    int len[TNUM][5];
    int num[TNUM];
} ab;

bool cmp(Line a, Line b) {
    return a.point < b.point;
}


void loadData(string filename) {

    int idx = 0;
    memset(graph.odr, 0, sizeof(char) * VNUM);
    memset(graph.idr, 0, sizeof(char) * VNUM);
    memset(graph.strlen, 0, sizeof(char) * VNUM);

    int fd = open(filename.c_str(), O_RDONLY);
    struct stat s;
    fstat(fd, &s);
    int bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);

    int cur = 0;
    unordered_map<int, int> idxmap;//{{},VNUM};
    idxmap.reserve(VNUM);

    while(cur < bufLen) {

        ++graph.lnum;

        int st = 0, end = 0, cost = 0;
        int stIdx = 0, endIdx = 0;
        char c;

        int last = cur;
        while((c = buffer[cur]) != ',') {
            st = st * 10 + (c - '0');
            ++cur;
        }
        if(idxmap.find(st) == idxmap.end()) {
            graph.list[idx] = st;
            stIdx = idx;
            graph.id[stIdx] = st;
            idxmap[st] = idx++;
            memcpy(graph.str[stIdx], buffer + last, cur - last);
            graph.strlen[stIdx] = cur - last;
            graph.str[stIdx][graph.strlen[stIdx]++] = ',';
        } else {
            stIdx = idxmap[st];
        }
        ++cur;

        last = cur;
        while((c = buffer[cur]) != ',') {
            end = end * 10 + (c - '0');
            ++cur;
        }
        if(idxmap.find(end) == idxmap.end()) {
            graph.list[idx] = end;
            endIdx = idx;
            graph.id[endIdx] = end;
            idxmap[end] = idx++;
            memcpy(graph.str[endIdx], buffer + last, cur - last);
            graph.strlen[endIdx] = cur - last;
            graph.str[endIdx][graph.strlen[endIdx]++] = ',';
        } else {
            endIdx = idxmap[end];
        }
        ++cur;

        // while((c = buffer[cur]) != '\r') {
        //     cost = cost * 10 + (c - '0');
        //     ++cur;
        // }
        // cur += 2;
        while((c = buffer[cur]) != '\n') {
            cost = cost * 10 + (c - '0');
            ++cur;
        }
        ++cur;

        graph.out[stIdx][graph.odr[stIdx]++] = {end, cost};
        graph.in[endIdx][graph.idr[endIdx]++] = {st, cost};
    }
    munmap(buffer, bufLen);
    close(fd);

    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + graph.pnum);
    for(int i = 0; i < graph.pnum; i++) {
        graph.list[i] = idxmap[graph.list[i]];
    }

    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.out[i], graph.out[i] + graph.odr[i], cmp);
        for(int j = 0; j < graph.odr[i]; j++) {
            graph.out[i][j].point = idxmap[graph.out[i][j].point];
        }
    }
    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.in[i], graph.in[i] + graph.idr[i], cmp);
        for(int j = 0; j < graph.idr[i]; j++) {
            graph.in[i][j].point = idxmap[graph.in[i][j].point];
        }
    }

}

void saveAnswer(string filename) {

    char data[10];
    int offset = 0;

    int cur = 0;
    for(int i = 0; i < TNUM; i++) {
        cur += ab.num[i];
        //cout<<"thread "<<i<<" find "<<ab.num[i]<<"\t circle!"<<endl;
    }
    int bit = 1000000000;
    while(bit > cur) bit /= 10;
    while(bit > 0) {
        data[offset++] = '0' + (cur / bit);
        cur %= bit;
        bit /= 10;
    }
    data[offset++] = '\n';

    FILE *fp = fopen(filename.c_str(), "w");
    fwrite(data, sizeof(char), offset, fp);
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < TNUM; j++) {
            fwrite(ab.ans[j][i], sizeof(char), ab.len[j][i], fp);
        }
    }
    fclose(fp);
    //exit(0);
}

struct Path{
    int p1;
    int p2;
    int w1;
    int w2;
};

inline bool valid(double a, double b) {
    return b <= a * 3.0 && b >= a * 0.2;
}

int border[TNUM + 1];

bool cmp1(Path p1, Path p2) {
    if(p1.p1 != p2.p1) return p1.p1 < p2.p1;
    else return p1.p2 < p2.p2; 
}

void *check(void *arg) {
    int idx = *(int*)arg;

    //Init answer buffer
    ab.ans[idx][0] = (char*)malloc(50 * 1024 * 1024);
    ab.ans[idx][1] = (char*)malloc(100 * 1024 * 1024);
    ab.ans[idx][2] = (char*)malloc(200 * 1024 * 1024);
    ab.ans[idx][3] = (char*)malloc(350 * 1024 * 1024);
    ab.ans[idx][4] = (char*)malloc(700 * 1024 * 1024);
    memset(ab.len[idx], 0, 5 * sizeof(int));
    ab.num[idx] = 0;

    unordered_map<int, int> map1;
    unordered_map<int, vector<Path>> map3;
    map1.reserve(50);
    map3.reserve(10000);

    for(int ii = border[idx]; ii < border[idx + 1]; ++ii){     
        int i = graph.list[ii];

        //backward 寻找能够三步到达终点的点，保存到map3，以及能一步到达终点的点，保存到map1
        // for(int j = 0; j < graph.idr[i]; j++) {
        //     int t1 = graph.in[i][j].point;
        //     map1[t1] = graph.in[i][j].weight;
        //     if(graph.id[t1] > graph.id[i]) {
        //         for(int k = 0; k < graph.idr[t1]; k++) {
        //             int t2  = graph.in[t1][k].point;
        //             if(graph.id[t2] > graph.id[i] && t2 != i && valid(graph.in[t1][k].weight, graph.in[i][j].weight)) {
        //                 for(int l = 0; l < graph.idr[t2]; l++) {
        //                     int t3 = graph.in[t2][l].point;
        //                     if(graph.id[t3] > graph.id[i] && t3 != i && t3 != t1 && valid(graph.in[t2][l].weight, graph.in[t1][k].weight)) {
        //                         map3[t3].push_back({t2, t1, graph.in[t2][l].weight, graph.in[i][j].weight});
        //                         //sort(map3[t3].begin(), map3[t3].end(), cmp1);
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }

        // for(auto &kv : map3) {
        //     sort(kv.second.begin(), kv.second.end(), cmp1);
        // }
        //长度为3的环
        // for(int j = 0; j < graph.odr[i]; j++){
        //     int t1 = graph.out[i][j].point;
        //     if(graph.id[t1] > graph.id[i]) {
        //         for(int k = 0; k < graph.odr[t1]; k++) {
        //             int t2 = graph.out[t1][k].point;
        //             if(graph.id[t2] > graph.id[i] && map1.find(t2) != map1.end()) {
        //                 double a = graph.out[i][j].weight, b = graph.out[t1][k].weight, c = map1[t2];
        //                 if(valid(a, b) && valid(b, c) && valid(c, a)){
        //                     ++ab.num[idx];
        //                     char *start = ab.ans[idx][0] + ab.len[idx][0];
        //                     int offset = 0;
        //                     memcpy(start + offset, graph.str[i], graph.strlen[i]);
        //                     offset += graph.strlen[i];
        //                     memcpy(start + offset, graph.str[t1], graph.strlen[t1]);
        //                     offset += graph.strlen[t1];
        //                     memcpy(start + offset, graph.str[t2], graph.strlen[t2]);
        //                     offset += graph.strlen[t2];
        //                     start[offset - 1] = '\n';
        //                     ab.len[idx][0] += offset;                        
        //                 }
        //             }
        //         }
        //     }
        // }

        // //长度为4-7的环---???ms
        // for(int j = 0; j < graph.odr[i]; j++){
        //     int t1 = graph.out[i][j].point;
        //     if(graph.id[t1] > graph.id[i]) {
        //         if(map3.find(t1) != map3.end()) {//重复问题还没解决
        //             for(Path p : map3[t1]) {
        //                 if(p.p2 != t1 && valid(graph.out[i][j].weight, p.w1) && valid(p.w2, graph.out[i][j].weight)) {
        //                     ++ab.num[idx];
        //                     if(graph.id[i] == 6000 && graph.id[t1] == 6001 && graph.id[p.p1] == 6002 && graph.id[p.p2] == 6005) {
        //                         cout<<graph.out[i][j].weight<<" "<<p.w1<<" "<<p.w2<<endl;
        //                     }
        //                     char *start = ab.ans[idx][1] + ab.len[idx][1];
        //                     int offset = 0;
        //                     memcpy(start + offset, graph.str[i], graph.strlen[i]);
        //                     offset += graph.strlen[i];
        //                     memcpy(start + offset, graph.str[t1], graph.strlen[t1]);
        //                     offset += graph.strlen[t1];
        //                     memcpy(start + offset, graph.str[p.p1], graph.strlen[p.p1]);
        //                     offset += graph.strlen[p.p1];
        //                     memcpy(start + offset, graph.str[p.p2], graph.strlen[p.p2]);
        //                     offset += graph.strlen[p.p2];
        //                     start[offset - 1] = '\n';
        //                     ab.len[idx][1] += offset;     
        //                 }
        //             }
        //         }
        //         for(int k = 0; k < graph.odr[t1]; k++) {
        //             int t2 = graph.out[t1][k].point;
        //             if(graph.id[t2] > graph.id[i] && t2 != i && valid(graph.out[i][j].weight, graph.out[t1][k].weight)) {
        //                 if(map3.find(t2) != map3.end()) {
        //                     for(Path p : map3[t2]) {
        //                         if(valid(graph.out[t1][k].weight, p.w1) && valid(p.w2, graph.out[i][j].weight)) {
        //                             ++ab.num[idx];
        //                             char *start = ab.ans[idx][2] + ab.len[idx][2];
        //                             int offset = 0;
        //                             memcpy(start + offset, graph.str[i], graph.strlen[i]);
        //                             offset += graph.strlen[i];
        //                             memcpy(start + offset, graph.str[t1], graph.strlen[t1]);
        //                             offset += graph.strlen[t1];
        //                             memcpy(start + offset, graph.str[t2], graph.strlen[t2]);
        //                             offset += graph.strlen[t2];
        //                             memcpy(start + offset, graph.str[p.p1], graph.strlen[p.p1]);
        //                             offset += graph.strlen[p.p1];
        //                             memcpy(start + offset, graph.str[p.p2], graph.strlen[p.p2]);
        //                             offset += graph.strlen[p.p2];
        //                             start[offset - 1] = '\n';
        //                             ab.len[idx][2] += offset;     
        //                         }
        //                     }
        //                 }
        //                 for(int l = 0; l < graph.odr[t2]; l++) {
        //                     int t3 = graph.out[t2][l].point;
        //                     if(graph.id[t3] > graph.id[i] && t3 != t1 && t3 != i && valid(graph.out[t1][k].weight, graph.out[t2][l].weight)) {
        //                         if(map3.find(t3) != map3.end()) {
        //                             for(Path p : map3[t3]) {
        //                                 if(valid(graph.out[t2][l].weight, p.w1) && valid(p.w2, graph.out[i][j].weight)) {
        //                                     ++ab.num[idx];
        //                                     char *start = ab.ans[idx][3] + ab.len[idx][3];
        //                                     int offset = 0;
        //                                     memcpy(start + offset, graph.str[i], graph.strlen[i]);
        //                                     offset += graph.strlen[i];
        //                                     memcpy(start + offset, graph.str[t1], graph.strlen[t1]);
        //                                     offset += graph.strlen[t1];
        //                                     memcpy(start + offset, graph.str[t2], graph.strlen[t2]);
        //                                     offset += graph.strlen[t2];
        //                                     memcpy(start + offset, graph.str[t3], graph.strlen[t3]);
        //                                     offset += graph.strlen[t3];
        //                                     memcpy(start + offset, graph.str[p.p1], graph.strlen[p.p1]);
        //                                     offset += graph.strlen[p.p1];
        //                                     memcpy(start + offset, graph.str[p.p2], graph.strlen[p.p2]);
        //                                     offset += graph.strlen[p.p2];
        //                                     start[offset - 1] = '\n';
        //                                     ab.len[idx][3] += offset;     
        //                                 }
        //                             }
        //                         }
        //                         for(int m = 0; m < graph.odr[t3]; m++) {
        //                             int t4 = graph.out[t3][m].point;
        //                             if(graph.id[t4] > graph.id[i] && t4 != t2 && t4 != t1 && t4 != i && valid(graph.out[t2][l].weight, graph.out[t3][m].weight)) {
        //                                 if(map3.find(t4) != map3.end()) {
        //                                     for(Path p : map3[t4]) {
        //                                         if(valid(graph.out[t3][m].weight, p.w1) && valid(p.w2, graph.out[i][j].weight)) {
        //                                             ++ab.num[idx];
        //                                             char *start = ab.ans[idx][4] + ab.len[idx][4];
        //                                             int offset = 0;
        //                                             memcpy(start + offset, graph.str[i], graph.strlen[i]);
        //                                             offset += graph.strlen[i];
        //                                             memcpy(start + offset, graph.str[t1], graph.strlen[t1]);
        //                                             offset += graph.strlen[t1];
        //                                             memcpy(start + offset, graph.str[t2], graph.strlen[t2]);
        //                                             offset += graph.strlen[t2];
        //                                             memcpy(start + offset, graph.str[t3], graph.strlen[t3]);
        //                                             offset += graph.strlen[t3];
        //                                             memcpy(start + offset, graph.str[t4], graph.strlen[t4]);
        //                                             offset += graph.strlen[t4];
        //                                             memcpy(start + offset, graph.str[p.p1], graph.strlen[p.p1]);
        //                                             offset += graph.strlen[p.p1];
        //                                             memcpy(start + offset, graph.str[p.p2], graph.strlen[p.p2]);
        //                                             offset += graph.strlen[p.p2];
        //                                             start[offset - 1] = '\n';
        //                                             ab.len[idx][4] += offset;     
        //                                         }
        //                                     }
        //                                 }
        //                             }
        //                         }
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }        
        map1.clear();
        map3.clear();
    } 

    timeval t; 
    gettimeofday(&t, NULL);
    cout<<"thread "<<idx<<" finished \t"<<1000000*(t.tv_sec) + (t.tv_usec)<<endl;

}

void checkCircle() {
    border[0] = 0;
    border[1] = 25 * graph.pnum / 1000;
    border[2] = 55 * graph.pnum / 1000;
    border[3] = 90 * graph.pnum / 1000;
    border[4] = 130 * graph.pnum / 1000;
    border[5] = 175 * graph.pnum / 1000;
    border[6] = 230 * graph.pnum / 1000;
    border[7] = 350 * graph.pnum / 1000;
    border[8] = graph.pnum;
    pthread_t tid[TNUM];

    int *arg = new int[TNUM];
    for(int i = 0; i < TNUM; ++i) {
        arg[i] = i;
    }
    for(int i = 0; i < TNUM; ++i) {
        pthread_create(&tid[i], NULL, check, arg + i);
    }
    for(int i = 0; i < TNUM; ++i) {
        pthread_join(tid[i], NULL);
    }
}

#define print_time true

int main(int argc,char* argv[]) {

    string dataFileName = argv[1];
    string answerFileName = argv[2];
#if print_time
    timeval start, end; 
    gettimeofday(&start, NULL);
#endif
    loadData(dataFileName);
#if print_time
    gettimeofday(&end, NULL);
    cout<<"load data \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
#endif
    checkCircle();
    //checkCircle();
#if print_time
    gettimeofday(&start, NULL);
    cout<<"check circle \t"<<1000000*(start.tv_sec - end.tv_sec) + (start.tv_usec - end.tv_usec)<<endl;
#endif
    saveAnswer(answerFileName);
#if print_time 
    gettimeofday(&end, NULL);
    cout<<"save answer \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
#endif
    return 0;
}