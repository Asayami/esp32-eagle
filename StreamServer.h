//
// Created by Simone on 23/06/2022.
//

#ifndef ELOQUENTSURVEILLANCE_STREAMSERVER
#define ELOQUENTSURVEILLANCE_STREAMSERVER


#include <WiFi.h>
#include <esp_camera.h>
#include <esp_http_server.h>
#include "./globals.h"
#include "./traits/HasErrorMessage.h"
#include "index.h"

#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN  (64)
namespace EloquentSurveillance {
    /**
     * ESP32 camera web server
     */
    class StreamServer : public HasErrorMessage {
    public:
        /**
         *
         * @param port
         */
        StreamServer(uint16_t port = 80) :
            _port(port) {
            setErrorMessage("");
            _config = HTTPD_DEFAULT_CONFIG();
            _config.server_port = port;
        }

        /**
        *
        * @return
        */
        bool begin() {
            if (httpd_start(&_httpd, &_config) != ESP_OK)
                return setErrorMessage("Cannot start HTTP server");

            httpd_uri_t stream_uri = {
                    .uri       = "/stream",
                    .method    = HTTP_GET,
                    .handler   = [](httpd_req_t *req) {
                        esp_err_t res = ESP_OK;
                        char *part[64];

                        if (httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=123456789000000000000987654321") != ESP_OK)
                        {
                          setSensorData(5,0);
                          return ESP_FAIL;
                        }
                        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
						
                        setSensorData(5,1);
                        delay(5000);
                        while (true) {
                            size_t contentTypeHeaderLength;

                            if (!gCapture()) {
                                delay(1);
                                continue;
                            }

                            if (!gIsFrame())
                                continue;

                            contentTypeHeaderLength = snprintf((char *) part, 64,
                                                               "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                                                               gFrame->len);

                            if (httpd_resp_send_chunk(req, (const char *) part, contentTypeHeaderLength) != ESP_OK)
                            {
                              setSensorData(5,0);
                                              return ESP_FAIL;
                            }
                            if (httpd_resp_send_chunk(req, (const char *) gFrame->buf, gFrame->len) != ESP_OK) {
                                esp_camera_fb_return(gFrame);
								                setSensorData(5,0);
                                return ESP_FAIL;
                            }

                            res = httpd_resp_send_chunk(req, "\r\n--123456789000000000000987654321\r\n", 37);
                        }
						
						            setSensorData(5,0);

                        return ESP_OK;
                    },
                    .user_ctx  = NULL
            };
			
			
            httpd_uri_t html_uri = {
              .uri       = "/",
              .method    = HTTP_GET,
              .handler   = [](httpd_req_t *req) {
                const char* html_content = index_html;
                size_t html_content_len = sizeof(index_html) - 1;
                
                httpd_resp_set_type(req, "text/html");
                httpd_resp_send(req, html_content, html_content_len);

                return ESP_OK;
              },
              .user_ctx  = NULL
            };
			
            httpd_uri_t set_sensor_uri = {
              .uri       = "/set-sensor",
              .method    = HTTP_GET,
              .handler   = [](httpd_req_t *req) {
                char*  buf;
                size_t buf_len;
                char id[32] = {0,};
                char value[32] = {0,};

                buf_len = httpd_req_get_url_query_len(req) + 1;
                if (buf_len > 1) {
                  buf = (char*)malloc(buf_len);
                  if(!buf){
                    httpd_resp_send_500(req);
                    return ESP_FAIL;
                  }
                  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
                    if (httpd_query_key_value(buf, "id", id, sizeof(id)) == ESP_OK &&
                      httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
                    } else {
                      free(buf);
                      httpd_resp_send_404(req);
                      return ESP_FAIL;
                    }
                  } else {
                    free(buf);
                    httpd_resp_send_404(req);
                    return ESP_FAIL;
                  }
                  free(buf);
                } else {
                  httpd_resp_send_404(req);
                  return ESP_FAIL;
                }
                int val = atoi(value);
                int id_num = atoi(id);
                sensorPtr(id_num, val);
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                return httpd_resp_send(req, NULL, 0);
              },
              .user_ctx  = NULL
            };
			
            httpd_uri_t get_sensor_uri = {
              .uri       = "/get-sensor",
              .method    = HTTP_GET,
              .handler   = [](httpd_req_t *req) {
                static char json_response[1024];
                char * p = json_response;
                *p++ = '{';
                p+=sprintf(p, "\"1\":%d,", getSensorData(1));
                p+=sprintf(p, "\"2\":%d,", getSensorData(2));
                p+=sprintf(p, "\"3\":%d,", getSensorData(3));
                p+=sprintf(p, "\"4\":%d,", getSensorData(4));
                p+=sprintf(p, "\"5\":%d,", getSensorData(5));
                p+=sprintf(p, "\"6\":%d", getSensorData(6));
                *p++ = '}';
                *p++ = 0;
                httpd_resp_set_type(req, "application/json");
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                return httpd_resp_send(req, json_response, strlen(json_response));
              },
              .user_ctx  = NULL
            };
			
			
			
      httpd_register_uri_handler(_httpd, &stream_uri);
			httpd_register_uri_handler(_httpd, &html_uri);
			httpd_register_uri_handler(_httpd, &set_sensor_uri);
			httpd_register_uri_handler(_httpd, &get_sensor_uri);
			setSensorData(5,0);
            return true;
        }
		
		


        /**
         *
         * @return
         */
        String getWelcomeMessage() {
            String ip = wifi.getIP();

            if (_port != 80) {
                ip += ':';
                ip += _port;
            }

            return
                String(F("StreamServer listening at http://"))
                + ip
                + String(F(".\nMJPEG stream is available at "))
                + ip
                + "/stream";
        }

    protected:
        bool _isClient;
        uint16_t _port;
        httpd_config_t _config;
        httpd_handle_t _httpd;
    };
}

#endif
