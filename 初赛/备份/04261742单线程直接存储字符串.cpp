#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

struct Graph {
    int lnum;   //边数
    int pnum;   //点数
    int list[280000];
    int fg[280000][50];    //正向图
    int bg[280000][50];    //反向图
    char fgl[280000];   //出度
    char bgl[280000];   //入度
    char strlen[280000]; //str长度
    char str[280000][10];  //映射：索引 -> str
} graph;


struct AnswerBuffer{
    char *ans[5];
    int len[5];
    int num;
} ab;

typedef struct stat Stat_t;

void loadData(string filename) {
    //cout<<"enter"<<endl;

    int idx = 0;
    memset(graph.fgl, 0, sizeof(char) * 280000);
    memset(graph.bgl, 0, sizeof(char) * 280000);
    memset(graph.strlen, 0, sizeof(char) * 280000);

    int fd = open(filename.c_str(), O_RDONLY);
    Stat_t s;
    fstat(fd, &s);
    int bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);
    int l = 0;
    int r = 1;

    while(r <= bufLen) {
        if(buffer[r] == '\n') {
            int st = 0, end = 0;
            int cur = r;
            while(buffer[cur] != ',') cur--;
            cur--;
            int bit = 1;
            while(buffer[cur] != ',') {
                end += bit * (buffer[cur] - '0');
                bit *= 10;
                cur--;
            }
            cur--;
            bit = 1;
            while(cur >= l) {
                st += bit * (buffer[cur] - '0');
                bit *= 10;
                cur--;
            }

            graph.lnum++;
            //cout<<graph.lnum<<" "<<st<<" "<<end<<endl;

            if(graph.strlen[st] == 0) {
                graph.list[idx++] = st;
                int bit = 1000000000, len = 0, cur = st;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[st][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.str[st][len++] = ',';
                graph.strlen[st] = len;

            } 
            if(graph.strlen[end] == 0) {
                graph.list[idx++] = end;
                int bit = 1000000000, len = 0, cur = end;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[end][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.str[end][len++] = ',';
                graph.strlen[end] = len;

            }

            graph.fg[st][graph.fgl[st]++] = end;
            graph.bg[end][graph.bgl[end]++] = st;

            l = r + 1;
            r += 2;
        } else {
            r++;
        }
    }
    //cout<<"enter"<<endl;

    munmap(buffer, bufLen);
    close(fd);

    //cout<<"finish"<<endl;

    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + idx);

    for(int ii = 0; ii < graph.pnum; ii++){
        int i = graph.list[ii];
        sort(graph.fg[i], graph.fg[i] + graph.fgl[i]);
    }
    for(int ii = 0; ii < graph.pnum; ii++){
        int i = graph.list[ii];
        sort(graph.bg[i], graph.bg[i] + graph.bgl[i]);
    }

}

void saveAnswer(string filename) {

    char data[10];
    int offset = 0;

    int cur = ab.num;
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
        fwrite(ab.ans[i], sizeof(char), ab.len[i], fp);
    }
    fclose(fp);
}

void prune(int cur, int target, bool *visit, char *distance, int dis, int *update, int &udlen) {
    for(int i = 0; i < graph.bgl[cur]; i++) {
        int t = graph.bg[cur][i];
        if(t < target) continue;
        if(visit[t]) continue;
        if(dis < distance[t]) {
            distance[t] = dis;
            update[udlen++] = t;
        }

        if(dis == 3) continue;

        visit[t] = true;
        prune(t, target, visit, distance, dis + 1, update, udlen);
        visit[t] = false;
    }
}

void dfs(int cur, int target, bool *visit, char *distance, int *path, int len) {
    for(int i = 0; i < graph.fgl[cur]; i++) {
        int t = graph.fg[cur][i];

        if(t < target) continue;

        if(visit[t] || (len > 3 && (len + distance[t] > 7))) continue;
        //180ms
        if(distance[t] == 1 && len > 1) {
            ab.num++;
            path[len] = t;
            char *start = ab.ans[len - 2] + ab.len[len - 2];
            int offset = 0;
            for(int j = 0; j <= len; j++) {
                memcpy(start + offset, graph.str[path[j]], graph.strlen[path[j]]);
                offset += graph.strlen[path[j]];
            }
            start[offset - 1] = '\n';
            ab.len[len - 2] += offset;
        }

        if(len < 6) {
            visit[t] = true;
            path[len] = t;
            dfs(t, target, visit, distance, path, len + 1);
            visit[t] = false;
        }
    }
}


void checkCircle() {

    //Mem Init
    bool *visit = new bool[280000];
    char *distance = new char[280000];
    int *update = new int[50 * 50 * 50];
    memset(visit, false, 280000 * sizeof(bool));
    memset(distance, 10, 280000 * sizeof(char));
    int *path = new int[7];

    //Init answer buffer
    ab.ans[0] = new char[3 * 500000 * 11];
    ab.ans[1] = new char[4 * 500000 * 11];
    ab.ans[2] = new char[5 * 1000000 * 11];
    ab.ans[3] = new char[6 * 2000000 * 11];
    ab.ans[4] = new char[7 * 3000000 * 11];
    memset(ab.len, 0, 5 * sizeof(int));
    ab.num = 0;


    for(int ii = 0; ii < graph.pnum; ii++){     
        int i = graph.list[ii];
        //cout<<i<<endl;
        int udlen = 0;
        visit[i] = true;
        path[0] = i;
        prune(i, i, visit, distance, 1, update, udlen);
        //370ms
        dfs(i, i, visit, distance, path, 1);
        for(int j = 0; j < udlen; j++) distance[update[j]] = 10;
        visit[i] = false;
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