#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct buffer {
    char *data;
    size_t size;
};

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total = size * nmemb;
    struct buffer *buf = userdata;

    char *tmp = realloc(buf->data, buf->size + total + 1);
    if (!tmp)
        return 0;

    buf->data = tmp;
    memcpy(buf->data + buf->size, ptr, total);
    buf->size += total;
    buf->data[buf->size] = '\0';

    return total;
}

int main(void)
{
    CURL *curl;
    struct buffer buf = {0};
    char *p;
    int projects, packages, builds;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl init failed\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://oss.qnx.com/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (curl_easy_perform(curl) != CURLE_OK) {
        fprintf(stderr, "curl request failed\n");
        curl_easy_cleanup(curl);
        return 1;
    }

    /* grep / sed / awk equivalent */
    p = strstr(buf.data, "Projects:");
    if (p && sscanf(p,
            "Projects: %d Packages: %d Builds: %d",
            &projects, &packages, &builds) == 3) {

        printf("Projects: %d Packages: %d Builds: %d\n",
               projects, packages, builds);
    } else {
        fprintf(stderr, "pattern not found\n");
    }

    curl_easy_cleanup(curl);
    free(buf.data);
    return 0;
}

