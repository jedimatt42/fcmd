#include "banks.h"
#define MYBANK BANK(7)

#include "pipeline.h"
#include "redirect.h"
#include "parsing.h"
#include "globals.h"
#include "variables.h"
#include "strutil.h"
#include "terminal.h"

static char pipe_prefix[REDIR_PATH_MAX];

static char* find_pipe(char* s) {
  int escaped = 0;
  while (*s) {
    if (!escaped && *s == '\\') {
      escaped = 1;
      s++;
      continue;
    }
    if (!escaped && *s == '|') {
      return s;
    }
    escaped = 0;
    s++;
  }
  return 0;
}

static void unescape_pipes(char* s) {
  char* read = s;
  char* write = s;
  while (*read) {
    if (*read == '\\' && read[1] == '|') {
      *write++ = '|';
      read += 2;
    } else {
      *write++ = *read++;
    }
  }
  *write = 0;
}

static int has_redirect_operator(const char* s) {
  int escaped = 0;
  while (*s) {
    if (!escaped && *s == '\\') {
      escaped = 1;
      s++;
      continue;
    }
    if (!escaped && *s == '>') {
      return 1;
    }
    escaped = 0;
    s++;
  }
  return 0;
}

static int is_blank_stage(const char* s) {
  while (*s == ' ') {
    s++;
  }
  return *s == 0;
}

static int prepare_pipe_prefix(void) {
  char* env = bk_vars_get(str2ram("FCTMP"));
  if ((int) env == -1 || env[0] == 0) {
    bk_strcpy(pipe_prefix, currentPath);
  } else {
    bk_strcpy(pipe_prefix, env);
  }

  int len = bk_strlen(pipe_prefix);
  while (len > 0 && pipe_prefix[len - 1] == ' ') {
    len--;
    pipe_prefix[len] = 0;
  }
  if (len == 0) {
    bk_strcpy(pipe_prefix, currentPath);
    len = bk_strlen(pipe_prefix);
  }
  if (len >= REDIR_PATH_MAX - 1) {
    return -1;
  }
  if (len == 0 || pipe_prefix[len - 1] != '.') {
    pipe_prefix[len] = '.';
    pipe_prefix[len + 1] = 0;
  }

  /*
   * Resolve the prefix now, so a cd performed by an earlier stage cannot move
   * later temporary files. Use a dummy filename and strip it afterwards.
   */
  char probe[REDIR_PATH_MAX];
  char resolved[REDIR_PATH_MAX];
  struct DeviceServiceRoutine* dsr;

  if (bk_strlen(pipe_prefix) + 7 >= REDIR_PATH_MAX) {
    return -1;
  }
  bk_strcpy(probe, pipe_prefix);
  bk_strcat(probe, str2ram("FCPIPEX"));
  bk_parsePathParam(probe, &dsr, resolved, PR_REQUIRED);
  if (dsr == 0) {
    return -1;
  }
  len = bk_strlen(resolved);
  if (len < 7) {
    return -1;
  }
  len -= 7;
  resolved[len] = 0;
  bk_strcpy(pipe_prefix, resolved);

  return 0;
}

static int build_temp_path(char* out, int slot) {
  char suffix[8] = "FCPIPE0";
  suffix[6] = slot ? '1' : '0';

  int plen = bk_strlen(pipe_prefix);
  if (plen + 7 > MAX_VAR_VAL) {
    return -1;
  }
  bk_strcpy(out, pipe_prefix);
  bk_strcat(out, suffix);
  return 0;
}

static int set_pipe_input(const char* path) {
  if (path == 0 || path[0] == 0) {
    char empty[1];
    empty[0] = 0;
    bk_vars_set(str2ram("PIPEIN"), empty);
    return 0;
  }
  if (bk_strlen((char*) path) > MAX_VAR_VAL) {
    return -1;
  }
  bk_vars_set(str2ram("PIPEIN"), (char*) path);
  return 0;
}

static int delete_pipe_temp(const char* path) {
  if (path == 0 || path[0] == 0) {
    return 0;
  }

  char raw[REDIR_PATH_MAX];
  char resolved[REDIR_PATH_MAX];
  struct DeviceServiceRoutine* dsr;

  bk_strncpy(raw, (char*) path, REDIR_PATH_MAX - 1);
  bk_parsePathParam(raw, &dsr, resolved, PR_REQUIRED);
  if (dsr == 0) {
    return -1;
  }

  struct PAB pab;
  bk_initPab(&pab);
  pab.pName = (unsigned char*) resolved;
  int err = bk_dsr_delete(dsr, &pab);
  if (err && bk_existsFile(dsr, resolved)) {
    tputs_rom("warning, could not remove temporary file ");
    bk_tputs_ram(resolved);
    bk_tputc('\n');
  }
  return err;
}

