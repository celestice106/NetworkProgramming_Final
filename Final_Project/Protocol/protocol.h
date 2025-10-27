#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAXLINE 1024
#define MAX_PAYLOAD 1000

typedef enum {
    CREATE_ACCOUNT,
    LOGIN,
    CHECK_USERNAME,
    CHECK_PASSWORD,
    SEARCH,
    LIST_MOVIES,
    LIST_GENRES,
    FILTER_GENRE,
    FILTER_TIME,
    GET_SEATMAP,
    BOOK_SEAT,
    UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    char payload[MAX_PAYLOAD];
} Message;

CommandType parse_command_type(const char *payload);
void parse_message(const char *raw, Message *msg);
void format_message(CommandType type, const char *payload, char *out);

#endif
