

    // auto t = clock();

    // int num = 0;
    // for(int i = 0; i < 5; i++) {
    //     for(int j = 0; j < 4; j++) {
    //         num += bucket[i][j].size();
    //     }
    // }

    // cout<<"data num : "<<num<<endl;
    // char *data = new char[num * 11 * 7];
    // int offset = sprintf(data, "%d\n", num);
    // cout<<"time "<<(clock() - t) / 1000<<endl;

    // for(int i = 0; i < 5; i++) {
    //     queue<vector<int>> q1, q2;
    //     while(!bucket[i][0].empty() && !bucket[i][1].empty()) {
    //         if(bucket[i][0].front()[0] < bucket[i][1].front()[0]) {
    //             q1.push(bucket[i][0].front());
    //             bucket[i][0].pop();
    //         } else {
    //             q1.push(bucket[i][1].front());
    //             bucket[i][1].pop();
    //         }
    //     }
    //     while(!bucket[i][0].empty()) {
    //         q1.push(bucket[i][0].front());
    //         bucket[i][0].pop();
    //     }
    //     while(!bucket[i][1].empty()) {
    //         q1.push(bucket[i][1].front());
    //         bucket[i][1].pop();
    //     }
    //     while(!bucket[i][2].empty() && !bucket[i][3].empty()) {
    //         if(bucket[i][2].front()[0] < bucket[i][3].front()[0]) {
    //             q2.push(bucket[i][2].front());
    //             bucket[i][2].pop();
    //         } else {
    //             q2.push(bucket[i][3].front());
    //             bucket[i][3].pop();
    //         }
    //     }
    //     while(!bucket[i][2].empty()) {
    //         q2.push(bucket[i][2].front());
    //         bucket[i][2].pop();
    //     }
    //     while(!bucket[i][3].empty()) {
    //         q2.push(bucket[i][3].front());
    //         bucket[i][3].pop();
    //     }
    //     while(!q1.empty() && !q2.empty()) {
    //         if(q1.front()[0] < q2.front()[0]) {
    //             int l = q1.front().size() - 1;
    //             for(int j = 0; j < l; j++) {
    //                 offset += sprintf(data + offset, "%d,", q1.front()[j]);
    //             }
    //             offset += sprintf(data + offset, "%d\n", q1.front()[l]);
    //             q1.pop();
    //         } else {
    //             int l = q2.front().size() - 1;
    //             for(int j = 0; j < l; j++) {
    //                 offset += sprintf(data + offset, "%d,", q2.front()[j]);
    //             }
    //             offset += sprintf(data + offset, "%d\n", q2.front()[l]);
    //             q2.pop();
    //         }
    //     }
    //     while(!q1.empty()) {
    //         int l = q1.front().size() - 1;
    //         for(int j = 0; j < l; j++) {
    //             offset += sprintf(data + offset, "%d,", q1.front()[j]);
    //         }
    //         offset += sprintf(data + offset, "%d\n", q1.front()[l]);
    //         q1.pop();
    //     }
    //     while(!q2.empty()) {
    //         int l = q2.front().size() - 1;
    //         for(int j = 0; j < l; j++) {
    //             offset += sprintf(data + offset, "%d,", q2.front()[j]);
    //         }
    //         offset += sprintf(data + offset, "%d\n", q2.front()[l]);
    //         q2.pop();            
    //     }
    // }

    // cout<<"write data finished "<<(clock() - t) / 1000<<" offset "<<offset<<endl;

    // int fd = open(filename.c_str(), O_CREAT|O_RDWR, 0666);
    // lseek(fd, 0, SEEK_END);
    // write(fd, "\0", offset);
    // close(fd);

    // cout<<"initial file finished "<<(clock() - t) / 1000<<endl;

    // fd = open(filename.c_str(), O_CREAT|O_RDWR, 0666);

    // cout<<"mmap finished "<<(clock() - t) / 1000<<endl;

    // int block = 2 * 1024 * 1024; //2M
    // int cur = 0;
    // while(cur + block < offset) {
    //     char *buffer = (char*)mmap(NULL, block, PROT_READ|PROT_WRITE, MAP_SHARED, fd, cur);
    //     cout<<0<<endl;
    //     memcpy(buffer, data + cur, block);
    //     cout<<1<<endl;
    //     munmap(buffer, block);
    //     cout<<2<<endl;
    //     cur += block;
    //     cout<<cur<<endl;
    // }
    // cout<<"3"<<endl;
    // char *buffer = (char*)mmap(NULL, block, PROT_READ|PROT_WRITE, MAP_SHARED, fd, cur);
    // memcpy(buffer, data + cur, offset - cur);
    // munmap(buffer, block);

    // close(fd);

    // cout<<"write file finished "<<(clock() - t) / 1000<<endl;


    // int fd = open(filename.c_str(), O_CREAT|O_RDWR|O_APPEND, 0666);
    // if(fd < 0) {
    //     cout<<"OPEN FILE ERROR!"<<endl;
    // }

    // char data[12];
    // int bufLen = 4096;//2 * 1024 * 1024;

    // int len = sprintf(data, "%d\n", bucket[0][0].size() + bucket[1][0].size() + bucket[2][0].size() + bucket[3][0].size() + bucket[4][0].size());

    // int count = 0;

    // int bucIdx = 0;
    // int thdIdx = 0;
    // int rowIdx = 0;
    // int colIdx = 0; 

    // while(bucIdx < 5) {

    //     lseek(fd, 0, SEEK_END);
    //     write(fd, "\0", bufLen);

    //     char *buffer = (char*)mmap(NULL, bufLen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, count * bufLen);
    //     memcpy(buffer, data, len);
    //     memset(data, '\0', len);

    //     int offset = len;
        
    //     while(bucIdx < 5) {
    //         if(bufLen - offset > 12) {
    //             len = sprintf(buffer + offset, "%d", bucket[bucIdx][0][rowIdx][colIdx]);
    //             offset += len;  
    //             colIdx++;
    //             if(colIdx == bucket[bucIdx][0][rowIdx].size()) {
    //                 buffer[offset++] = '\n';
    //                 colIdx = 0;
    //                 rowIdx++;
    //                 if(rowIdx == bucket[bucIdx][0].size()) {
    //                     rowIdx = 0;
    //                     bucIdx++;
    //                 }
    //             } else {
    //                 buffer[offset++] = ',';
    //             }
    //         } else {
    //             len = sprintf(data, "%d", bucket[bucIdx][0][rowIdx][colIdx]);   
    //             colIdx++;
    //             if(colIdx == bucket[bucIdx][0][rowIdx].size()) {
    //                 data[len++] = '\n';
    //                 colIdx = 0;
    //                 rowIdx++;
    //                 if(rowIdx == bucket[bucIdx][0].size()) {
    //                     rowIdx = 0;
    //                     bucIdx++;
    //                 }
    //             } else {
    //                 data[len++] = ',';
    //             }
    //             if(offset + len <= bufLen) {
    //                 memcpy(buffer + offset, data, len);
    //                 offset += len;
    //             } else {
    //                 memcpy(buffer + offset, data, bufLen - offset);
    //                 len = offset + len - bufLen;
    //                 for(int i = 0; i < len; i++) {
    //                     data[i] = data[i + bufLen - offset];
    //                 }
    //                 count++;
    //                 break;
    //             }
    //         }
    //     }
    //     munmap(buffer, bufLen);
    // }
    // close(fd);