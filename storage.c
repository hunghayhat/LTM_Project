#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "storage.h"
#include "server.h"

//Con trỏ void* trỏ đến bất kỳ dữ liệu nào muốn lưu trữ trong nút mới.
Node createNode(void* value) {
    Node node = (Node) malloc(sizeof(node));
    node->value = value;
    node->next = NULL;
    return node;
}
List newList() {
    List l;
    l.head = NULL;
    l.count = 0;
    return l;
}

Room* getRoomByName(char *name) {
    char rname[100];

    if (name == NULL)   return NULL;
    FILE *f = fopen(ROOM_FILE, "r");
    if (f == NULL)  return NULL;

    Room *r = (Room*) malloc(sizeof(Room));
    while(fscanf(f, "%[^\n] ", rname) != EOF){
        if (strcmp(rname, name) == 0) {
            strcpy(r->name, rname);
            fclose(f);
            return r;
        }
    }
    fclose(f);
    return NULL;
}

int saveRoom(char *name) {
    char rname[100];
    Room *r;
    FILE *f;    

    if (name == NULL)  return 0;
    f = fopen(ROOM_FILE, "a");

    if (f == NULL)  return 0;
    if ((r = getRoomByName(name)) != NULL) return 0;

    fprintf(f, "%s\n", name);
    fclose(f);
    return 1;
}

void addEnd(List* l, void* value) {

    Node temp = l->head;
    Node node = createNode(value);
    if (l->head == NULL) {
        l->head = node;
        l->count++;
        return;
    }
    
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = node;
    l->count++;
}

List getAllRooms(char *f_room) {
    char rname[100];
    int status;

    FILE *f = fopen(f_room, "r");
    List l = newList();
    if (f == NULL)  return l;

    Room *r;
    // f, "%s %d", rname, &status)
    // fscanf(f, "%[^\n] ", rname
    while(fscanf(f, "%s %d", rname, &status) != EOF){
        r = (Room*) malloc(sizeof(Room));
        // printf("rname: %s..\n", rname);
        // printf("status: %d..\n", status);
        strcpy(r->name, rname);
        r->status = status;
        addEnd(&l, r);
    }
    fclose(f);
    return l;
}

List getAllAccount(char *acc_file) {
    char username[100];
    char password[100];

    FILE *f = fopen(acc_file, "r");
    List l = newList();
    if (f == NULL)  return l;

    Account *acc;
    while(fscanf(f, "%s %[^\n] ", username, password) != EOF){
        acc = (Account*) malloc(sizeof(Account));
        // printf("user: %s.. pass: %s..\n", username, password);
        strcpy(acc->username, username);
        strcpy(acc->password, password);
        addEnd(&l, acc);
    }
    fclose(f);
    return l;
}

List getAllQuestion(char *ques_file) {
    char line[500] = "";

    FILE *f = fopen(ques_file, "r");
    List l = newList();
    if (f == NULL)  return l;

    Question *ques;
    while(fgets(line, 500, f) != NULL){
        ques = (Question*) malloc(sizeof(Question));
        // printf("ques: %s..\n", line);
        ques->id = atoi(strtok(line, "|"));
        ques->level = atoi(strtok(NULL, "|"));
        strcpy(ques->ques, strtok(NULL, "|"));
        strcpy(ques->choice[0], strtok(NULL, "|"));
        strcpy(ques->choice[1], strtok(NULL, "|"));
        strcpy(ques->choice[2], strtok(NULL, "|"));
        strcpy(ques->choice[3], strtok(NULL, "|"));
        strcpy(ques->answer, strtok(NULL, "|"));
        addEnd(&l, ques);
    }
    fclose(f);
    return l;
}

List getAllRoomPoint(char *file) {
    char stud_name[100] = "";
    char point[20] = "";

    FILE *f = fopen(file, "r");
    List l = newList();
    if (f == NULL)  return l;

    RoomPoint *rp;
    while(fscanf(f, "%s %s\n", stud_name, point) != EOF){
        rp = (RoomPoint*) malloc(sizeof(RoomPoint));
        // printf("name: %s.. point: %d..\n", stud_name, point);
        strcpy(rp->stud_name, stud_name);
        strcpy(rp->point, point);
        addEnd(&l, rp);
    }
    fclose(f);
    return l;
}

int compareRoomByName(void* room, void* name) {
    Room* r = (Room*)room;
    char* n = (char*)name;
    if (strcmp(r->name, n) == 0) {
        return 1;
    } return 0;
}

