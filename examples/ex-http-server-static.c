/* ----------------------------------------------------------------------------
  Copyright (c) 2018, Microsoft Research, Daan Leijen
  This is free software; you can redistribute it and/or modify it under the
  terms of the Apache License, Version 2.0. A copy of the License can be
  found in the file "license.txt" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include <nodec.h>
#include "examples.h"

/*-----------------------------------------------------------------
  Example of a static http server
-----------------------------------------------------------------*/
static const char* crt = 
-----BEGIN CERTIFICATE-----
MIIDsTCCApmgAwIBAgIJAJ25oPl2dHMYMA0GCSqGSIb3DQEBCwUAMG8xCzAJBgNV
BAYTAlVTMRMwEQYDVQQIDApXYXNoaW5ndG9uMRAwDgYDVQQHDAdSZWRtb25kMRIw
EAYDVQQKDAlNaWNyb3NvZnQxETAPBgNVBAsMCFJlc2VhcmNoMRIwEAYDVQQDDAls
b2NhbGhvc3QwHhcNMTgwODA5MTUxMTQ4WhcNMTkwODA5MTUxMTQ4WjBvMQswCQYD
VQQGEwJVUzETMBEGA1UECAwKV2FzaGluZ3RvbjEQMA4GA1UEBwwHUmVkbW9uZDES
MBAGA1UECgwJTWljcm9zb2Z0MREwDwYDVQQLDAhSZXNlYXJjaDESMBAGA1UEAwwJ
bG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtMWohvqt
lS2LWPzzWEbVocbTCkts1JbVxw1dSbxunzDpROeAIhcrhoady5AQ0OGib4OBxXK+
aPrfQuFPggM9T1XSGfo5oRgz5jZtcl+WYKzB//t7ZP1/mZFPPIs7rOlX3ToYXhe5
0Y/dcGA90RQsrV+zUua62JXRmh3yDniO/BctJbbCkpsXhukXzgthhlQt8TOuY9Tz
LXvUzD0SEW0ejFyVuRu9q7EdERJ+siA0LV7XKK2ocClrlagNJMqShV63BxjUBAYb
Ck7ioucADTnbnbmSlU7xCyrv6sZ/isnV2KToiWsh9XTgEcmvwsO4I8PumYCij2Z4
C3JGCDUGM892CQIDAQABo1AwTjAdBgNVHQ4EFgQUyb2qtTsL+LHv6JsZHpsUjTDe
ExcwHwYDVR0jBBgwFoAUyb2qtTsL+LHv6JsZHpsUjTDeExcwDAYDVR0TBAUwAwEB
/zANBgkqhkiG9w0BAQsFAAOCAQEAcVdPHp3xeWz9ufq/Cf8JK3XXykZBA1mreTGI
g3iaH17PZf+wA8fXRr3tkgvE2E+/jTz9hHUiYcMIFS+w4HqYsgJgpXA+LYcOL2RZ
SJX8BVj7MJpwzRW81+yVfNFOkj6FdCXxV/JNv77Xl4jK0FnsCs1903aJP+bnwirK
h14nWMXX4sqiNS823UuESQBBdRgeTUeVvZcF0LjX+Ql4RhfBtET21GsxyukchpxM
7ZNIruaBulHwDjakAwoOsxTXwbCxiVRKQGCun8D/ZykRKXigyShN2Dxzg20exyQw
2dqAUf7X78h7Md5wz5OaUJkLCfjcePOeVmFQGFIDI85BEJfABw==
-----END CERTIFICATE-----

static void http_serve() {
#ifndef NDEBUG
  fprintf(stderr,"\nrequest url: %s, content length: %llu\n", http_req_url(), (unsigned long long)http_req_content_length());
  http_req_print();
#endif
  http_serve_static( "./examples/web", NULL );
}

static void http_server() {
  const char* host = "127.0.0.1:8080";
  fprintf(stderr,"serving at: %s\n", host);
  async_http_server_at( host, NULL, &http_serve);
}

void ex_http_server_static() {
  async_stop_on_enter(&http_server);
}
