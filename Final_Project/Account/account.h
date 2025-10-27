#ifndef ACCOUNT_H
#define ACCOUNT_H

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_ROLE 10
#define MAX_USERS 100

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char role[MAX_ROLE]; // "admin" or "user"
} User;

// Core user database functions
int load_users(const char *filename, User *users, int max_users);
int save_user(const char *filename, const User *user);
int username_exists(const char *username, User *users, int count);
int authenticate(const char *username, const char *password, User *users, int count, char *role_out);

// Server-side handlers
void handle_login(const char *payload, char *response_out);
void handle_create_account(const char *payload, char *response_out);

#endif