int compareAccountByUserName(void* account, void* username) {
    Account* acc = (Account*)account;
    char* n = (char*)username;
    if (strcmp(acc->username, n) == 0) {
        return 1;
    } return 0;
}

int compareRoomPoint(void* rp, void* username) {
    RoomPoint* x = (RoomPoint*)rp;
    char* n = (char*)username;
    if (strcmp(x->stud_name, n) == 0) {
        return 1;
    } return 0;
}

int compareQuestionByLevel(void* question, void* level) {
    Question* ques = (Question*)question;
    int lv = atoi((char*)level);
    return ques->level == lv;
}

void printRoom(void* room) {
    Room* r = (Room*)room;
    printf("room: %s.\n", r->name);
}

void printAccount(void* account) {
    Account* acc = (Account*)account;
    printf("acc: %s. - %s.\n", acc->username, acc->password);
}

void printQuestion(void* quesiton) {
    Question *q = (Question*) quesiton;
    printf("id: %d.. level: %d..\n", q->id, q->level);
    printf("question: %s..\n", q->ques);
    for(int i=0; i<4; i++) {
        printf("choice %d: %s..\n", i+1, q->choice[i]);
    }
    printf("answer: %s..\n\n", q->answer);
}


void printList(List l, void (*print)(void*)) {
    Node node = l.head;
    printf("\n");
    while (node != NULL) {
        print(node->value);
        node = node->next;
    }
}

void printRoomList(List l) {
    printList(l, printRoom);
}

void printQuesList(List l) {
    printList(l, printQuestion);
}

void printAccList(List l) {
    printList(l, printAccount);
}

