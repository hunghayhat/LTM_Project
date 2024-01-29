#ifndef __CLIENT_H__
#define __CLIENT_H__

void home();
void signup();
void login(int mode);
void startTest();
void joinRoom(char *list_room);
void answer(char *x);
void parseQuestion(char *q, char *ques, char choice[][100]);

void createRoom();
void showRoom();
void updateRoom(); 
void deleteRoom();
void showPoint();
void addQuestion();

void teacherMenu();

void makeReq(char *req, char *op, char *message);
void sendReq(char *req);
int recvRes(char *res);
void parseRes(char *res, char *op, char *message);

void handleRes(char *res, char *req);
#endif