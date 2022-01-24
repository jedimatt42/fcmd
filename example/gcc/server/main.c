#include "fc_api.h"

volatile int vol_x = 0;

int bind_server_port() {
  // bind server
  int tries = 500;
  fc_tputs("binding port");
  while(tries) {
    char bind_request[15];
    bind_request[0] = 0x22;
    bind_request[1] = 0x00;
    bind_request[2] = 0x05;
    fc_strcpy(bind_request + 3, "0.0.0.0:9640");
    unsigned int len = 15;

    fc_tipi_sendmsg(len, bind_request);

    char bind_response;
    fc_tipi_recvmsg(&len, &bind_response);;

    if (bind_response == 0) {
      fc_tputs(".");
      tries--;
    } else {
      fc_tputs(" - complete\n");
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

  fc_tipi_sendmsg(3, accept_request);
  char client_id = 0;
  unsigned int len = 0;
  fc_tipi_recvmsg(&len, &client_id);
  return client_id;
}

void close_client(char client_id) {
  char close_request[3];
  close_request[0] = 0x22;
  close_request[1] = client_id;
  close_request[2] = 0x02;
  fc_tipi_sendmsg(3, close_request);
  // always receive after sending... in this case the value is always one byte
  unsigned int dummy = 0;
  fc_tipi_recvmsg(&dummy, (char*) &dummy);
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
  fc_strset(line, 0, 80);

  char read_request[5];
  read_request[0] = 0x22;
  read_request[1] = client_id;
  read_request[2] = 0x04;
  read_request[3] = 0;
  read_request[4] = 80;

  fc_tipi_sendmsg(5, read_request);

  unsigned int len = 0;
  fc_tipi_recvmsg(&len, line);

  // send something client unique back
  if (len) {
    fc_tputs("client ");
    fc_tputs(fc_uint2hex(client_id));
    fc_tputs(": ");
    fc_tputs(line);

    line[0] = 0x22;
    line[1] = client_id;
    line[2] = 0x03;

    fc_strcpy(line + 3, "your id is ");
    fc_strcpy(line + 14, fc_uint2hex(client_id));

    fc_tipi_sendmsg(fc_strlen(line), line);
    // we'll just re-use line here.. only 1 byte is returned.
    fc_tipi_recvmsg(&len, line);
    return line[0] == 0; // if it is zero, it is an error.
  } else {
    // send a null character just to verify the connection
    line[0] = 0x22;
    line[1] = client_id;
    line[2] = 0x03;
    line[3] = 0;
    fc_tipi_sendmsg(4, line);
    fc_tipi_recvmsg(&len, line);
    if (len == 1 && line[0] == 0) {
      return 1; // detected failure to write
    }
  }
  return 0;
}

int fc_main(char* args) {
  fc_tipi_on();
  int err = bind_server_port();
  if (err) {
    fc_tipi_off();
    return err;
  }

  int client_count = 0;
  char client_ids[10];
  // clear the list of active clients
  fc_strset(client_ids, 0, 10); 

  while(1) {
    // see if there are clients waiting to connect
    if (client_count < 10) {
      int client_id = server_accept();
      if (client_id == 0xff) {
        fc_tputs("server socket error\n");
        fc_tipi_off();
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

  fc_tipi_off();
  return 0;
}
