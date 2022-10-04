# include <iostream>
# include <fstream>
# include <vector>
# include <string>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h> 
# include <cmath>
# include <limits>
# include <thread>
# include <windows.h>
# include <sstream>
using namespace std;

int *numberlist;
int numberlistsize;

class Sorting {
    public:

    static void WaitForMultipleObjectsExpand(HANDLE * handles, DWORD count) {
        DWORD index = 0;
        DWORD result = 0;
        DWORD handleCount = count;

        /// 每64個Handle分為一組
        while (handleCount >= MAXIMUM_WAIT_OBJECTS) {
            WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, &handles[index], TRUE, INFINITE);

            handleCount -= MAXIMUM_WAIT_OBJECTS;
            index += MAXIMUM_WAIT_OBJECTS;
        } // while

        if (handleCount > 0) {
            WaitForMultipleObjects(handleCount, &handles[index], TRUE, INFINITE);
        } // if
    } // WaitForMultipleObjectsExpand

    static void FastBubble( long start, long end ) {	// start index, end index
        bool hasChanged = false ;
        long total = end-start+1 ;
		int temp = 0;
        for ( int pass = 1; pass < total && !hasChanged ; ++pass ) {		
            hasChanged = true;
            for ( int i = 0; i < total-pass; ++i ) {
                if ( numberlist[start+i] > numberlist[start+i+1] ) {
                    temp = numberlist[start+i];
                    numberlist[start+i] = numberlist[start+i+1];
                    numberlist[start+i+1] = temp;
                    hasChanged = false ;
                } // if
            } // for
        } // for
	} // FastBubble

    static void Merge(int front, int mid, int end) {   
        int *tempArray = new int[end-front+1];
        for (int i = front, j = 0; i <= end; i++, j++) {
            tempArray[j] = numberlist[i];
        } // for
        
        int idxLeft = 0, idxRight = mid-front+1;
        for (int i = front; i <= end; i++) {
            if (idxLeft == mid-front+1) {
                numberlist[i] = tempArray[idxRight];
                idxRight++;
            } // if
            else if (idxRight == end-front+1) {
                numberlist[i] = tempArray[idxLeft];
                idxLeft++;
            } // else if
            else {
                if (tempArray[idxLeft] < tempArray[idxRight]) {
                    numberlist[i] = tempArray[idxLeft];
                    idxLeft++;
                } // if
                else {
                    numberlist[i] = tempArray[idxRight];
                    idxRight++;
                } // else
            } // else
        } // for

        delete[] tempArray;
    } // Merge

    static void MergeKArray(int k) {
        long tempk = k, start = 0, last = 0;
        long amount = numberlistsize/tempk;
        double tempk_d = 0;
        while ( tempk >= 2 ) {
            tempk_d = tempk;
            for ( long i = 0; i < ceil(tempk_d/2); i++ ) {
                start = i*amount*2;
                last = start+amount*2-1;
                if ( i < ceil(tempk_d/2) - 1 )
                    Merge(start, start+amount-1, last);
                else {
                    if ( tempk % 2 == 0 )
                        Merge(start, start+amount-1, numberlistsize-1);
                } // else
            } // for
            tempk = ceil( tempk_d / 2 );
            amount = amount*2;
        } // while

    } // MergeKArray

    static void MergeKArray_Process(int k) {     
        int *remainlist;
        int filenum = 0, buffer = 0, lastfilenum = 0;
        bool notRemain = false;
        int tempk = k, start = 0, last = 0, handleNum = 0;
        double tempk_d = tempk;
        int amount = numberlistsize/k;
        FILE *fout, *fin ;

        while ( tempk >= 2 ) {        // tempk:now have how many to merge
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            handleNum = floor(tempk_d/2);
            HANDLE *lpHandles = new HANDLE[handleNum];
            HANDLE *lpThreads = new HANDLE[handleNum];

            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);
            ZeroMemory( &pi, sizeof(pi) );

            filenum = 0;
            while ( filenum < ceil(tempk_d/2) ) {
                start = filenum*amount*2;
                last = start+amount*2-1;
                lastfilenum = 0;
                stringstream ss, ss_par;
                ss.str("");
                ss_par.str("");
                ss << (filenum+1) << ".txt";
                char outputFname[ss.str().size()+1];
                strcpy(outputFname, ss.str().c_str());

                if ( filenum < ceil(tempk_d/2) - 1 ) {
                    fout = fopen ( outputFname ,  "wb" );
                    ss_par << (filenum+1) << ".txt " << amount-1 << " " << amount*2;
                    fwrite( &numberlist[start], sizeof(int), last-start+1, fout );
                    fclose(fout);
                } // if
                else {
                    if ( tempk % 2 == 0 ) {
                        fout = fopen ( outputFname ,  "wb" );
                        ss_par << (filenum+1) << ".txt " << amount-1 << " " << numberlistsize-start;
                        lastfilenum = numberlistsize - start;
                        fwrite( &numberlist[start], sizeof(int), lastfilenum, fout );
                        fclose(fout);
                        notRemain = true;
                    } // if
                    else 
                        break;
                } // else

                char output_par[ss_par.str().size()+1];
                strcpy(output_par, ss_par.str().c_str());
                if (!CreateProcess("Merge.exe", output_par, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                    cout << "ERROR: Failed to create process" << GetLastError() << endl;
                    return;
                } // if
                lpHandles[filenum] = pi.hProcess;
                lpThreads[filenum] = pi.hThread;
                filenum++ ;
            } // while
            
            WaitForMultipleObjectsExpand( lpHandles, (DWORD)filenum );
            start = filenum*amount*2;
            if ( !notRemain && start < numberlistsize ) {
                remainlist = new int[numberlistsize-start];
                for ( int i=0, j=start; j<numberlistsize; i++, j++ ) {
                    remainlist[i] = numberlist[j];
                } // for
            } // if
            delete[] numberlist;
            
            int i = 0, j = 0;
            numberlist = new int[numberlistsize];
            for (int t = 0; t < filenum; t++) {
                stringstream ss ;
                string s;
                ss << (t+1) << ".txt";
                char outputFname[ss.str().size()+1];
                strcpy(outputFname, ss.str().c_str());
                fin = fopen ( outputFname ,  "rb" );
                if ( t == filenum-1 && lastfilenum > 0 ) {
                    fread( &numberlist[i], sizeof(int), lastfilenum, fin );
                    i = i + lastfilenum;
                } // if
                else {
                    fread( &numberlist[i], sizeof(int), amount*2, fin );
                    i = i + amount*2;
                } // else
                fclose ( fin );
                remove(outputFname);
                CloseHandle( lpHandles[t] );
                CloseHandle( lpThreads[t] );
            } // for
            if ( !notRemain ) {
                for ( j = 0; i<numberlistsize; i++, j++ ) {
                    numberlist[i] = remainlist[j];
                } // for
                delete[] remainlist;
            } // if

            delete[] lpHandles;
            delete[] lpThreads;
            tempk = ceil( tempk_d / 2 );
            tempk_d = tempk;
            amount = amount*2 ;
            notRemain = false;
        } // while

    } // MergeKArray_Process

    static void MergeKArray_Thread(int k) {
        long tempk = k, start = 0, last = 0;
        long amount = numberlistsize/tempk;
        double tempk_d = tempk;
        while ( tempk/2 >= 1 ) {
            vector<thread> threads;
            tempk_d = tempk;
            for ( long i = 0; i < ceil(tempk_d/2); i++ ) {
                start = i*amount*2;
                last = start+amount*2-1;
                if ( i < ceil(tempk_d/2) - 1 )
                    threads.push_back(move(thread(Merge, start, start+amount-1, last)));
                else {
                    if ( tempk % 2 == 0 )
                        threads.push_back(move(thread(Merge, start, start+amount-1, numberlistsize-1)));
                } // else
            } // for
            
            for (int i = 0; i < threads.size(); i++) {
                threads[i].join();
            } // for

            threads.clear();
            tempk = ceil( tempk_d / 2 );
            amount = amount*2 ;
        } // while
        
    } // MergeSort

    void Method1() {        // bubble sort
        FastBubble(0, numberlistsize-1);
    } // Method1

    void Method2(int k) {   // same process, k bubble sort, then merge k sorted array
        long count = 0, i = 0;
        long amount = numberlistsize/k; 
        while ( count < numberlistsize) {
            if ( i < k-1 )
                FastBubble(count, count+amount-1);
            else
                FastBubble(count, numberlistsize-1);
            count = count + amount;
            i++;
        } // while
        MergeKArray(k);
    } // Method2

    void Method3(int k) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        HANDLE *lpHandles = new HANDLE[k];
        HANDLE *lpThreads = new HANDLE[k];
        int count = 0, lastfilenum = 0;
        long amount = floor(numberlistsize/k);
        int buffer = 0;
        FILE * fout, * fin ;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        for (int i = 0; i < k ; i++ ) {
            stringstream ss, ss_par;
            ss.str("");
            ss_par.str("");
            ss << (i+1) << ".txt";
            char outputFname[ss.str().size()+1];
            strcpy(outputFname, ss.str().c_str());

            if ( i == k-1 ) {
                fout = fopen ( outputFname ,  "wb" );
                ss_par << (i+1) << ".txt " << (numberlistsize-count) << "";
                lastfilenum = numberlistsize - count;
                fwrite( &numberlist[count], sizeof(int), lastfilenum, fout );
                fclose(fout);
            } // if
            else {
                fout = fopen ( outputFname ,  "wb" );
                ss_par << (i+1) << ".txt " << (amount) << "";
                fwrite( &numberlist[count], sizeof(int), amount, fout );
                fclose(fout);
            } // else
            char output_par[ss_par.str().size()+1];
            strcpy(output_par, ss_par.str().c_str());

            if (!CreateProcess("FastBubble.exe", output_par, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                cout << "ERROR: Failed to create process" << GetLastError() << endl;
                return;
            } // if
            lpHandles[i] = pi.hProcess;
            lpThreads[i] = pi.hThread;
            count = count + amount;
        } // for

        WaitForMultipleObjectsExpand( lpHandles, (DWORD)k );
        delete[] numberlist;

        int j = 0;
        numberlist = new int[numberlistsize];
        for (int i = 0; i < k; i++) {
            stringstream ss ;
            string s;
            ss << (i+1) << ".txt";
            char outputFname[ss.str().size()+1];
            strcpy(outputFname, ss.str().c_str());
            fin = fopen ( outputFname ,  "rb" );
            if ( i == k-1 ) {
                fread( &numberlist[j], sizeof(int), lastfilenum, fin );
                j = j + lastfilenum;
            } // if
            else {
                fread( &numberlist[j], sizeof(int), amount, fin );
                j = j + amount;
            } // else
            fclose ( fin );
            remove(outputFname);
            CloseHandle( lpHandles[i] );
            CloseHandle( lpThreads[i] );
        } // for

        delete[] lpHandles;
        delete[] lpThreads;

        MergeKArray_Process(k);
    } // Method3

    void Method4(int k) {
        vector<thread> threads;
        long count = 0; 
        long amount = numberlistsize/k;   // amount of every cut
        for (int i = 0; i < k ; i++ ) {
            if ( i < k-1 )
                threads.push_back(move(thread(Sorting::FastBubble, count, count+amount-1)));
            else
                threads.push_back(move(thread(Sorting::FastBubble, count, numberlistsize-1)));
            count = count + amount;
        } // for

        for (int i = 0; i < threads.size(); i++) {
            threads[i].join();
        } // for
        threads.clear();
        MergeKArray_Thread(k);
    } // Method4

}; // class Sorting