Node search(List *l, void* key, int (*compare)(void*, void*)) {
    Node current = l->head;
    if (l->head == NULL) {
        return NULL;
    }
    while(current != NULL) {
        Room* r = (Room*)current->value;
        if (compare(current->value, key)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Room* searchRoomByName(List *l, char* name) {
    Node n = search(l, name, compareRoomByName);
    if (n) {
        return (Room*)n->value;
    }
    return NULL;
}

Account* searchAccountByUsername(List *l, char* name) {
    Node n = search(l, name, compareAccountByUserName);
    if (n) {
        return (Account*)n->value;
    }
    return NULL;
}

RoomPoint* searchRoomPoint(List *l, char* acc_name) {
    Node n = search(l, acc_name, compareRoomPoint);
    if (n) {
        return (RoomPoint*)n->value;
    }
    return NULL;
}

Node deleteA(List* l, void* key, int (*compare)(void*, void*)) {
    Node current = l->head;
    Node prev = NULL;
    if (l->head == NULL) {
        return NULL;
    }

    while (!compare(current->value, key)) {
        if (current->next == NULL) {
            return NULL;
        } else {
            prev =current;
            current = current->next;
        }
    }

    if (current == l->head) {
        l->head = l->head->next;
    } else {
        prev->next = current->next;
    }
    l->count--;
    return current;
}

Room* deleteRoomByName(List *l, char* name) {
    Node n = deleteA(l, name, compareRoomByName);
    if (n) {
        return (Room*)n->value;
    }
    return NULL;
}

int saveAllRoom(char* room_f, List l) {
    Room* r;
    Node node = l.head;
    FILE *f = fopen(room_f, "w");
    if (f == NULL) {
        return 0;
    }
    while (node != NULL) {
        r = (Room*)node->value;
        fprintf(f, "%s %d\n", r->name, r->status);
        node = node->next;
    }
    fclose(f);
    return 1;
}

int saveAllQuestions(char* ques_f, List l) {
    Question *q;
    Node node = l.head;
    FILE *f = fopen(ques_f, "w");
    if (f == NULL) {
        return 0;
    }
    while (node != NULL) {
        q = (Question*)node->value;
        fprintf(f, "%d|%d|%s|%s|%s|%s|%s|%s|\n", 
                    q->id, q->level,
                    q->ques,
                    q->choice[0], q->choice[1], q->choice[2], q->choice[3],
                    q->answer);
        node = node->next;
    }
    return 1;
}

int saveAllRoomPoint(char* result_f, List l) {
    RoomPoint* r;
    Node node = l.head;
    FILE *f = fopen(result_f, "w");
    if (f == NULL) {
        return 0;
    }
    while (node != NULL) {
        r = (RoomPoint*)node->value;
        fprintf(f, "%s\t%s\n", r->stud_name, r->point);
        // fprintf(stdout, "%s\t%s\n", r->stud_name, r->point);
        node = node->next;
    }
    fclose(f);
    return 1;
}


List getQuestionsByLevel(char *ques_file, int level) {
    char line[500] = "";
    int id = 0, lv = 0;
    FILE *f = fopen(ques_file, "r");
    List l = newList();
    if (f == NULL)  return l;

    Question *ques;
    while(fgets(line, 500, f) != NULL){
        id = atoi(strtok(line, "|"));
        lv = atoi(strtok(NULL, "|"));
        if (lv != level) {
            continue;
        }
        ques = (Question*) malloc(sizeof(Question));
        // printf("ques: %s..\n", line);
        ques->id = id;
        ques->level = lv;
        strcpy(ques->ques, strtok(NULL, "|"));
        strcpy(ques->choice[0], strtok(NULL, "|"));
        strcpy(ques->choice[1], strtok(NULL, "|"));
        strcpy(ques->choice[2], strtok(NULL, "|"));
        strcpy(ques->choice[3], strtok(NULL, "|"));
        strcpy(ques->answer, strtok(NULL, "|"));
        printf("\n-----ques anser: %s-------\n", ques->answer);
        addEnd(&l, ques);
    }
    fclose(f);
    return l;
}

Question* searchQuestionByPosition(List *l, int position) {
    Node node = l->head;
    Question* q;
    if (l == NULL || l->count == 0) {
        return NULL;
    }
    if (position < 1 || position > l->count);
    for (int i = 0; i<l->count; i++) {
        q = (Question*)node->value;
        if ((i+1) == position) {
            return q;
        }
        node = node->next;
    }
    return NULL;
}

int randUniqueArr(int arr[], int arr_size, int min, int max) {
    int count = 0;
    if ((max-min+1) < arr_size) {
        return 0;
    }

    while (1) {
        int n = (rand()%max) - min + 2;
        int found = 0;
        for (int i = 0; i<count; i++) {
            if (n == arr[i]) {
                found = 1;
                break;
            }
        }

        if (found == 1) continue;
        arr[count++] = n;
        // printf("%d\n", n);
        if (count == arr_size) {
            break;
        }
    }
    return 1;
}

// chuyển đổi từ cấu trúc question sang string có định dạng cụ thể
char* quesToString(Question *q, char* s) {
    // char *s;
    // s = (char*)malloc(sizeof(char)*1000);
    if (q == NULL)  return NULL;
    
    // printf("id: %d\n", q->id);
    strcat(s, q->ques);
    strcat(s, "|");
    strcat(s, q->choice[0]);
    strcat(s, "|");
    strcat(s, q->choice[1]);
    strcat(s, "|");
    strcat(s, q->choice[2]);
    strcat(s, "|");
    strcat(s, q->choice[3]);
    strcat(s, "|");

    return s;
}

List makeQues(char *f) {
    char path[200] = "";
    strcpy(path, "question/");
    strcat(path, f);
    strcat(path, ".txt");
    List easyL = getQuestionsByLevel(path, 1);
    List mediumL = getQuestionsByLevel(path, 2);
    List hardL = getQuestionsByLevel(path, 3);

    srand(time(0));

    List quesL = newList();
    int ezArr[4], mediumArr[4], hardArr[2];
    randUniqueArr(ezArr, 4, 1, easyL.count);
    randUniqueArr(mediumArr, 4, 1, mediumL.count);
    randUniqueArr(hardArr, 2, 1, hardL.count);

    Question *q;
    for(int i = 0; i<4; i++) {
        q = searchQuestionByPosition(&easyL, ezArr[i]);
        if (q) {
            addEnd(&quesL, q);
        }
    }
    for(int i = 0; i<4; i++) {
        q = searchQuestionByPosition(&mediumL, mediumArr[i]);
        if (q) {
            addEnd(&quesL, q);
        }
    }
    for(int i = 0; i<2; i++) {
        q = searchQuestionByPosition(&hardL, hardArr[i]);
        if (q) {
            addEnd(&quesL, q);
        }
    }

    return quesL;
}


void freeNode(Node n) {
    if (n != NULL) {
        free(n->value);
        free(n);
    }
}

void freeList(List *l) {
    Node n;
    if (l != NULL) {
        n = l->head;
        while (n != NULL) {
            l->head = n->next;
            freeNode(n);
            n = l->head;
            l->count--;
        }
    }
}
