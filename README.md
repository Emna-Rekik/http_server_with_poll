# http_server_with_poll

This program enables concurrent handling of client requests on http://localhost:8080/ using poll() I/O multiplexing system call. I have tested its performance by simulating 1000 client requests using the curl command and also by using the Apache benchmarking tool.

---
#### Compilation and test output
---
![Screenshot from 2023-06-19 11-16-37](https://github.com/Emna-Rekik/http_server_with_poll/assets/89909599/cf38288c-b781-4e54-8c4f-26ef41ce4836)

---
#### Apache Benchmarking tool
---
![Screenshot from 2023-06-19 11-00-40](https://github.com/Emna-Rekik/http_server_with_poll/assets/89909599/93c25c03-2f31-4805-8d16-41326dc8739a)
![Screenshot from 2023-06-19 11-00-44](https://github.com/Emna-Rekik/http_server_with_poll/assets/89909599/a88fe986-fc20-4559-9abf-e2a190a7c638)

### Reference :
https://beej.us/guide/bgnet/html/#poll
