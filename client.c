#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "client.h"
#include "storage.h"

#define MAXLINE 4096 /*max text line length*/
#define MAX 100


int sockfd; // số nguyên dùng để lưu trữ socket kết nối đến máy chủ
char res[MAXLINE], req[MAXLINE];

int main(int argc, char **argv) 
{
    // int sockfd;
    struct sockaddr_in servaddr; // lưu trữ thông tin địa chỉ của máy chủ.
    char sendline[MAXLINE], recvline[MAXLINE]; // lưu trữ dữ liệu gửi và nhận trong quá trình giao tiếp.
        
    //basic check of the arguments
    //additional checks can be inserted
    if (argc !=3) {
        perror("Usage: Client <IP server> <PORT>"); 
        exit(1);
    }
        
    //Create a socket for the client
    //If sockfd<0 there was an error in the creation of the socket
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
    }
        
    //Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr= inet_addr(argv[1]);
    servaddr.sin_port =  htons(atoi(argv[2])); //convert to big-endian order
        
    //Connection of the client to the socket 
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
        perror("Problem in connecting to the server");
        exit(3);
    }
        
    while(1) {
        home();
    }
    exit(0);
}

void home() {
    int choice;
    printf("\n");
    printf("____________ HOME ____________\n");
    printf("                              \n");
    printf("   1. Login Student           \n");
    printf("   2. Login Teacher           \n");
    printf("   3. Sign Up                 \n");
    printf("   0. Exit                    \n");
    printf("______________________________\n\n");

    while(1) {
        printf("---> Your choice: ");
        scanf("%d", &choice);
        while(getchar() != '\n');
        
        if (choice > -1 && choice < 4)  break;
    }

    switch (choice) {
        case 1:
        case 2: 
            login(choice);
            break;
        case 3: 
            signup();
            break;
        case 0:
            printf("\n[Exit]\n");
            exit(0);
            break;
    }
}

/*
* xử lý đăng nhập vào hệ thống
* @param {int} mode: 1: students, 2: teacher
*/
void login(int mode) {
    char req[MAXLINE] = "";
    char buf[MAXLINE] = "";
    char username[MAX] = ""; 
    char password[MAX] = "";


    printf("\n____________Login____________\n\n");
    printf("--> username: ");
    scanf("%s", username);
    while(getchar() != '\n');

    printf("--> password: ");
    scanf("%[^\n]", password);
    while(getchar() != '\n');
    
    // lưu vào buf định dạng <usename> <password>
    strcat(buf, username);
    strcat(buf, " ");
    strcat(buf, password);

    
    switch (mode) {
    case 1:
        makeReq(req, "LOGIN_STUDENT", buf);
        break;
    
    case 2:
        makeReq(req, "LOGIN_TEACHER", buf);
        break;
    }
    sendReq(req);

    // char res[MAXLINE];
    // recvRes(res);
    handleRes(res, req);
}

/*
* xử lý đăng ký tài khoản
*/
void signup() {
    char req[MAXLINE] = "";
    char buf[MAXLINE] = "";
    char username[MAX] = ""; 
    char password[MAX] = "";
    char confirmPassword[MAX] = "";
    char roleName[MAX] = "";

    printf("\n____________Sign up____________\n\n");
    printf("--> username: ");
    scanf("%s", username);
    while(getchar() != '\n');

    printf("--> password: ");
    scanf("%[^\n]", password);
    while(getchar() != '\n');

    printf("--> confirm password: ");
    scanf("%[^\n]", confirmPassword);
    while(getchar() != '\n');

    do
    {
        printf("--> role: ");
        scanf("%[^\n]", roleName);
        while(getchar() != '\n');
    } while (strcmp(roleName, "teacher") != 0 && strcmp(roleName, "student") != 0);
    

    // lưu vào buf định dạng <usename> <password> <confirmPW> <roleName>
    strcat(buf, username);
    strcat(buf, " ");
    strcat(buf, password);
    strcat(buf," ");
    strcat(buf, confirmPassword);
    strcat(buf, " ");
    strcat(buf, roleName);

    int role = strcmp(roleName, "student") == 0 ? 1 : 2;

    switch (role) {
    case 1:
        makeReq(req, "SIGNUP_STUDENT", buf);
        break;
    case 2:
        makeReq(req, "SIGNUP_TEACHER", buf);
        break;
    }
    sendReq(req);

    // char res[MAXLINE];
    // recvRes(res);
    handleRes(res, req);
}

