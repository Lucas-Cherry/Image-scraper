#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <cpr/cpr.h>
#include <gumbo.h>

size_t callbackfunction(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    FILE* stream = (FILE*)userdata;
    if (!stream)
    {
        printf("!!! No stream\n");
        return 0;
    }

    size_t written = fwrite((FILE*)ptr, size, nmemb, stream);
    return written;
}

bool download_jpeg(char* url)
{
    FILE* fp = fopen("out.png", "wb");
    if (!fp)
    {
        printf("!!! Failed to create file on the disk\n");
        return false;
    }

    CURL* curlCtx = curl_easy_init();
    curl_easy_setopt(curlCtx, CURLOPT_URL, url);
    curl_easy_setopt(curlCtx, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curlCtx, CURLOPT_WRITEFUNCTION, callbackfunction);
    curl_easy_setopt(curlCtx, CURLOPT_FOLLOWLOCATION, 1);

    CURLcode rc = curl_easy_perform(curlCtx);
    if (rc)
    {
        printf("!!! Failed to download: %s\n", url);
        return false;
    }

    long res_code = 0;
    curl_easy_getinfo(curlCtx, CURLINFO_RESPONSE_CODE, &res_code);
    if (!((res_code == 200 || res_code == 201) && rc != CURLE_ABORTED_BY_CALLBACK))
    {
        printf("!!! Response code: %d\n", res_code);
        return false;
    }

    curl_easy_cleanup(curlCtx);

    fclose(fp);

    return true;
}

//std::string extract_html() { 
//    cpr::Url adres = cpr::Url{ "" };
//    cpr::Response odpowiedz = cpr::Get(adres);
//    return odpowiedz.text;
//}

int main(){

    std::string s = ""; // site url goes here

    const int length = s.length();

    char* url = new char[length + 1];

    strcpy(url, s.c_str());

    if (!download_jpeg(&url[0]))
    {
        printf("!! Failed to download file: %s\n", &url[0]);
        return -1;
    }

    delete[] url;

    return 0;
}