#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATASTREAM_STARTSIZE    20

typedef struct DataStream {
    int size;
    int SIZE;
    void* Memory;
} DataStream;

DataStream DataStream_New() {
    DataStream v;
    v.size = 0;
    v.SIZE = DATASTREAM_STARTSIZE;
    v.Memory = malloc(v.SIZE);
    return v;
}
DataStream DataStream_Make(size_t SIZE) {
    DataStream v;
    v.size = 0;
    v.SIZE = SIZE;
    v.Memory = malloc(v.SIZE);
    return v;
}
DataStream DataStream_Cpy(DataStream* v){
    DataStream out;
    out.size = v->size;
    out.SIZE = v->SIZE;
    out.Memory = malloc(v->SIZE);
    memcpy(out.Memory,v->Memory,v->size);
    return out;
}
DataStream DataStream_Null(){
    DataStream out;
    out.size = -1;
    out.SIZE = -1;
    out.Memory = NULL;
    return out;
}
int DataStream_Size(DataStream* v){
    return v->size;
}
void DataStream_Expand(DataStream* v) {
    if (v->size >= v->SIZE) {
        int NewSize = v->SIZE * 2;
        char* NewMemory = (char*)malloc(NewSize);
        memcpy(NewMemory,v->Memory,v->size);
        if (v->Memory) free(v->Memory);
        v->Memory = NewMemory;
        v->SIZE = NewSize;
    }
}
void DataStream_Compress(DataStream* v) {
    if (v->size <= (v->SIZE / 2)) {
        int NewSize = v->SIZE / 2;
        NewSize = NewSize<DATASTREAM_STARTSIZE ? DATASTREAM_STARTSIZE:NewSize;
        char* NewMemory = (char*)malloc(NewSize);
        memcpy(NewMemory,v->Memory,NewSize);
        if (v->Memory) free(v->Memory);
        v->Memory = NewMemory;
        v->SIZE = NewSize;
    }
}
void DataStream_ExpandTo(DataStream* v,size_t ExpandSize) {
    if (ExpandSize >= v->size){
        int NewSize = ExpandSize;
        char* NewMemory = (char*)malloc(NewSize);
        memcpy(NewMemory, v->Memory, v->size);
        if (v->Memory) free(v->Memory);
        v->Memory = NewMemory;
        v->SIZE = NewSize;
    }
    else{
        printf("Couln't Expand DataStream to: %llu\n",(long long unsigned int)ExpandSize);
    }
}
void DataStream_ExpandBy(DataStream* v,size_t ExpandSize) {
    v->SIZE += ExpandSize;
    char* NewMemory = (char*)malloc(v->SIZE);
    memcpy(NewMemory,v->Memory,v->size);
    if (v->Memory) free(v->Memory);
    v->Memory = NewMemory;
}
void DataStream_Move(DataStream* v, unsigned int Index, int Count) {
    DataStream_ExpandBy(v,v->size + Count);
    if (Index >= 0 && Index < v->size) {
        void* Src = ((char*)v->Memory + Index);
        void* Dst = ((char*)v->Memory + (Index + Count));
        memmove(Dst,Src,(size_t)((v->size - Index)));
    }else {
        printf("[DataStream]: Move -> not able to move!\n");
    }
}
void DataStream_PushCount(DataStream* v, void* Items, int Count) {
    DataStream_ExpandBy(v,Count);
    if (v->size < v->SIZE){
        memcpy((char*)v->Memory + v->size,Items,Count);
        v->size += Count;
    }
    else printf("[DataStream]: PushCount -> Not able to!\n");
}
void DataStream_AddCount(DataStream* v, void* Items, int Count, unsigned int Index) {
    DataStream_ExpandBy(v,Count);
    if (v->size < v->SIZE){
        DataStream_Move(v,Index,Count);
        memcpy(v->Memory + Index, Items, Count);
        v->size += Count;
    }
    else printf("[DataStream]: AddCount -> Not able to!\n");
}
void DataStream_PopTopCount(DataStream* v,int Count) {
    if (v->size - Count >= 0) {
        v->size -= Count;
        DataStream_Compress(v);
    }else {
        printf("[DataStream]: Not able to PopTopCount\n");
    }
}
void DataStream_RemoveCount(DataStream* v, unsigned int Index,int Count) {
    if (Index >= 0 && Index < v->size-1) {
        DataStream_Move(v,Index,-Count);
        v->size -= Count;
    }else {
        printf("[DataStream]: RemoveC -> Not able to at Index: %d!\n", Index);
    }
}
void DataStream_Clear(DataStream* v) {
    if(v->size==0) return;
    if(v->SIZE<=10){
        v->size = 0;
        return;
    }
    if(v->Memory) free(v->Memory);
    v->size = 0;
    v->Memory = malloc(DATASTREAM_STARTSIZE);
    v->SIZE = DATASTREAM_STARTSIZE;
}
void DataStream_Free(DataStream* v) {
    if (v->Memory) free(v->Memory);
    v->Memory = NULL;
    v->size = 0U;
}
void DataStream_Print(DataStream* v) {
    printf("--- DataStream ---");
    printf("SIZE: %d\n", (int)v->SIZE);
    printf("Size: %d\n", (int)v->size);
    printf("--------------");
}
#define DATASTREAM_END  DataStream_Null()

#endif