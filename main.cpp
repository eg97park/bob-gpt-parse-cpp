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
    uint8_t FIRST_LBA[8];
    uint8_t LAST_LBA[8];
    uint8_t ATTRIBUTE_FLAGS[8];
    uint8_t PARTITION_NAME[36];
};

uint64_t htonll(uint32_t _){
    return (((uint64_t)htonl(_)) << 32) + htonl(_ >> 32);
}

int main(int argc, char* argv[]){
    if (argc != 2){
        printf("usage: %s [IMAGE_FILE]\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL){
        printf("@fopen error\n");
        return 1;
    }

    if (fseek(fp, 0L, SEEK_END) != 0){
        printf("@fseek error\n");
        return 1;
    }

    size_t sz = ftell(fp);
    if (sz == -1){
        printf("@ftell error\n");
        return 1;
    }
    rewind(fp);

    uint8_t* buff = (uint8_t*)malloc(sizeof(uint8_t) * sz);
    if (buff == NULL){
        printf("@malloc error\n");
        return 1;
    }

    size_t rsz = fread(buff, sizeof(uint8_t), sz, fp);
    if (rsz != sz){
        printf("@fread error\n");
        return 1;
    }

    uint8_t** partition_entries = (uint8_t**)malloc(sizeof(uint8_t**) * MAX_GPT_PARTITION);
    if (partition_entries == NULL){
        printf("@malloc error\n");
        return 1;
    }

    for (int _ = 0; _ < MAX_GPT_PARTITION; _++){
        partition_entries[_] = (uint8_t*)malloc(GPT_PARTITION_ENTRY_SIZE);
        if (partition_entries[_] == NULL){
            printf("@malloc error\n");
            return 1;
        }

        size_t CURRENT_OFFSET = GPT_PARTITION_ENTRY_OFFSET + (_ * GPT_PARTITION_ENTRY_SIZE);
        struct GPT_PARTITION_ENTRY* CURRENT_GPT_PARTITION_ENTRY = (struct GPT_PARTITION_ENTRY*)malloc(GPT_PARTITION_ENTRY_SIZE);
        if (CURRENT_GPT_PARTITION_ENTRY == NULL){
            printf("@malloc error\n");
            return 1;
        }

        memcpy(CURRENT_GPT_PARTITION_ENTRY, &(buff[CURRENT_OFFSET]), GPT_PARTITION_ENTRY_SIZE);
        if (CURRENT_GPT_PARTITION_ENTRY == NULL){
            printf("@memcpy error\n");
            return 1;
        }

        printf("Partition Type GUID: ");
        for(int __ = 0; __ < GPT_PARTITION_ENTRY_TYPE_GUID_SIZE; __++){
            printf("%02x", CURRENT_GPT_PARTITION_ENTRY->PARTITION_TYPE_GUID[__]);
        }
        printf("\n");

        
        printf("First LBA: ");
        for(int __ = 0; __ < GPT_PARTITION_ENTRY_LBA_SIZE; __++){
            printf("%02x", CURRENT_GPT_PARTITION_ENTRY->FIRST_LBA[__]);
        }
        printf("\n");

        printf("Last  LBA: ");
        for(int __ = 0; __ < GPT_PARTITION_ENTRY_LBA_SIZE; __++){
            printf("%02x", CURRENT_GPT_PARTITION_ENTRY->LAST_LBA[__]);
        }
        printf("\n");
    }


    for (int _ = 0; _ < MAX_GPT_PARTITION; _++){
        free(partition_entries[_]);
        partition_entries[_] = NULL;
    }
    free(partition_entries);
    partition_entries = NULL;    

    return 0;
}