/*
* xử lý thực hiện bài thi
*/
void startTest() {
    char buf[MAXLINE];
    int choice;
    printf("\n________________________________\n");
    printf("\n____Do you want start test?_____\n");
    printf("     1. Yes\n");
    printf("     2. No\n\n");

    while(1) {
        printf("--> Your choice: ");
        scanf("%d", &choice);
        while(getchar() != '\n');

        if (choice > 0 && choice < 3)   break;
    }

    switch (choice) {
        case 1:
            makeReq(req, "START_TEST", "");
            sendReq(req);
            break;
        case 2:
            return;
            break;
    }
    handleRes(res, req);
}

/*
* xử lý phản hồi từ server đối với 1 yêu cầu cụ thể của client
* @param {char *} res: phản hồi từ server
* @param {char *} req: yêu cầu của client
*/
void handleRes(char *res, char *req) {
    char op[MAX], message[MAX];

    recvRes(res); // nhận phản hồi từ server
    printf("Phan hoi tu server %s\n", res);
    parseRes(res, op, message); // lấy ra mã thao tác và message
    if(strcmp(op, "SIGNUP_OK") == 0) {
        printf("\n[%s]\n", op);
    } else if(strcmp(op, "PASSWORDS_NOT_MATCH") == 0) {
        printf("\n[%s]\n", op);
    } else if(strcmp(op, "USERNAME_ALREADY_EXISTS") == 0) {
        printf("\n[%s]\n", op);
    } else if (strcmp(op, "LOGIN_STUDENT_OK") == 0) {
        printf("\n[%s]\n", op);
        joinRoom(message);
    } else if (strcmp(op, "LOGIN_STUDENT_NOT_OK") == 0) {
        printf("\n[%s]\n", op);

    } else if (strcmp(op, "LOGIN_TEACHER_OK") == 0) {
        printf("\n[%s]\n", op);
        teacherMenu();

    } else if (strcmp(op, "LOGIN_TEACHER_NOT_OK") == 0) {
        printf("\n[%s]\n", op);

    } else if (strcmp(op, "JOIN_ROOM_OK") == 0) {
        printf("\n[%s]\n", op);
        startTest();
    } else if (strcmp(op, "JOIN_ROOM_NOT_OK") == 0) {
        printf("\n[%s]\n", op); 
    } 
    else if(strcmp(op, "ROOM_CLOSE") == 0) {
        printf("\n[%s]\n", op);
    } 
    else if (strcmp(op, "CREATE_ROOM_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "CREATE_ROOM_NOT_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "DELETE_ROOM_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "DELETE_ROOM_NOT_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "SHOW_ROOM_OK") == 0) {
        printf("\n_________ ROOMS _________\n");
        printf("%s\n", message);
    } else if (strcmp(op, "SHOW_ROOM_NOT_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "SHOW_POINT_OK") == 0) {
        // printf("\n[%s]\n", op);
        printf("\n________Resutl______\n");
        printf("%s\n", message);
        
    } else if (strcmp(op, "SHOW_POINT_NOT_OK") == 0) {
        // printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "START_TEST_OK") == 0) {
        printf("\n[%s]\n", op);
        answer(message);
    } else if (strcmp(op, "START_TEST_NOT_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "ANSWER_OK") == 0) {
        printf("\n[Score: %s]\n", message);
    } else if (strcmp(op, "ANSWER_NOT_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "ADD_QUESTION_OK") == 0) {
        printf("\n[%s]\n", op);
        
    } else if (strcmp(op, "ADD_QUESTION_NOT_OK") == 0) {
        printf("\n[%s]\n", op);
    } else if(strcmp(op, "UPDATE_ROOM_OK") == 0) {
        printf("\n[%s]\n", op);
    }
}

/*
* xử lý tham gia phòng thi của user
* * @param {char *} list_room: danh sách phòng học
*/
void joinRoom(char* list_room) {
    char room_name[MAX];
    printf("\n________List room________\n");
    printf("%s\n\n", list_room);

    printf("--> Type room name: ");
    scanf("%[^\n]", room_name);
    while(getchar() != '\n');

    // printf("phong: %s", room_name);
    makeReq(req, "JOIN_ROOM", room_name);
    // printf("req gui di %s", req);
    sendReq(req);

    handleRes(res, req);
}

/*
* xử lý trả lời câu hỏi cho bài thi
* @param {char *} x: câu hỏi
*/
void answer(char* x) {
    int i = 0;
    int c = 0;
    int control;
    int editQuesNumber;
    char ans[MAX];
    char question[MAXLINE], choice[4][MAX];
    int userAnswer[10]; // Biến lưu trữ câu trả lời của người dùng
    Question listQuestion[100];
    memset(listQuestion, 0, sizeof(listQuestion));
    do {
        if (i == 0) {
            // parseQuestion(x, question, choice);
            parseQuestion(x, listQuestion[i].ques, listQuestion[i].choice);
            // strcpy(listQuestion[i].ques, question);
        } else {
            recvRes(res);
            // parseQuestion(res, question, choice);
            // strcpy(listQuestion[i].ques, question);
            parseQuestion(res, listQuestion[i].ques, listQuestion[i].choice);
        }

        // printf("\nCau %d: %s\n", i+1, question);
        // printf("1. %s\n", choice[0]);
        // printf("2. %s\n", choice[1]);
        // printf("3. %s\n", choice[2]);
        // printf("4. %s\n", choice[3]);

        printf("\nCau %d: %s\n", i+1, listQuestion[i].ques);
        printf("1. %s\n", listQuestion[i].choice[0]);
        printf("2. %s\n", listQuestion[i].choice[1]);
        printf("3. %s\n", listQuestion[i].choice[2]);
        printf("4. %s\n", listQuestion[i].choice[3]);
        do {
            printf("--> Your answer: ");
            scanf("%d", &c);
            while(getchar() != '\n');
        } while(c < 1 || c > 4);
        userAnswer[i] = c;
        sprintf(ans, "%d", c);
        makeReq(req, "", ans);
        sendReq(req);
        i++;
    } while (i < 10);
    while(i == 10) {
        printf("\n");
        printf("_____Submit_____\n");
        printf("1. Yes\n");
        printf("2. No, edit answer\n");
        do
        {
            printf("--> Your choice: ");
            scanf("%d", &control);
            while(getchar() != '\n');
        } while (control < 1 || control > 2);
        switch (control)
        {
        case 1:
            i++;
            makeReq(req, "ANSWER", "");
            sendReq(req);
            handleRes(res, req);
            break;
        case 2: 
            do {
                printf("--> Edit question: ");
                scanf("%d", &editQuesNumber);
                while(getchar() != '\n');
            } while(editQuesNumber < 1 || editQuesNumber > 10);

            printf("\nCau %d: %s\n", editQuesNumber, listQuestion[editQuesNumber - 1].ques);
            printf("1. %s\n", listQuestion[editQuesNumber -1].choice[0]);
            printf("2. %s\n", listQuestion[editQuesNumber -1].choice[1]);
            printf("3. %s\n", listQuestion[editQuesNumber -1].choice[2]);
            printf("4. %s\n", listQuestion[editQuesNumber -1].choice[3]);

            do {
                printf("--> Your answer: ");
                scanf("%d", &c);
                while(getchar() != '\n');
            } while(c < 1 || c > 4);
            userAnswer[editQuesNumber - 1] = c;
            sprintf(ans, "%d %d",editQuesNumber, c);
            // printf("gui di mode edit %s\n", ans);
            makeReq(req, "EDIT", ans);
            sendReq(req);
        }
    }
    
}

/*
* phân tích chuỗi câu hỏi thành các phần riêng biệt: câu hỏi và các đáp án
* @param {char *} q: thông tin các câu hỏi và các lựa chọn. Được phân tách bằng ký tự |
* @param {char *} ques: biến lưu câu hỏi
* @param {char [][]} choice: chứa các lựa chọn của câu hỏi
*/
void parseQuestion(char *q, char *ques, char choice[][100]) {
    if (q == NULL || ques ==  NULL || choice == NULL) return;
    strcpy(ques, strtok(q, "|")); // lấy nội dung câu hỏi lưu vào ques
    strcpy(choice[0], strtok(NULL, "|")); // lấy các lựa chọn
    strcpy(choice[1], strtok(NULL, "|"));
    strcpy(choice[2], strtok(NULL, "|"));
    strcpy(choice[3], strtok(NULL, "|"));
}

/*
* phân tích chuỗi phản hồi từ server thành các phần cụ thể
* @param {char *} res: phản hồi từ server
* @param {char *} op: mã thao tác
* @param {char *} message: thông điệp đi kèm với mã thao tác
*/
void parseRes(char *res, char *op, char *message) {
    char *next_s;
    strcpy(op, strtok(res, " ")); // lưu lại mã thao tác từ res
    if ((next_s = strtok(NULL, ""))) {
        strcpy(message, next_s); // lưu lại message
    }
}

/*
* tạo chuỗi yêu cầu để gửi đến server
* @param {char*} req: yêu cầu từ client
* @param {char*} op: mã thao tác
* @param {char*} message: thông điệp đi kèm với mã thao tác
*/
void makeReq(char *req, char *op, char *message) {
    // lưu vào req định dạng <op> <message>
    memset(req, 0, MAXLINE);
    if (op != NULL && strlen(op) > 0) {
        strcat(req, op);
    }
    if (message != NULL && strlen(message) > 0) {
        if (op != NULL && strlen(op) > 0) {
            strcat(req, " ");
        }
        strcat(req, message);
    }
}

/*
* gửi yêu cầu cho máy chủ
* @param {char*} req: yêu cầu từ client
*/
void sendReq(char *req) {
    // if (req == NULL) return;
    // printf("\nReq: %s..\n", req);
    send(sockfd, req, strlen(req), 0);
}

/*
* nhận phản hồi từ server
* @param {char*} res: phản hồi từ server
???
*/
int recvRes(char *res) {
    memset(res, 0, MAXLINE);
    if (0 > recv(sockfd, res, MAXLINE, 0)) {
        printf("\nres: %s\n", res);
        return strlen(res);
    }
    return -1;
}

void teacherMenu() {
    int choice;
    while(1) {
        printf("\n");
        printf(" ___________ TEACHER _________ \n");
        printf("|                             |\n");
        printf("| 1. SHOW ROOM                |\n");
        printf("| 2. CREATE ROOM              |\n");
        printf("| 3. DELETE ROOM              |\n");
        printf("| 4. UPDATE ROOM              |\n");
        printf("| 5. SHOW POINT               |\n");
        printf("| 6. ADD QUESTION             |\n");
        printf("| 0. EXIT                     |\n");
        printf("|_____________________________|\n");
        while(1) {
            choice = -1;
            printf("\n---> Your choice: ");
            scanf("%d", &choice);
            while(getchar() != '\n');
            
            switch (choice) {
                case 1:
                    showRoom();
                    break;
                case 2:
                    createRoom();
                    break;
                case 3:
                    deleteRoom();
                    break;
                case 4:
                    updateRoom();
                    break;
                case 5:
                    showPoint();
                    break;
                case 6:
                    addQuestion();
                    break;
                case 0:
                    printf("\n[Exit]\n");
                    break;
                default:
                    printf("\n[Try again]\n");
            }
            if (choice > -1 && choice < 7) {
                break;
            }
        }
        if (choice == 0) {
            break;
        }
    }
}

/*
* xem thông tin các room hiện có
*/
void showRoom() {
    makeReq(req, "SHOW_ROOM", "");
    sendReq(req);
    handleRes(res, req);
}

/*
* xử lý tạo room mới
*/
void createRoom() {
    char room_name[100]; 
    int status = 1;
    char status_str[100];
    status = sprintf(status_str, "%d", status);;
    printf("\n");
    printf("__________CREATE ROOM__________\n");
    printf("---> Type room name: ");
    scanf("%[^\n]s", room_name);
    while(getchar() != '\n');

    char roomInfo[150];
    strcpy(roomInfo, room_name);
    strcat(roomInfo, " ");
    strcat(roomInfo, status_str);

    makeReq(req, "CREATE_ROOM", roomInfo);
    sendReq(req);

    handleRes(res, req);
}

/*
* xử lý xóa room
*/
void deleteRoom() {
    char room_name[100]; 
    printf("\n");
    printf("_________ DELETE ROOM _________\n");
    printf("---> Type room name: ");
    scanf("%[^\n]s", room_name);
    while(getchar() != '\n');

    makeReq(req, "DELETE_ROOM", room_name);
    sendReq(req);

    handleRes(res, req);
}

void updateRoom() { 
    char room_name[100]; 
    int status;
    printf("\n");
    printf("_________ UPDATE ROOM _________\n");
    printf("---> Type room name: ");
    scanf("%[^\n]s", room_name);
    while(getchar() != '\n');

    printf("---> Type status: ");
    scanf("%d", &status);
    while(getchar() != '\n');

    char status_str[20];
    sprintf(status_str, "%d", status);

    char roomInfo[150];
    strcpy(roomInfo, room_name);
    strcat(roomInfo, " ");
    strcat(roomInfo, status_str);

    makeReq(req, "UPDATE_ROOM", roomInfo);
    sendReq(req);

    handleRes(res, req);
}

/*
* xử lý xem điểm của room
*/
void showPoint() {
    char room_name[100]; 
    printf("\n");
    printf("__________ Show point __________\n");
    printf("---> Type room name: ");
    scanf("%[^\n]s", room_name);
    while(getchar() != '\n');

    makeReq(req, "SHOW_POINT", room_name);
    sendReq(req);

    handleRes(res, req);
}

/*
* xử lý thêm câu hỏi vào room
*/
void addQuestion() {
    char room_name[100] = ""; 
    char f_path[200] = ""; 
    char buf[MAXLINE] = "";
    char f_content[MAXLINE] = "";
    FILE *f = NULL;

    printf("\n");
    printf("__________ ADD QUESTION __________\n");
    printf("---> Type room name: ");
    scanf("%[^\n]s", room_name);
    while(getchar() != '\n');
    printf("---> Type file path: ");
    scanf("%[^\n]s", f_path);
    while(getchar() != '\n');

    if ( (f = fopen(f_path, "r")) == NULL) {
        printf("\n[Error: file not exist !!]\n");
        return;
    }

    // đọc file vào chuỗi f_content
    char c = fgetc(f);
    int i = 0;
    while(c != EOF) {
        f_content[i++] = c;
        c = fgetc(f);
    }
    f_content[i] = '\0';
    fclose(f);
    strcpy(buf, room_name);
    strcat(buf, " ");
    strcat(buf, f_content);

    makeReq(req, "ADD_QUESTION", buf);
    sendReq(req);

    handleRes(res, req);
}