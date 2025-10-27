#define _GNU_SOURCE

#include "movie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Movie movie_cache[100];
extern int movie_count;


int find_movie_by_title(const char *title) {
    for (int i = 0; i < movie_count; i++) {
        printf("Searching for title: %s\n", title);
        printf("Comparing with movie: %s\n", movie_cache[i].title);
        if (strcmp(movie_cache[i].title, title) == 0)
            return i;
    }
    return -1;
}


int find_time_slot(int movie_index, int day_index, const char *time) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (strcmp(movie_cache[movie_index].schedule[day_index][i], time) == 0)
            return i;
    }
    return -1;
}



void normalize_time(char *out, const char *in) {
    int hour;
    if (sscanf(in, "%dh", &hour) == 1) {
        sprintf(out, "%02dh", hour);  // pad with leading zero
    } else {
        out[0] = '\0';  // fallback if input is invalid
    }
}


int map_day_to_index(const char *day) {
    if (strcmp(day, "Thứ 2") == 0) return 0;
    if (strcmp(day, "Thứ 3") == 0) return 1;
    if (strcmp(day, "Thứ 4") == 0) return 2;
    if (strcmp(day, "Thứ 5") == 0) return 3;
    if (strcmp(day, "Thứ 6") == 0) return 4;
    if (strcmp(day, "Thứ 7") == 0) return 5;
    if (strcmp(day, "Chủ nhật") == 0) return 6;
    return -1;
}

int load_movies(const char *filename, Movie *movies, int max_movies) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Cannot open file");
        return 0;
    }

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max_movies) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        // Top-level tokenization
        char *saveptr1;
        char *title = strtok_r(line, "|", &saveptr1);
        char *genre = strtok_r(NULL, "|", &saveptr1);
        char *duration_str = strtok_r(NULL, "|", &saveptr1);
        char *schedule_str = strtok_r(NULL, "", &saveptr1); // rest of line

        if (!title || !genre || !duration_str || !schedule_str) {
            fprintf(stderr, "Skipping malformed line: %s\n", line);
            continue;
        }

        strncpy(movies[count].title, title, sizeof(movies[count].title) - 1);
        strncpy(movies[count].genre, genre, sizeof(movies[count].genre) - 1);
        movies[count].title[sizeof(movies[count].title) - 1] = '\0';
        movies[count].genre[sizeof(movies[count].genre) - 1] = '\0';
        movies[count].duration = atoi(duration_str);

        // Clear schedule
        for (int d = 0; d < MAX_DAYS; d++)
            for (int s = 0; s < MAX_SLOTS; s++)
                movies[count].schedule[d][s][0] = '\0';

        // Copy schedule_str to safely tokenize
        char schedule_copy[800];
        strncpy(schedule_copy, schedule_str, sizeof(schedule_copy) - 1);
        schedule_copy[sizeof(schedule_copy) - 1] = '\0';

        char *saveptr2;
        char *day_block = strtok_r(schedule_copy, ";", &saveptr2);
        while (day_block) {
            char *saveptr3;
            char *day = strtok_r(day_block, ":", &saveptr3);
            char *times = strtok_r(NULL, "", &saveptr3); // rest of block

            if (day && times) {
                int day_index = map_day_to_index(day);
                if (day_index >= 0 && day_index < MAX_DAYS) {
                    int slot = 0;
                    char *saveptr4;
                    char *time = strtok_r(times, ",", &saveptr4);
                    while (time && slot < MAX_SLOTS) {
                        strncpy(movies[count].schedule[day_index][slot],
                                time,
                                sizeof(movies[count].schedule[day_index][slot]) - 1);
                        movies[count].schedule[day_index][slot][sizeof(movies[count].schedule[day_index][slot]) - 1] = '\0';
                        slot++;
                        time = strtok_r(NULL, ",", &saveptr4);
                    }
                }
            }

            day_block = strtok_r(NULL, ";", &saveptr2);
        }

        count++;
    }

    fclose(fp);
    return count;
}

int save_movie(const char *filename, const Movie *movie) {
    FILE *fp = fopen(filename, "a");
    if (!fp) return 0;

    fprintf(fp, "%s|%s|%d|", movie->title, movie->genre, movie->duration);

    for (int d = 0; d < MAX_DAYS; d++) {
        int has_day = 0;
        for (int s = 0; s < MAX_SLOTS; s++) {
            if (strlen(movie->schedule[d][s]) > 0) {
                if (!has_day) {
                    fprintf(fp, "%s:", d == 0 ? "Thứ 2" :
                                        d == 1 ? "Thứ 3" :
                                        d == 2 ? "Thứ 4" :
                                        d == 3 ? "Thứ 5" :
                                        d == 4 ? "Thứ 6" :
                                        d == 5 ? "Thứ 7" : "Chủ nhật");
                    has_day = 1;
                }
                fprintf(fp, "%s,", movie->schedule[d][s]);
            }
        }
        if (has_day) fprintf(fp, ";");
    }

    fprintf(fp, "\n");
    fclose(fp);
    return 1;
}
#include "movie.h"
#include <string.h>

void handle_list_movies(char *response_out) {
    strcpy(response_out, "");
    for (int i = 0; i < movie_count; i++) {
        char line[256];
        sprintf(line, "Title: %s\nGenre: %s\nDuration: %d mins\n\n",
                movie_cache[i].title, movie_cache[i].genre, movie_cache[i].duration);
        strcat(response_out, line);
    }
}

