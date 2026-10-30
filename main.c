#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"

void print_tag(const char* frame, const char* data, int size) {
    if (size <= 0) return;
    char *buf = malloc(size+1);
    memcpy(buf, data, size);
    buf[size] = '\0';
    char *text = buf;
    if ((unsigned char)buf[0] == 0 || (unsigned char)buf[0] == 3) text = buf+1;
    printf("%s : %s\n", frame, text);
    free(buf);
}

int sync_to_int(unsigned char *b) {
    return (b[0]<<21)|(b[1]<<14)|(b[2]<<7)|b[3];
}
void int_to_sync(int sz, unsigned char *b) {
    b[0] = (sz>>21)&0x7F; b[1] = (sz>>14)&0x7F; b[2] = (sz>>7)&0x7F; b[3] = sz&0x7F;
}

int read_tags(FILE *fp) {
    unsigned char header[10];
    if (fread(header,1,10,fp)!=10) { printf("Not an MP3 with ID3v2\n"); return 0; }
    if (strncmp((char*)header,"ID3",3)!=0) { printf("No ID3v2 tag\n"); return 0; }
    int tag_size = sync_to_int(header+6);
    printf("ID3v2 Version: 2.%d.%d Tag Size: %d\n", header[3], header[4], tag_size);
    printf("---------------------------------\n");
    int read = 0;
    while (read < tag_size) {
        unsigned char frame_h[10];
        if (fread(frame_h,1,10,fp)!=10) break;
        if (frame_h[0]==0) break;
        char frame_id[5]; memcpy(frame_id, frame_h,4); frame_id[4]='\0';
        int frame_size = (frame_h[4]<<24)|(frame_h[5]<<16)|(frame_h[6]<<8)|frame_h[7];
        if (frame_size<=0 || frame_size>10000) break;
        unsigned char *data = malloc(frame_size);
        fread(data,1,frame_size,fp);
        if (strcmp(frame_id,"TIT2")==0) print_tag("Title (TIT2) ", (char*)data, frame_size);
        else if (strcmp(frame_id,"TPE1")==0) print_tag("Artist (TPE1)", (char*)data, frame_size);
        else if (strcmp(frame_id,"TALB")==0) print_tag("Album (TALB) ", (char*)data, frame_size);
        else if (strcmp(frame_id,"TYER")==0 || strcmp(frame_id,"TDRC")==0) print_tag("Year (TYER/TDRC)", (char*)data, frame_size);
        else if (strcmp(frame_id,"TCON")==0) print_tag("Genre (TCON) ", (char*)data, frame_size);
        else if (strcmp(frame_id,"COMM")==0) print_tag("Comment (COMM)", (char*)data, frame_size);
        free(data);
        read += 10+frame_size;
    }
    return 1;
}

int edit_tag(const char *src, const char *dst, const char *frame_to_edit, const char *new_value) {
    FILE *fp = fopen(src,"rb");
    if(!fp){ perror("src open"); return 0; }
    FILE *out = fopen(dst,"wb");
    if(!out){ perror("dst open"); fclose(fp); return 0; }
    unsigned char header[10];
    fread(header,1,10,fp);
    if(strncmp((char*)header,"ID3",3)!=0){ printf("No ID3 tag to edit\n"); fclose(fp); fclose(out); return 0; }
    int tag_size = sync_to_int(header+6);
    unsigned char *tag_data = malloc(tag_size);
    fread(tag_data,1,tag_size,fp);
    fseek(fp,0,SEEK_END);
    long total = ftell(fp);
    long audio_size = total - 10 - tag_size;
    fseek(fp,10+tag_size,SEEK_SET);
    unsigned char *audio = malloc(audio_size);
    fread(audio,1,audio_size,fp);
    fclose(fp);
    unsigned char new_frame_data[256];
    new_frame_data[0]=3;
    strcpy((char*)new_frame_data+1, new_value);
    int new_frame_size = strlen(new_value)+1;
    unsigned char *new_tag = malloc(tag_size + 1000);
    int pos=0, i=0;
    int replaced=0;
    while(i < tag_size){
        if(i+10 > tag_size) break;
        char fid[5]; memcpy(fid, tag_data+i,4); fid[4]=0;
        int fsize = (tag_data[i+4]<<24)|(tag_data[i+5]<<16)|(tag_data[i+6]<<8)|tag_data[i+7];
        if(fsize<=0 || fsize>10000 || tag_data[i]==0) break;
        if(strcmp(fid, frame_to_edit)==0 &&!replaced){
            memcpy(new_tag+pos, fid,4); pos+=4;
            new_tag[pos++] = (new_frame_size>>24)&0xFF;
            new_tag[pos++] = (new_frame_size>>16)&0xFF;
            new_tag[pos++] = (new_frame_size>>8)&0xFF;
            new_tag[pos++] = new_frame_size&0xFF;
            new_tag[pos++]=0; new_tag[pos++]=0;
            memcpy(new_tag+pos, new_frame_data, new_frame_size); pos+=new_frame_size;
            replaced=1;
        } else {
            memcpy(new_tag+pos, tag_data+i, 10+fsize);
            pos+=10+fsize;
        }
        i+=10+fsize;
    }
    if(!replaced){
        memcpy(new_tag+pos, frame_to_edit,4); pos+=4;
        new_tag[pos++] = (new_frame_size>>24)&0xFF;
        new_tag[pos++] = (new_frame_size>>16)&0xFF;
        new_tag[pos++] = (new_frame_size>>8)&0xFF;
        new_tag[pos++] = new_frame_size&0xFF;
        new_tag[pos++]=0; new_tag[pos++]=0;
        memcpy(new_tag+pos, new_frame_data, new_frame_size); pos+=new_frame_size;
    }
    int new_tag_size = pos;
    unsigned char new_header[10];
    memcpy(new_header, header,10);
    int_to_sync(new_tag_size, new_header+6);
    fwrite(new_header,1,10,out);
    fwrite(new_tag,1,new_tag_size,out);
    fwrite(audio,1,audio_size,out);
    printf("Edited %s to '%s' -> %s created\n", frame_to_edit, new_value, dst);
    free(tag_data); free(audio); free(new_tag);
    fclose(out);
    return 1;
}

void usage() {
    printf("Usage:\n");
    printf("./mp3tag -v <file.mp3> : view tags\n");
    printf("./mp3tag -e -t \"Title\" <file.mp3> : edit title\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) { usage(); return 0; }
    if (strcmp(argv[1],"-v")==0) {
        FILE *fp = fopen(argv[2],"rb");
        if(!fp){ perror("file"); return 1; }
        read_tags(fp);
        fclose(fp);
    }
    else if (strcmp(argv[1],"-e")==0) {
        if (argc < 5) { usage(); return 0; }
        const char *opt = argv[2];
        const char *value = argv[3];
        const char *filename = argv[4];
        const char *frame = NULL;
        if (strcmp(opt,"-t")==0) frame="TIT2";
        else if (strcmp(opt,"-a")==0) frame="TPE1";
        else if (strcmp(opt,"-A")==0) frame="TALB";
        else if (strcmp(opt,"-y")==0) frame="TYER";
        else if (strcmp(opt,"-m")==0) frame="TCON";
        else if (strcmp(opt,"-c")==0) frame="COMM";
        else { usage(); return 0; }
        edit_tag(filename, "dup_sample.mp3", frame, value);
    } else {
        usage();
    }
    return 0;
}