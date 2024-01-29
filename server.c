#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "server.h"
#include "storage.h"

#define MAX 100

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/


int point;
char currentUserName[100];
char currentRoom[100];
int fd;
int isCorrectAnswer[20];
int correctAnswer[20];


int main (int argc, char **argv)
{
    int listenfd, connfd, n;
    pid_t childpid;
    socklen_t clilen;
    char buf[MAXLINE];
    char req[MAXLINE], res[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;

    //Create a socket for the soclet
    //If sockfd<0 there was an error in the creation of the socket
    if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
    }

    //preparation of the socket address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    //bind the socket
    bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    //listen to the socket by creating a connection queue, then wait for clients
    listen (listenfd, LISTENQ);

    printf("%s\n","Server running...waiting for connections.");

    for ( ; ; ) {
        clilen = sizeof(cliaddr);

        //accept a connection
        connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
        fd = connfd;
        printf("%s\n","Received request...");

        if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process

            printf ("%s\n","Child created for dealing with client requests");

            //close listening socket
            close (listenfd);
            // printf("fd: %d\n", connfd);
            while (1)  {
                printf("server handle req...");
                handle(req, res);    
            }

            if (n < 0)
                printf("%s\n", "Read error");
            exit(0);
        }
        //close socket of the server
        close(connfd);
        
    }
}


/**
* xử lý yêu cầu đến từ máy khách
* @param {char*} req: yêu cầu từ client
*/
void handle(char *req, char *res) {

    // op: xác định loại thao tác mà client muốn thực hiện
    // message: thông điệp đi kèm với thao tác đó
    char op[MAX], message[MAXLINE];
    recvReq(req); // lưu lại chuỗi req
    parseReq(req, op, message); // tách phần op và message
    if(strcmp(op, "SIGNUP_STUDENT") == 0 || strcmp(op,"SIGNUP_TEACHER") == 0) {
        Account acc;
        strcpy(acc.username, strtok(message, " "));
        strcpy(acc.password, strtok(NULL, " "));
        char *confirmPassword = strtok(NULL, " ");
        char *roleName = strtok(NULL, " ");
        int role = strcmp(roleName, "student") == 0 ? 1 : 2;
        int signupSuccess = signUp(acc.username, acc.password, confirmPassword, role);
    }
    else if (strcmp(op, "LOGIN_STUDENT") == 0) {
        Account acc;
        strcpy(acc.username, strtok(message, " "));
        strcpy(acc.password, strtok(NULL, ""));
        int loginSuccess = loginStudent(acc.username, acc.password);
    
        if(loginSuccess) {
            char infoActivity[100];
            strcpy(infoActivity, "LOGIN ");
            strcat(infoActivity, currentUserName);
            writeLog(infoActivity);
        }

    } else if (strcmp(op, "LOGIN_TEACHER") == 0) {
        Account acc;
        strcpy(acc.username, strtok(message, " "));
        strcpy(acc.password, strtok(NULL, ""));
        int loginSuccess = loginTeacher(acc.username, acc.password);

        if(loginSuccess) {
            char infoActivity[100];
            strcpy(infoActivity, "LOGIN ");
            strcat(infoActivity, currentUserName);
            writeLog(infoActivity);
        }

    } else if (strcmp(op, "SHOW_ROOM") == 0) {
        int status = showRoom();

    } else if (strcmp(op, "CREATE_ROOM") == 0) {
        int status = createRoom(message);

        if(status) {
            char infoActivity[100];
            strcpy(infoActivity, "CREATE_ROOM ");
            strcat(infoActivity, currentUserName);
            writeLog(infoActivity);
        }

    } else if (strcmp(op, "DELETE_ROOM") == 0) {
        int status = deleteRoom(message);

        if(status) {
            char infoActivity[100];
            strcpy(infoActivity, "DELETE_ROOM ");
            strcat(infoActivity, currentUserName);
            writeLog(infoActivity);
        }

    } else if(strcmp(op, "UPDATE_ROOM") == 0) {
        printf("vao update\n");
        int status = updateRoom(message);
    } 
    else if (strcmp(op, "JOIN_ROOM") == 0) {
        int status = joinRoom(message);
        if(status) {
            char infoActivity[100];
            strcpy(infoActivity, "JOIN_ROOM ");
            strcat(infoActivity, currentUserName);
            strcat(infoActivity, " room: ");
            strcat(infoActivity, currentRoom);
            writeLog(infoActivity);
        }

    } else if (strcmp(op, "START_TEST") == 0) {
        int status = startTest();
        if(status) {
            char infoActivity[100];
            strcpy(infoActivity, "START_TEST ");
            strcat(infoActivity, currentUserName);
            writeLog(infoActivity);
        }
    } else if(strcmp(op, "EDIT") == 0) {
        int status = editAnswerQuestion(message);
    } else if (strcmp(op, "ANSWER") == 0) {
        // printf("Chon nop bai thi \n");
        makeRes(res, "", "");
        sprintf(res, "%s %d", "ANSWER_OK", point);
        sendRes(res);

        char infoActivity[100];
        char strPoint[100];
        sprintf(strPoint, "%d", point);

        strcpy(infoActivity, "END_TEST ");
        strcat(infoActivity, currentUserName);
        strcat(infoActivity, " point: ");
        strcat(infoActivity, strPoint);
        writeLog(infoActivity);
    } else if (strcmp(op, "ADD_QUESTION") == 0) {
        int status = addQuestion(message);

    } else if (strcmp(op, "SHOW_POINT") == 0) {
        int status = showPoint(message);
    }
}