void handle_search(const char *payload, char *response_out) {
    char title[MAX_TITLE];
    sscanf(payload, "SEARCH title=%s", title);

    for (int i = 0; i < movie_count; i++) {
        if (strcmp(movie_cache[i].title, title) == 0) {
            sprintf(response_out, "Title: %s\nGenre: %s\nDuration: %d mins\n",
                    movie_cache[i].title, movie_cache[i].genre, movie_cache[i].duration);
            return;
        }
    }
    strcpy(response_out, "NOT_FOUND");
}

void handle_list_genres(char *response_out) {
    char genres[100][MAX_GENRE];
    int count = 0;
    strcpy(response_out, "");

    for (int i = 0; i < movie_count; i++) {
        int found = 0;
        for (int j = 0; j < count; j++) {
            if (strcmp(genres[j], movie_cache[i].genre) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(genres[count++], movie_cache[i].genre);
        }
    }

    for (int i = 0; i < count; i++) {
        strcat(response_out, genres[i]);
        strcat(response_out, "\n");
    }
}

void handle_filter_genre(const char *payload, char *response_out) {
    char genre[MAX_GENRE];
    sscanf(payload, "FILTER_GENRE genre=\"%[^\"]\"", genre);
    printf("Filtering genre: %s\n", genre);
    strcpy(response_out, "");

    for (int i = 0; i < movie_count; i++) {
        if (strcmp(movie_cache[i].genre, genre) == 0) {
            printf("Matched movie: %s\n", movie_cache[i].title);
            char line[256];
            sprintf(line, "Title: %s\nDuration: %d mins\n\n",
                    movie_cache[i].title, movie_cache[i].duration);
            strcat(response_out, line);
        }
    }
}

void handle_filter_time(const char *payload, char *response_out) {
    char day[10], begin[10], end[10];
    int day_index;
    sscanf(payload, "FILTER_TIME day=\"%[^\"]\" begin=%s end=%s", day, begin, end);
    strcpy(response_out, "");
    printf("Filtering time on day: %s from %s to %s\n", day, begin, end);
    day_index = map_day_to_index(day);
    for (int i = 0; i < movie_count; i++) {
        for (int s = 0; s < MAX_SLOTS; s++) {
            char *time = movie_cache[i].schedule[day_index][s];
            printf("Checking movie: %s, time: %s\n", movie_cache[i].title, time);
            if (strlen(time) == 0) continue;
            char norm_time[10], norm_begin[10], norm_end[10];
            normalize_time(norm_time, time);
            normalize_time(norm_begin, begin);
            normalize_time(norm_end, end);
            printf("Normalized times - movie: %s, begin: %s, end: %s\n", norm_time, norm_begin, norm_end);
            if (strcmp(norm_time, norm_begin) >= 0 && strcmp(norm_time, norm_end) <= 0) {
                char line[256];
                printf("Matched movie: %s at time: %s\n", movie_cache[i].title, time);
                sprintf(line, "Title: %s\nTime: %s\n\n", movie_cache[i].title, time);
                strcat(response_out, line);
                printf("Added to response: %s\n", line);
            }

        }
    }
}

void handle_get_seatmap(const char *payload, char *response_out) {
    char title[MAX_TITLE], day[10], time[10];
    sscanf(payload, "GET_SEATMAP title=\"%[^\"]\" day=\"%[^\"]\" start=%s", title, day, time);
    printf("Getting seatmap for movie: %s, day: %s", title, day);
    int movie_index = find_movie_by_title(title);
    if (movie_index == -1) {
        sprintf(response_out, "Movie not found: %s", title);
        return;
    }

    int day_index = map_day_to_index(day);
    if (day_index < 0 || day_index >= MAX_DAYS) {
        sprintf(response_out, "Invalid day: %s", day);
        return;
    }

    int time_index = find_time_slot(movie_index, day_index, time);
    if (time_index == -1) {
        sprintf(response_out, "Time slot not found: %s", time);
        return;
    }

    response_out[0] = '\0';
    for (int r = 0; r < 3; r++) {
        strcat(response_out, "|");
        for (int c = 0; c < 5; c++) {
            char seat[4];
            sprintf(seat, "%s|", movie_cache[movie_index].seatmap[day_index][time_index][r][c] == 'x' ? "x" : " ");
            strcat(response_out, seat);
        }
        strcat(response_out, "\n");
    }
}



void handle_book_seat(const char *payload, char *response_out) {
    int row, col;
    char title[MAX_TITLE], day[10], time[10];
    sscanf(payload, "BOOK_SEAT title=\"%[^\"]\" day=\"%[^\"]\" time=%s row=%d col=%d",
           title, day, time, &row, &col);

    int movie_index = find_movie_by_title(title);
    if (movie_index == -1) {
        sprintf(response_out, "Movie not found: %s", title);
        return;
    }

    int day_index = map_day_to_index(day);
    if (day_index < 0 || day_index >= MAX_DAYS) {
        sprintf(response_out, "Invalid day: %s", day);
        return;
    }

    int time_index = find_time_slot(movie_index, day_index, time);
    if (time_index == -1) {
        sprintf(response_out, "Time slot not found: %s", time);
        return;
    }

    if (row < 1 || row > 3 || col < 1 || col > 5) {
        sprintf(response_out, "Invalid seat position.");
        return;
    }

    if (movie_cache[movie_index].seatmap[day_index][time_index][row - 1][col - 1] == 'x') {
        sprintf(response_out, "Seat (%d,%d) is already booked.", row, col);
        return;
    }

    movie_cache[movie_index].seatmap[day_index][time_index][row - 1][col - 1] = 'x';
    sprintf(response_out, "Seat (%d,%d) booked for %s at %s %s", row, col, title, day, time);
}


