// gcc -o gpt-parse main.c

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <arpa/inet.h>


const int SECTOR_SIZE = 512;
const int MAX_GPT_PARTITION = 128;
const int GPT_PARTITION_ENTRY_OFFSET = SECTOR_SIZE * 2;
const int GPT_PARTITION_ENTRY_SIZE = 128;
const int GPT_PARTITION_ENTRY_TYPE_GUID_SIZE = 16;
const int GPT_PARTITION_ENTRY_LBA_SIZE = 8;


struct GPT_PARTITION_ENTRY{
    uint8_t PARTITION_TYPE_GUID[16];
    uint8_t UNIQUE_PARITTION_GUID[16];
    uint64_t FIRST_LBA;
    uint64_t LAST_LBA;
    uint8_t ATTRIBUTE_FLAGS[8];
    uint8_t PARTITION_NAME[36];
};


uint8_t* readFile(const char* filePath_, size_t* sz_){
    FILE* fp = fopen(filePath_, "rb");
    if (fp == NULL){
        printf("@fopen error\n");
        return NULL;
    }

    if (fseek(fp, 0L, SEEK_END) != 0){
        printf("@fseek error\n");
        fclose(fp);
        return NULL;
    }

    size_t sz = ftell(fp);
    if (sz == -1){
        printf("@ftell error\n");
        fclose(fp);
        return NULL;
    }
    *sz_ = sz;
    rewind(fp);

    uint8_t* buff = (uint8_t*)malloc(sizeof(uint8_t) * sz);
    if (buff == NULL){
        printf("@malloc error\n");
        fclose(fp);
        return NULL;
    }

    size_t rsz = fread(buff, sizeof(uint8_t), sz, fp);
    if (rsz != sz){
        printf("@fread error\n");
        free(buff);
        buff = NULL;
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return buff;
}


void free2DArray(uint8_t** ptr, size_t sz){
    for(int _ = 0; _ < sz; _++){
        free(ptr[_]);
        ptr[_] = NULL;
    }
    free(ptr);
    ptr = NULL;
}


int main(int argc, char* argv[]){
    if (argc != 2){
        printf("usage: %s [IMAGE_FILE]\n", argv[0]);
        return 1;
    }

    size_t readSize = 0;
    uint8_t* buff = readFile(argv[1], &readSize);
    if (buff == NULL){
        printf("@readFile error\n");
        return -1;
    }

    uint8_t** partition_entries = (uint8_t**)malloc(sizeof(uint8_t**) * MAX_GPT_PARTITION);
    if (partition_entries == NULL){
        printf("@malloc partition_entries error\n");
        return 1;
    }
    
    int GPT_PARTITION_NUM = 0;
    uint8_t* NO_GPT_PARTITION = (uint8_t*)calloc(GPT_PARTITION_ENTRY_TYPE_GUID_SIZE, sizeof(uint8_t));
    for (int _ = 0; _ < MAX_GPT_PARTITION; _++){
        partition_entries[_] = (uint8_t*)malloc(GPT_PARTITION_ENTRY_SIZE);
        if (partition_entries[_] == NULL){
            printf("@malloc partition_entries[] error\n");
            free2DArray(partition_entries, _);
            free(NO_GPT_PARTITION);
            return 1;
        }

        size_t CURRENT_OFFSET = GPT_PARTITION_ENTRY_OFFSET + (_ * GPT_PARTITION_ENTRY_SIZE);
        struct GPT_PARTITION_ENTRY* CURRENT_GPT_PARTITION_ENTRY = (struct GPT_PARTITION_ENTRY*)malloc(GPT_PARTITION_ENTRY_SIZE);
        if (CURRENT_GPT_PARTITION_ENTRY == NULL){
            printf("@malloc CURRENT_GPT_PARTITION_ENTRY error\n");
            free2DArray(partition_entries, _);
            free(NO_GPT_PARTITION);
            return 1;
        }

        if (memcmp(&(buff[CURRENT_OFFSET]), NO_GPT_PARTITION, GPT_PARTITION_ENTRY_TYPE_GUID_SIZE) == 0){
            free(CURRENT_GPT_PARTITION_ENTRY);
            continue;
        }

        memcpy(CURRENT_GPT_PARTITION_ENTRY, &(buff[CURRENT_OFFSET]), GPT_PARTITION_ENTRY_SIZE);
        if (CURRENT_GPT_PARTITION_ENTRY == NULL){
            printf("@memcpy error\n");
            free2DArray(partition_entries, _);
            free(CURRENT_GPT_PARTITION_ENTRY);
            free(NO_GPT_PARTITION);
            return 1;
        }
        
        for(int __ = 0; __ < GPT_PARTITION_ENTRY_TYPE_GUID_SIZE; __++){
            printf("%02X", CURRENT_GPT_PARTITION_ENTRY->PARTITION_TYPE_GUID[__]);
        }
        
        uint64_t FIRST_LBA = CURRENT_GPT_PARTITION_ENTRY->FIRST_LBA;
        uint64_t LAST_LBA = CURRENT_GPT_PARTITION_ENTRY->LAST_LBA;
        uint64_t LBA_REAL_ADDR = FIRST_LBA * SECTOR_SIZE;
        uint64_t LBA_GAP = (LAST_LBA - FIRST_LBA + 1) * SECTOR_SIZE; // size


        printf(" %zu", LBA_REAL_ADDR);
        printf(" %zu\n", LBA_GAP);
        free(CURRENT_GPT_PARTITION_ENTRY);

        if (GPT_PARTITION_ENTRY_OFFSET + (_ + 1) * GPT_PARTITION_ENTRY_SIZE >= readSize){
            GPT_PARTITION_NUM = _ + 1;
            break;
        }
    }

    free2DArray(partition_entries, GPT_PARTITION_NUM);
    free(NO_GPT_PARTITION);
    free(buff);

    return 0;
}