/*
* nhận yêu cầu từ client
* @param {char*} req: yêu cầu từ client
*/
int recvReq(char *req) {
    int len = 0; // độ dài yêu cầu nhận được
    memset(req, 0, MAXLINE);
    // if (0 > recv(fd, req, MAXLINE, 0)) {
    //     perror("Lỗi recv");
    //     return -1;
    // }
    recv(fd, req, MAXLINE, 0); // nhận yêu cầu qua socket fd và lưu vào vùng nhớ req
    len = strlen(req);
    // printf("\nReq: %s.. len: %d\n", req, len);
    return len;
}

/*
* phân tích yêu cầu từ client
*@param {char*} req: yêu cầu từ client
*@param {char*} op: thao tác mà client muốn thực hiện
*@param {char*} message: thông điệp đi kèm với thao tác đó
*/
void parseReq(char *req, char *op, char *message) {
    char *next_s;
    // tách các phần của req
    printf("req server: %s\n", req);
    // if(!req) return;
    strcpy(op, strtok(req, " ")); // lưu phần đầu vào biến op
    if ((next_s = strtok(NULL, ""))) {
        strcpy(message, next_s);
    }
}

/*
* tạo response gửi lại cho client
* @param {char*} res: phản hồi cho client
*@param {char*} op: thao tác mà client muốn thực hiện
*@param {char*} message: thông điệp đi kèm với thao tác đó
*/
void makeRes(char *res, char *op, char *message) {
    memset(res, 0, MAXLINE);
    if (op != NULL && strlen(op) > 0) {
        strcat(res, op); // gắn op vào đầu chuỗi res
    }
    // thêm dấu cách và message(nếu có) vào res
    if (message != NULL && strlen(message) > 0) {
        if (op != NULL && strlen(op) > 0) {
            strcat(res, " ");
        }
        strcat(res, message);
    }
}

/*
* gửi phản hồi cho client
* @param {char*} res: phản hồi cho client
*/
void sendRes(char *res) {
    if (res == NULL) return;
    printf("\nRes: %s..\n", res);
    // gửi dữ liệu phản hồi thông qua kết nối socket fd. truyền đi nội dung chuỗi res.
    send(fd, res, strlen(res), 0); 
}

