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

char* to_char_arr(std::string s) {
    const int length = s.length();
    char* char_arr = new char[length + 1];
    strcpy(char_arr, s.c_str());
    return char_arr;
}

bool download_image(char* url,std::string file_name)
{
    FILE* fp = fopen(to_char_arr(file_name), "wb");
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

std::string extract_html(std::string s) { 
    cpr::Url adres = cpr::Url{s};
    cpr::Response odpowiedz = cpr::Get(adres);
    return odpowiedz.text;
}


void find_images(GumboNode * node, std::vector<std::string> &image_sources) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    if (node->v.element.tag == GUMBO_TAG_A) {

        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href"); 

        if (href) {
            std::string link = href->value;
            if (link.rfind(".png") != std::string::npos || link.rfind(".jpg") != std::string::npos) {
                link = "https:" + link;
                if (std::find(image_sources.begin(), image_sources.end(), link) == image_sources.end()) {
                    image_sources.push_back(link);
                }
            }
        }
        return;
    }


    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; i++) {
        find_images(static_cast<GumboNode*>(children->data[i]),image_sources);
    }
}

void download_images(std::vector<std::string> img_sources) {
    for (int i = 0; i < img_sources.size() ; i++) {
        std::string file_name = "F:\\imgs\\img"+std::to_string(i)+".png"; // example folder
        char* url = to_char_arr(img_sources[i]);

        if (!download_image(&url[0],file_name)) {
            printf("!! Failed to download file: %s\n", &url[0]);
            return;
        }

        delete[] url;
    }
}


int main(){


    std::string content = extract_html(""); // site url goes here

    GumboOutput* content_gumbo = gumbo_parse(content.c_str());

    std::vector<std::string> image_sources;

    find_images(content_gumbo->root,image_sources);

    download_images(image_sources);

    gumbo_destroy_output(&kGumboDefaultOptions, content_gumbo);

    return 0;
}