/* object defination of:
    - commit
    - blob
        - hash_object
        - compress the new content
        -
    - tree
and helper classes and definations

note: zlib compression and decompression taken from  - https://gist.github.com/arq5x/5315739#file-zlib-example-cpp-L7 8
store = header + data
*/



#include "zlib.h"

#include "sha1.hpp"
// #include <string>
// #include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h> // removes
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "checkDir.h"
using namespace std;


#define MAX_FILE_NAME_LENGTH 1024
#define PATH_INDEX "git/index"
#define git_dir "git"
#define UPPER_BOUND 2147483647
#define SIZE_STORE  "git/size_store"
#define ROOT_PATH "."

// #define PATH_INDEX "p.txt"
string GIT_DIR(git_dir);
// int dot_flag=0;

void add_run(string location);

string hash_object(string content, string type){
    /*
    type : blob, commit, tree
    */
    const string header = type + " " + std::to_string(content.length())+'\0';
    SHA1 checksum;
    const string store= header+content;
    // cout << store << endl;
    checksum.update(store);
    const string hash = checksum.final();
    // cout << hash.substr(0,2) << endl;

    return hash;
}


int decimaltoOctal(int deciNum){

    // initializations
    int octalNum = 0, countval = 1;
    int dNo = deciNum;

    while (deciNum != 0) {

        // decimals remainder is calculated
        int remainder = deciNum % 8;

        // storing the octalvalue
        octalNum += remainder * countval;

        // storing exponential value
        countval = countval * 10;
        deciNum /= 8;
    }
    // cout << octalNum << endl;
    return octalNum;
}

string mysplit(string inLine){
    char line[MAX_FILE_NAME_LENGTH];
    strcpy(line, inLine.c_str());
    char* tokens[4];
    int k=0;

    char* token = strtok(line, " ");
    while (token != NULL){
        tokens[k]= token;
        token = strtok(NULL, " ");
        k+=1;
    }
    string curPath(tokens[k-1]);
    return curPath;
}

void updateIndexFile(string sha, string pathname){
    // get the file permissions

    int isPresent=1;
    const char* path= pathname.c_str();
    struct stat fb;
    if (stat(path, &fb)==-1){ // 0 when file exist
        // printf("cannot stat on %s location!\n", PATH);  //are you planning to print this?
        isPresent= 0; // is not present in working dir
    }
    if (isPresent){
        unsigned long var= (unsigned long)fb.st_mode;

        int mode= decimaltoOctal(var);
        string identifier= to_string(mode) + " "+ sha + " " + to_string(0) + " "+ pathname ;
        // todo
        cout<<"indetifier "<<identifier<<endl;
        string line;
        // read file content
        string index_data;
        ifstream myfile(PATH_INDEX);
        if(myfile.is_open()){
            while(getline(myfile,line)){
                // cout<<line<<"------"<<identifier<<endl;
                // if (line.compare(identifier)==0){
                //     return;
                char line_[MAX_FILE_NAME_LENGTH];
                strcpy(line_,line.c_str());
                char** contents=split_index_line(line_," ");
                string curr_pathname(contents[3]);
                if(curr_pathname.compare(pathname)!=0){
                    index_data+=line+"\n";
                }
                cout<<index_data<<endl;
            }
            index_data+=identifier+"\n"; ///this line
            myfile.close();
        }
        else{
            cout<<"Error: cannot find index file"<<endl;
            exit(0);
        }
        cout<<"time to<< write in index file"<<endl;
        // write to the index file
        ofstream fapp;
        fapp.open(PATH_INDEX, ofstream::trunc);
        fapp << index_data;
        fapp.close();

    }
    else{
        string line;
        string content="";
        // read file content
        ifstream myfile(PATH_INDEX);
        if(myfile.is_open()){
            while(getline(myfile, line)){
                string curPath= mysplit(line);
                if (curPath.compare(pathname)){
                    content+=line;
                }
            }
            myfile.close();
        }
        else{
            cout<<"Error"<<endl;
            exit(0);
        }
        ofstream ftrunc;
        ftrunc.open(PATH_INDEX, ofstream::trunc);
        ftrunc << content;
        ftrunc.close();
    }
}

void blobDir(char* dirname){
    /* creates a blob for all the files present in this directory. */
    DIR* dir = opendir(dirname); // open the directory location to read.

    if (dir == NULL){
        // handelling error
        fprintf(stderr, "opendir: '%s': %s\n", dirname, strerror(errno));
        return;
    }

    struct dirent* dir_reader; // to read the directory
    char newlocation[MAX_FILE_NAME_LENGTH] = "";
    while ((dir_reader = readdir(dir)) !=NULL){
   
        if ( (!strcmp(dir_reader->d_name, ".")) || (!strcmp(dir_reader->d_name, ".."))){
            // ignore "." and ".." directories
        }
        else{
            strcpy(newlocation, dirname);
            strcat(newlocation, "/");
            strncat(newlocation, dir_reader->d_name, strlen(dir_reader->d_name));
            string newLocation(newlocation);
            add_run(newLocation);
        }
    }
    closedir(dir);
}