/*
* xử lý kiểm tra usename khi đăng ký tài khoản
* @param {char*} usename: tên đăng nhập
* @param {int} role: vai trò đăng ký(1-student, 2-teacher)
*/
int isUsernameExist(char *username, int role) {
    FILE *file;
    char filename[MAX];

    if (role == 1) {
        strcpy(filename, "acc_stud.txt");
    } else if (role == 2) {
        strcpy(filename, "acc_teacher.txt");
    }

    file = fopen(filename, "r"); // Mở file để đọc

    if (file == NULL) {
        printf("Cannot open file!\n");
        return -1; // Trả về -1 nếu không thể mở file
    }

    char line[MAX];
    while (fgets(line, sizeof(line), file)) { // Đọc từng dòng trong file
        char savedUsername[MAX];
        sscanf(line, "%s", savedUsername); // Lấy tên người dùng từ dòng đọc được

        if (strcmp(savedUsername, username) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0; 
}

/*
* xử lý đăng ký tài khoản
* @param {char*} usename: tên đăng nhập
* @param {char*} password:  mật khẩu
* @param {char*} comfirmPassword: xác nhận mất khẩu
* @param {int} role: vai trò đăng ký(1-student, 2-teacher)
*/
int signUp(char *username, char *password, char *comfirmPassword, int role) {
    char res[MAXLINE] = "";
    if (strcmp(password, comfirmPassword) != 0) {
        makeRes(res, "PASSWORDS_NOT_MATCH", "");
        sendRes(res);
        return 0;
    }

    if (isUsernameExist(username, role) == 1) {
        makeRes(res, "USERNAME_ALREADY_EXISTS", "");
        sendRes(res);
        return 0; // Trả về 0 nếu tên người dùng đã tồn tại
    }

    FILE *file;
    char filename[MAX];
    if (role == 1) {
        strcpy(filename, "acc_stud.txt");
    } else if (role == 2) {
        strcpy(filename, "acc_teacher.txt");
    }
    file = fopen(filename, "a");

    if (file == NULL) {
        printf("Cannot open file!\n");
        return 0; // Trả về 0 nếu không thể mở file
    }

    fprintf(file, "%s %s\n", username, password);
    fclose(file);

    makeRes(res, "SIGNUP_OK", "");
    sendRes(res);
    return 1;
}

/*
* xử lý đăng nhập với student
* @param {char*} usename: tên đăng nhập
* @param {char*} password: mật khẩu
*/
int loginStudent(char *username, char *password) {
    char buf[MAXLINE] = ""; // danh sách phòng học
    char res[MAXLINE] = "";

    List accStudL = getAllAccount(ACCCOUNT_STUDENT); // lấy tất cả acc student
    Account *stud = searchAccountByUsername(&accStudL, username); // tìm kiếm acc trogn ds
    if (stud) {
        if (strcmp(stud->password, password) == 0) {
            List roomL = getAllRooms(ROOM_FILE); // lấy all roon
            Node n = roomL.head;
            for (int i = 0; i<roomL.count; i++) {
                Room* r = (Room*)n->value;
                char *roomStatus = r->status == 0 ? "close" : "available";
                if (i != 0) {
                    strcat(buf, "\n");
                    strcat(buf, r->name);
                    strcat(buf, " ");
                    strcat(buf, roomStatus);
                } else {
                    strcat(buf, r->name);
                    strcat(buf, " ");
                    strcat(buf, roomStatus);
                }
                n = n->next;
            }
            strcpy(currentUserName, username); // lưu lại tên người dùng hiện tại

            makeRes(res, "LOGIN_STUDENT_OK", buf);
            sendRes(res);

            freeList(&roomL);
            freeList(&accStudL);
            return 1;
        }
    }
    // không khớp thì gửi phản hồi thông báo 
    makeRes(res, "LOGIN_STUDENT_NOT_OK", "");
    sendRes(res);
    freeList(&accStudL);
    return 0;
}

/*
* xử lý đăng nhập với teacher
* @param {char*} usename: tên đăng nhập
* @param {char*} password: tên mật khẩu
*/
int loginTeacher(char *username, char *password) {
    char res[MAXLINE] = "";

    List accStudL = getAllAccount(ACCCOUNT_TEACHER); // lấy danh sách gv
    Account *acc = searchAccountByUsername(&accStudL, username); // ktr gv
    if (acc) {
        if (strcmp(acc->password, password) == 0) {
            strcpy(currentUserName, acc->username);
            makeRes(res, "LOGIN_TEACHER_OK", "");
            sendRes(res);
            return 1;
        }
    }
    makeRes(res, "LOGIN_TEACHER_NOT_OK", "");
    sendRes(res);

    return 0;
}

/*
* xử lý tạo room
* @param {char*} room_info: tên phòng
*/
int createRoom(char *room_info) {
    char res[MAXLINE] = "";
    char file[MAX] = "";
    
    char room_name[MAX];
    int status;
    sscanf(room_info, "%s %d", room_name, &status);

    List roomL = getAllRooms(ROOM_FILE);
    // kiển tra trùng tên phòng

    if (searchRoomByName(&roomL, room_name)) {
        makeRes(res, "CREATE_ROOM_NOT_OK", "");
        sendRes(res);
        return 0;
    }

    Room *r = (Room*)malloc(sizeof(Room));
    strcpy(r->name, room_name);
    r->status = status;
    addEnd(&roomL, r);
    saveAllRoom(ROOM_FILE, roomL);

    strcat(file, RESULT_FOLDER);
    strcat(file, "/");
    strcat(file, room_name);
    strcat(file, ".txt");

    // printf("file path: %s..\n", file);
    FILE *f = fopen(file, "w");
    // if (f) {fclose(f);}
    fclose(f);
    makeRes(res, "CREATE_ROOM_OK", "");
    sendRes(res);
    return 1;
}

/*
* xử lý khi cập nhật lại trạng thái của room
* @param {char *} room_info: thông tin room cần chỉnh sửa <tên> <trạng thái>
*/
int updateRoom(char *room_info) {
    char res[MAXLINE] = "";
    char *room_name = strtok(room_info, " ");
    char *status_str = strtok(NULL, " ");
    int  status = atoi(status_str);
    // printf("thong tin phong: %s %d", room_name, status);
    List roomL = getAllRooms(ROOM_FILE);
    
    Room *r = searchRoomByName(&roomL, room_name);
    if (!r) {
        makeRes(res, "UPDATE_ROOM_NOT_OK", "");
        sendRes(res);
        return 0;
    }
    // printf("Da thay room %s %d", room_name, status);

    FILE *file = fopen(ROOM_FILE, "r+");
    if (file == NULL) {
        printf("Không thể mở file!\n");
        return 0;
    }
    Room room;
    int found = 0;
    while (fscanf(file, "%s %d\n", room.name, &room.status) == 2) {
        if (strcmp(room.name, room_name) == 0) {
            fseek(file, -strlen(room.name) - 3, SEEK_CUR); // Di chuyển con trỏ đọc/ghi về vị trí đúng
            fprintf(file, "%s %d\n", room.name, status); // Ghi thông tin mới vào file
            found = 1;
            break;
        }
    }
    fclose(file);
    makeRes(res, "UPDATE_ROOM_OK", "");
    sendRes(res);
    return 1;
}

/*
* xử lý xóa room
* @param {char*} room_name: tên phòng
*/
int deleteRoom(char *room_name) {
    char res[MAXLINE] = "";
    char file[MAX] = "";

    List roomL = getAllRooms(ROOM_FILE);
    if (deleteRoomByName(&roomL, room_name)) {
        saveAllRoom(ROOM_FILE, roomL);

        // xóa file kết quả phòng thi
        strcat(file, RESULT_FOLDER);
        strcat(file, "/");
        strcat(file, room_name);
        strcat(file, ".txt");
        if (remove(file) == 0) {
            // printf("deletefileok\n");
        }

        // xóa file câu hỏi thi của phòng
        strcpy(file, "question/");
        strcat(file, room_name);
        strcat(file, ".txt");
        if (remove(file) == 0) {
            // printf("deletefileok\n");
        }

        makeRes(res, "DELETE_ROOM_OK", "");
        sendRes(res);
        return 1;
    }
    makeRes(res, "DELETE_ROOM_NOT_OK", "");
    sendRes(res);
    return 0;
}

/*
* hiển thị các room
*/
int showRoom() {
    char res[MAXLINE] = "";
    char buf[MAXLINE] = "";

    List roomL = getAllRooms(ROOM_FILE);
    Node node = roomL.head;
    // lưu danh sách tên của phòng
    for (int i = 0; i<roomL.count; i++) {
        if (i != 0) strcat(buf, "\n");
        strcat(buf, ((Room*)node->value)->name);
        strcat(buf, " ");
        char *roomStatus = ((Room*)node->value)->status == 0 ? "close" : "available";
        strcat(buf, roomStatus);
        node = node->next;
    }

    makeRes(res, "SHOW_ROOM_OK", buf);
    sendRes(res);
    return 1;
}

/*
* xử lý vào phòng thi
* @param {char*} room_name: tên phòng
*/
int joinRoom(char *room_name) {
    char res[MAXLINE] = "";

    List roomL = getAllRooms(ROOM_FILE);
    Room *r = (Room*)malloc(sizeof(Room));
    r = searchRoomByName(&roomL, room_name);
    if (r) {
        if(r->status == 0) {
            makeRes(res, "ROOM_CLOSE", "");
            sendRes(res);
            return 0;
        }else {
            strcpy(currentRoom, room_name);
            makeRes(res, "JOIN_ROOM_OK", "");
            sendRes(res);
            return 1;
        }
        // strcpy(currentRoom, room_name);
        // makeRes(res, "JOIN_ROOM_OK", "");
        // sendRes(res);
        // return 1;
    }
    makeRes(res, "JOIN_ROOM_NOT_OK", "");
    sendRes(res);
    return 0;

}

/*
* xử lý bắt đầu thi
*/
int startTest() {
    char res[MAXLINE] = "";
    char req[MAXLINE] = "";
    char buf[MAXLINE] = "";

    memset(isCorrectAnswer, 0, sizeof(isCorrectAnswer));
    memset(correctAnswer, 0, sizeof(correctAnswer));
    
    Question *q;
    List quesL = makeQues(currentRoom);
    // printf("room: %s - count: %d\n", currentRoom, quesL.count);
    Node n = quesL.head;
    point = 0;
    for (int i = 0; i<10; i++) {
        q = (Question*)n->value;
        // printf("anwser %s", q->answer);
        quesToString(q, buf);

        if (i == 0)
            makeRes(res, "START_TEST_OK", buf);
        else 
            makeRes(res, "", buf);
        sendRes(res);
        recvReq(req); // nhận câu trả lời từ client

        printf("\nanswer req: %s.\n", req);
        printf("\nanswer correct: %s.\n", q->answer);

        if(atoi(req) == atoi(q->answer)) {
            point++;
            isCorrectAnswer[i] = 1; 
        }
        correctAnswer[i] = atoi(q->answer);
        printf("Diem %d\n", point);
        memset(buf, 0, MAXLINE);
        n = n->next;
    }
    // else if(mode == 1) {
    //     int editQuesNumber;
    //     int answerEdit;
    //     char option[200];
    //     recvReq(req); // nhận câu trả lời từ client
    //     printf("req mode edit: %s\n", req);
    //     sscanf(req, "%s %d %d",option, &editQuesNumber, &answerEdit);
    //     printf("option: %s.\n", option);
    //     printf("edit ques number: %d.\n", editQuesNumber);
    //     printf("answer edit: %d.\n", answerEdit);
    //     printf("is correct %d\n", isCorrectAnswer[editQuesNumber - 1]);
    //     printf( "correct answer %d\n", correctAnswer[editQuesNumber - 1]);

    //     if(isCorrectAnswer[editQuesNumber - 1]) {
    //         if(answerEdit != correctAnswer[editQuesNumber - 1]) point--;
    //     }else {
    //         if(answerEdit == correctAnswer[editQuesNumber - 1]) point++;
    //     }
    // }

    printf("\npoint: %d\n", point);

    char f[100];
    strcpy(f, RESULT_FOLDER);
    strcat(f, "/");
    strcat(f, currentRoom);
    strcat(f, ".txt");

    List roompL = getAllRoomPoint(f);
    RoomPoint *rp = searchRoomPoint(&roompL, currentUserName);
    // nếu tìm thấy kết quả của học sinh thì cập nhật
    if (rp) {
        sprintf(rp->point, "%d", point);
    } else { // nếu k thì tạo mới kết quả và thêm vào danh sách
        RoomPoint *new = (RoomPoint*)malloc(sizeof(RoomPoint));
        strcpy(new->stud_name, currentUserName);
        sprintf(new->point, "%d", point);
        addEnd(&roompL, new);
    }
    saveAllRoomPoint(f, roompL);
    freeList(&roompL);
    return 1;
}

/*
* chỉnh sửa câu trả lời
* @param {char *}:  lưu thông tin câu trả lời chỉnh sửa:<op> <câu hỏi> <đáp án chỉnh sửa>
*/
int editAnswerQuestion(char *message) {
    char res[MAXLINE] = "";
    int editQuesNumber;
    int answerEdit;
    printf("req mode edit: %s\n", message);
    sscanf(message, "%d %d", &editQuesNumber, &answerEdit);
    printf("edit ques number: %d.\n", editQuesNumber);
    printf("answer edit: %d.\n", answerEdit);
    printf("is correct %d\n", isCorrectAnswer[editQuesNumber - 1]);
    printf( "correct answer %d\n", correctAnswer[editQuesNumber - 1]);

    if(isCorrectAnswer[editQuesNumber - 1]) {
        if(answerEdit != correctAnswer[editQuesNumber - 1]) point--;
    }else {
        if(answerEdit == correctAnswer[editQuesNumber - 1]) point++;
    }
    printf("Diem sau edit %d\n", point);
    // makeRes(res, "EDIT_OK", "");
    // sendRes(res);
    char f[100];
    strcpy(f, RESULT_FOLDER);
    strcat(f, "/");
    strcat(f, currentRoom);
    strcat(f, ".txt");

    List roompL = getAllRoomPoint(f);
    RoomPoint *rp = searchRoomPoint(&roompL, currentUserName);
    // nếu tìm thấy kết quả của học sinh thì cập nhật
    if (rp) {
        sprintf(rp->point, "%d", point);
    } else { // nếu k thì tạo mới kết quả và thêm vào danh sách
        RoomPoint *new = (RoomPoint*)malloc(sizeof(RoomPoint));
        strcpy(new->stud_name, currentUserName);
        sprintf(new->point, "%d", point);
        addEnd(&roompL, new);
    }
    saveAllRoomPoint(f, roompL);
    freeList(&roompL);
    return 1;
}


/*
* thêm câu hỏi vào phòng học cụ thể
* @param {char*} room_ques: tên phòng và nội dung câu hỏi 
*/
int addQuestion(char *room_ques) {
    char res[MAXLINE] = "";
    char *buf =(char*)malloc(sizeof(char)*MAXLINE);
    char f_path[100] = "";
    char room[100] = "";
    FILE *fq;

    strcpy(room, strtok(room_ques, " ")); // tách tên phòng
    strcpy(buf, strtok(NULL, ""));   // tách câu hỏi

    List roomL = getAllRooms(ROOM_FILE);
    Room* r = searchRoomByName(&roomL, room);

    if (r) {
        if(r->status == 0) {
            makeRes(res, "ROOM_CLOSE", "");
            sendRes(res);
            return 0;
        }
        strcpy(f_path, "question/");
        strcat(f_path, room);
        strcat(f_path, ".txt");
        // printf("path: %s\n", f_path);

        fq = fopen(f_path, "w");
        // fprintf(f, "%s", buf);
        // char c = buf[0];
        // int i = 0;
        // fputc(c, fq);
        
        fwrite(buf, strlen(buf), 1, fq);
        // printf("len: %d\n", strlen(buf));
        // printf("buf: %s\n", buf);
        fclose(fq);
        makeRes(res, "ADD_QUESTION_OK", "");
    } else {
        makeRes(res, "ADD_QUESTION_NOT_OK", "");
    }
    sendRes(res);
    return 0;
}

/*
* hiển thị bảng điểm của phòng thi
* @param {char*} room_name: tên phòng
*/
int showPoint(char *room_name) {
    char file[MAX] = "";
    char buf[MAXLINE] = "";
    char res[MAXLINE] = "";
    
    List roomL = getAllRooms(ROOM_FILE);
    Room *r = searchRoomByName(&roomL, room_name);
    if (r) {
        strcpy(file, RESULT_FOLDER);
        strcat(file, "/");
        strcat(file, room_name);
        strcat(file, ".txt");

        List roompL = getAllRoomPoint(file);
        Node n = roompL.head;
        // tạo chuỗi chứa thông tin <tên sv> <điểm>
        for (int i = 0; i<roompL.count; i++) {
            RoomPoint *rp = (RoomPoint*)n->value;
            // printf("name: %s.. point: %d..\n", rp->stud_name, rp->point);
            strcat(buf, "\n");
            strcat(buf, rp->stud_name);
            strcat(buf, "\t");
            strcat(buf, rp->point);

            // printf("buf: %s..\n", buf);
            n = n->next;
        }

        makeRes(res, "SHOW_POINT_OK", buf);
        sendRes(res);
        return 1;
    } else {
        makeRes(res, "SHOW_POINT_NOT_OK", "");
        sendRes(res);
    }
    return 0;
}

/*
* ghi log hoạt động của nguời dùng
* @param {char *} activity: thông tin về hoạt động của nguời dùng
*/
void writeLog(char *activity) {
    // printf("activity %s", activity);
    FILE *file = fopen("log.txt", "a");
    if (file != NULL) {
        time_t t = time(NULL); // Lấy thời gian hiện tại
        struct tm *tm_info = localtime(&t);

        fprintf(file, "[%d-%02d-%02d %02d:%02d:%02d] %s\n",
                tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, activity
                );

        fclose(file);
    } else {
        printf("Error opening log file!\n");
    }
}