#include "fc_api.h"

volatile int vol_x = 0;

int bind_server_port() {
  // bind server
  int tries = 500;
  term_puts("binding port");
  while(tries) {
    char bind_request[15];
    bind_request[0] = 0x22;
    bind_request[1] = 0x00;
    bind_request[2] = 0x05;
    str_copy(bind_request + 3, "0.0.0.0:9640");
    unsigned int len = 15;

    tipi_on();
    tipi_sendmsg(len, bind_request);

    char bind_response;
    tipi_recvmsg(&len, &bind_response);;
    tipi_off();

    if (bind_response == 0) {
      term_puts(".");
      tries--;
    } else {
      term_puts(" - complete\n");
      return 0;
    }
    for(unsigned int w=0;w < 0xA000; w++) {
      vol_x++;
    }
  }
  return 1;
}

char server_accept() {
  char accept_request[3];
  accept_request[0] = 0x22;
  accept_request[1] = 0x00;
  accept_request[2] = 0x07;

  tipi_on();
  tipi_sendmsg(3, accept_request);
  char client_id = 0;
  unsigned int len = 0;
  tipi_recvmsg(&len, &client_id);
  tipi_off();
  return client_id;
}

void close_client(char client_id) {
  char close_request[3];
  close_request[0] = 0x22;
  close_request[1] = client_id;
  close_request[2] = 0x02;
  tipi_on();
  tipi_sendmsg(3, close_request);
  // always receive after sending... in this case the value is always one byte
  unsigned int dummy = 0;
  tipi_recvmsg(&dummy, (char*) &dummy);
  tipi_off();
}

void remove_client(char* client_ids, int idx) {
  close_client(client_ids[idx]);

  // compact the list of clients... 
  while(idx < 9) {
    client_ids[idx] = client_ids[idx+1];
    idx++;
  }
  client_ids[9] = 0;
}

int handle_client(char client_id) {
  char line[80];
  str_set(line, 0, 80);

  char read_request[5];
  read_request[0] = 0x22;
  read_request[1] = client_id;
  read_request[2] = 0x04;
  read_request[3] = 0;
  read_request[4] = 80;

  tipi_on();
  tipi_sendmsg(5, read_request);

  unsigned int len = 0;
  tipi_recvmsg(&len, line);
  tipi_off();

  // send something client unique back
  if (len) {
    term_puts("client ");
    term_puts(hex_from_uint(client_id));
    term_puts(": ");
    term_puts(line);

    line[0] = 0x22;
    line[1] = client_id;
    line[2] = 0x03;

    str_copy(line + 3, "your id is ");
    str_copy(line + 14, hex_from_uint(client_id));

    tipi_on();
    tipi_sendmsg(str_len(line), line);
    // we'll just re-use line here.. only 1 byte is returned.
    tipi_recvmsg(&len, line);
    tipi_off();
    return line[0] == 0; // if it is zero, it is an error.
  } else {
    // send a null character just to verify the connection
    line[0] = 0x22;
    line[1] = client_id;
    line[2] = 0x03;
    line[3] = 0;
    tipi_on();
    tipi_sendmsg(4, line);
    tipi_recvmsg(&len, line);
    tipi_off();
    if (len == 1 && line[0] == 0) {
      return 1; // detected failure to write
    }
  }
  return 0;
}

int main(char* args) {
  int err = bind_server_port();
  if (err) {
    return err;
  }

  int client_count = 0;
  char client_ids[10];
  // clear the list of active clients
  str_set(client_ids, 0, 10); 

  while(1) {
    // see if there are clients waiting to connect
    if (client_count < 10) {
      int client_id = server_accept();
      if (client_id == 0xff) {
        term_puts("server socket error\n");
        return 1;
      }    
      if (client_id != 0) {
        client_ids[client_count++] = client_id;
      }
    }

    // if we have any clients, interact with each of them.
    // - this isn't a perfectly fair loop, but does it need to be?
    for(int c=0; c < client_count; c++) {
      char client_id = client_ids[c];
      if (client_id != 0 && handle_client(client_id)) {
	remove_client(client_ids, c);
      }
    }
  }

  return 0;
}