auto return_split_content_from_sha(string sha){
    // get secret code and size from indexfile (SIZE_STORE)
    // decompress 
    // simply return
    string content;
    string path= GIT_DIR+"/objects/"+sha.substr(0,2)+"/"+sha.substr(2,38);
    // string path= ".git/objects/b5/22cff36efe4c57d1e3b1977531fa0dca7f5842";
    // const char* path= "/home/ac-optimus/implement_git/copy_sha";
    // cout<<"before reading"<<path<<endl;
    ifstream f_reader(path);
    string final_content;
    while(getline (f_reader, content)){
        final_content+=content+"\n";
     } // compressed data in content
    // pair<int, int> compressed_data= get_compressed_store_data(sha); //secret code, size_store
    // cout<<"content of path  "<< compressed_data.first<< " "<< compressed_data.second<<" "<< content<< " " << endl;

    // char* decompress_string= decompress(content, compressed_data.first, compressed_data.second);
    // print_string(decompress_string, compressed_data.second);
    // split based on "\n"
    long long int content_size=final_content.length();
    char content_[content_size];
    strcpy(content_,final_content.c_str());
    // cout<< "blah! "<<content_<<endl;
    int len=0;
    char** lines= split_index_line(content_,"\n",&len);

    vector<string> v;
    for(int i=0;i<len;i++){
        v.push_back(string(lines[i]));
    }
    return v;
}

auto return_content(string sha){
    string content;
    string path= GIT_DIR+"/objects/"+sha.substr(0,2)+"/"+sha.substr(2,38);
    ifstream f_reader(path);
    string final_content;
    while(getline (f_reader, content)){
        final_content+=content+"\n";
     }
    return final_content;
}

void write_object(string sha1, string content, string type){
    string path= GIT_DIR+"/objects/"+sha1.substr(0,2)+"/"+sha1.substr(2,38);
    string pathDir=GIT_DIR+"/objects/"+sha1.substr(0,2);
    char dirPath[MAX_FILE_NAME_LENGTH];
    strcpy(dirPath, pathDir.c_str());
    if (mkdir(dirPath,0777)==-1){
        // cout << "Error: cannot create directory"<< endl;
    }
    cout<< "writing at- "<<path<<endl;
    ofstream fptr;
    fptr.open(path, ofstream::trunc);
    fptr<<content;
    fptr.close();
}


void createBlob(string file_name){
    /* create a blob for given file*/
    string content="";
    string line;
    // read file content
    ifstream myfile(file_name);
    if(myfile.is_open()){
        while(getline(myfile,line)){
            // cout<<line<<endl;
            content+=line+"\n";// remove "\n" when compression in use
        }
        myfile.close();
    }
    else{
        cout<<"Error"<<endl;
    }
    string sha1=hash_object(content,"blob");

    write_object(sha1, content, "blob");
    // update file
    updateIndexFile(sha1, file_name);

}

void add_run(string location){
    /*
    checks  if location is file -- create a blob
            if directory then first look for the files and create blob for all of them
    */
    char location_new[MAX_FILE_NAME_LENGTH];
    strcpy(location_new, location.c_str());
    int status = isDir(location_new); // location is file or directory
    cout<<"status "<< status<<"--"<<location<<endl;
    if (status == 2){
        //location is file;
        createBlob(location);
    }
    else if (status == 1){
        // location is directory
        blobDir(location_new);
    }
    else{
        updateIndexFile("", location); // why required
    }
}

void add_dot(){
    // char* dirname=;
    cout<< "running add_dot\n";
    DIR* dir = opendir(ROOT_PATH); // open the directory location to read.
    if (dir == NULL){
        // handling error
        fprintf(stderr, "opendir: '%s': %s\n", ROOT_PATH, strerror(errno));
        return;
    }

    struct dirent* dir_reader; // to read the directory
    
    while ((dir_reader = readdir(dir)) !=NULL){
        char newlocation[MAX_FILE_NAME_LENGTH] = "";
        cout<< dir_reader->d_name<<" d_name"<<endl;
        if ( (!strcmp(dir_reader->d_name, ".")) || (!strcmp(dir_reader->d_name, "..")) || (!strcmp(dir_reader->d_name, "git")) ){
            // ignore "." and ".." directories
            cout<< "ignoring git folder\n";
        }
        else{
            strncat(newlocation, dir_reader->d_name, strlen(dir_reader->d_name));
            // query to blob the directory file/ subdirectory
            string newLocation(newlocation);
            
            add_run(newLocation);
        }
    }
    closedir(dir);
}

void add(int argc,char* argv[]){
    // char * dot= ".";
    if(argc==2){
        printf("No arguments given\n");
        exit(1);
    }
    else if(strcmp(ROOT_PATH, argv[2])==0){
        // dot_flag=1;
        // remove_file(PATH_INDEX);
        ofstream f_create;
        f_create.open(PATH_INDEX, ofstream::trunc);
        f_create << "";
        f_create.close();
        add_dot();
    }
    else{
        string file_name=argv[2];
        add_run(file_name);
    }
}