int main( int argc, char* argv[] ) {
    Sorting st;
    ifstream fin;
    ofstream fout;
    //FILE *fin ;
    string inputFname, outputFname, s;
    clock_t start, finish;  
    bool kInputError = false; 
    char* dt;
    double duration;  
    int k = 0, method = 0, fileAmount = 0, i = 0;
    cout << "\n請輸入檔案名稱(0: end of program): " << endl;
    cin >> inputFname;
    while ( inputFname != "0" ) {
        kInputError = false;
        fileAmount = 0;
        fin.open( inputFname + ".txt" );
        //fin = fopen((inputFname + ".txt").c_str(), "r");
        if ( fin.is_open() ) {    //.is_open()
            i = 6;
            while ( inputFname[i] >= '0' && inputFname[i] <= '9') {
                fileAmount = 10*fileAmount + (inputFname[i]-'0');
                i++;
            } // for

            fileAmount = fileAmount*10000;
            numberlistsize = fileAmount;
            numberlist = new int[fileAmount];

            cout << "請輸入方法編號(1, 2, 3, 4): " << endl;
            cin >> method;
            if ( method == 2 || method == 3 || method == 4 ) {
                cout << "請輸入要切成幾份(>0): " << endl;
                cin >> k;
                if ( k <= 0 || k > numberlistsize ) 
                    kInputError = true;
            } // if
            
            start = clock();
            if ( ( method == 1 || method == 2 || method == 3 || method == 4 ) && !kInputError ) {
                i = 0;
                while ( getline(fin, s) ) {
                    numberlist[i] = atoi(s.c_str());
                    i++;
                } // while
                fin.close();
                //fread( numberlist, sizeof(int), numberlistsize, fin );
                //fclose(fin);

                if ( method == 1 )
                    st.Method1();
                else if ( method == 2 )
                    st.Method2(k);
                else if ( method == 3 )
                    st.Method3(k);
                else if ( method == 4 )
                    st.Method4(k);
                finish = clock();
                duration = (double)(finish - start) / CLOCKS_PER_SEC;
                time_t now = time(0);
                dt = ctime(&now);

                outputFname = inputFname + "_output" + to_string(method);
                fout.open(outputFname + ".txt");
                if ( fout.is_open() ) {
                    fout << "Sort:" << endl;
                    for ( int i = 0; i<numberlistsize; i++ ) {
                        fout << numberlist[i] << endl;
                    } // for
                    fout << "CPU Time: " << duration << endl;
                    fout << "Output Time: " << dt;
                    fout.close();
                } // if
                delete[] numberlist; 
                
                cout << "\nCPU Time: " << duration << "seconds" << endl;
                cout << "Output Time: " << dt << endl << endl;
            } // if
            else {
                if ( kInputError )
                    cout << "切的份數輸入錯誤！" << endl << endl;
                else
                    cout << "方法選擇輸入錯誤！" << endl << endl;
            } // else
        } // if
        else
            cout << "檔案名稱輸入錯誤！" << endl << endl;
        cout << "請輸入檔案名稱(0: end of program): " << endl;
        cin >> inputFname;
    }// while

} // main