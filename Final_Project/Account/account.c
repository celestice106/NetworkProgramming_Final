#include "account.h"
#include <stdio.h>
#include <string.h>

extern User users[MAX_USERS];
extern int user_count;

int load_users(const char *filename, User *users, int max_users) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    int count = 0;
    while (fscanf(fp, "%s %s %s", users[count].username, users[count].password, users[count].role) == 3) {
        count++;
        if (count >= max_users) break;
    }

    fclose(fp);
    return count;
}

int save_user(const char *filename, const User *user) {
    FILE *fp = fopen(filename, "a");
    if (!fp) return 0;

    fprintf(fp, "%s %s %s\n", user->username, user->password, user->role);
    fclose(fp);
    return 1;
}

int username_exists(const char *username, User *users, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0) return 1;
    }
    return 0;
}

int authenticate(const char *username, const char *password, User *users, int count, char *role_out) {
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            strcpy(role_out, users[i].role);
            return 1;
        }
    }
    return 0;
}


void handle_login(const char *payload, char *response_out) {
    char username[MAX_USERNAME], password[MAX_PASSWORD];
    sscanf(payload, "LOGIN username=%s password=%s", username, password);

    char role[MAX_ROLE];
    int ok = authenticate(username, password, users, user_count, role);
    if (ok) {
        sprintf(response_out, "SUCCESS %s", role);
    } else {
        strcpy(response_out, "FAIL");
    }
}

void handle_create_account(const char *payload, char *response_out) {
    char username[MAX_USERNAME], password[MAX_PASSWORD], role[MAX_ROLE];
    printf("pass: %s\n", password);
    // Safe and correct code
    sscanf(payload, "CREATE_ACCOUNT username=%49s password=%49s role=%19s", 
        username, password, role);
    printf("%s\n", password);
    printf("Creating account: %s, %s, %s\n", username, password, role);


    if (username_exists(username, users, user_count)) {
        strcpy(response_out, "EXISTS");
        return;
    }

    User new_user;
    strcpy(new_user.username, username);
    strcpy(new_user.password, password);
    strcpy(new_user.role, role);

    if (save_user("users.txt", &new_user)) {
        users[user_count++] = new_user;
        strcpy(response_out, "SUCCESS");
    } else {
        strcpy(response_out, "FAIL");
    }
}