int runPipeline(char* buffer) {
  char pipe_in[MAX_VAR_VAL + 1];
  char pipe_out[MAX_VAR_VAL + 1];
  int first_error = 0;
  int stage_no = 0;
  char* stage = buffer;

  pipe_in[0] = 0;
  pipe_out[0] = 0;

  if (redirect_state.open) {
    tputs_rom("error, pipelines cannot run inside an active redirection\n");
    setstrtok(0);
    bk_setCommandStatus(1);
    return 1;
  }

  set_pipe_input(0);

  if (request_break) {
    setstrtok(0);
    bk_setCommandStatus(1);
    return 1;
  }

  if (prepare_pipe_prefix() < 0) {
    tputs_rom("error, invalid or too-long temporary path: ");
    bk_tputs_ram(pipe_prefix);
    bk_tputc('\n');
    setstrtok(0);
    bk_setCommandStatus(1);
    return 1;
  }
  if (bk_strlen(pipe_prefix) + 7 > MAX_VAR_VAL) {
    tputs_rom("error, temporary path is too long for PIPEIN: ");
    bk_tputs_ram(pipe_prefix);
    bk_tputc('\n');
    setstrtok(0);
    bk_setCommandStatus(1);
    return 1;
  }

  while (stage != 0) {
    while (*stage == ' ') {
      stage++;
    }

    char* pipe = find_pipe(stage);
    char* next = 0;
    if (pipe != 0) {
      *pipe = 0;
      next = pipe + 1;
    }
    unescape_pipes(stage);

    if (is_blank_stage(stage)) {
      tputs_rom("error, empty pipeline stage\n");
      if (first_error == 0) {
        first_error = 1;
      }
      break;
    }

    int is_final = (next == 0);

    if (!is_final) {
      if (has_redirect_operator(stage)) {
        tputs_rom("error, redirection is only supported on the last pipeline stage\n");
        if (first_error == 0) {
          first_error = 1;
        }
        break;
      }
      /*
       * Two slots are enough because stages are sequential: the next stage
       * consumes the previous slot, and that file is deleted before a later
       * stage reuses the slot. Stage 0 uses FCPIPE0, stage 1 FCPIPE1, stage 2
       * FCPIPE0 again, and so on.
       */
      if (build_temp_path(pipe_out, stage_no & 1) < 0) {
        tputs_rom("error, temporary path is too long\n");
        if (first_error == 0) {
          first_error = 1;
        }
        break;
      }
    } else {
      pipe_out[0] = 0;
    }

    if (!is_final && redirect_state.open) {
      tputs_rom("error, pipelines cannot run inside an active redirection\n");
      if (first_error == 0) {
        first_error = 1;
      }
      break;
    }

    if (set_pipe_input(pipe_in) < 0) {
      tputs_rom("error, temporary path is too long for PIPEIN\n");
      if (first_error == 0) {
        first_error = 1;
      }
      break;
    }

    int fatal = 0;
    int stage_status = bk_executeStage(stage, is_final ? 0 : pipe_out, &fatal);
    if (first_error == 0 && stage_status != 0) {
      first_error = stage_status;
    }
    if (fatal && first_error == 0) {
      first_error = 1;
    }

    if (!is_final && redirect_state.path[0] != 0) {
      if (bk_strlen(redirect_state.path) > MAX_VAR_VAL) {
        tputs_rom("error, temporary path is too long for PIPEIN\n");
        if (first_error == 0) {
          first_error = 1;
        }
        delete_pipe_temp(redirect_state.path);
        pipe_out[0] = 0;
        fatal = 1;
      } else {
        bk_strcpy(pipe_out, redirect_state.path);
      }
    }

    if (pipe_in[0] != 0) {
      delete_pipe_temp(pipe_in);
      pipe_in[0] = 0;
    }

    if (fatal) {
      if (!is_final && pipe_out[0] != 0) {
        delete_pipe_temp(pipe_out);
        pipe_out[0] = 0;
      }
      break;
    }

    if (request_break) {
      if (first_error == 0) {
        first_error = 1;
      }
      if (!is_final && pipe_out[0] != 0) {
        delete_pipe_temp(pipe_out);
        pipe_out[0] = 0;
      }
      break;
    }

    if (!is_final && pipe_out[0] != 0) {
      bk_strcpy(pipe_in, pipe_out);
      pipe_out[0] = 0;
    }

    stage = next;
    stage_no++;
  }

  if (pipe_in[0] != 0) {
    delete_pipe_temp(pipe_in);
    pipe_in[0] = 0;
  }
  if (pipe_out[0] != 0) {
    delete_pipe_temp(pipe_out);
    pipe_out[0] = 0;
  }

  set_pipe_input(0);
  setstrtok(0);
  bk_setCommandStatus(first_error);
  return first_error;
}
