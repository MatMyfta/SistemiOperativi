#include "../p/p.h"
#include "counter.h"

#define LOG_TAG "counter"
#include "../../containers/list.h"
#include "../../containers/tree.h"
#include "../../containers/dictionary.h"
#include "../../logger.h"
#include "../../path_node.h"
#include "../../protocol.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct counter_state {
  list *p_list;
  unitnos_dictionary *paths;
  unsigned int n;
  unsigned int m;
};

static int compare_key (const void* key1, const void* key2, void* dictionary) {
  int ret_value;
  ret_value=strcmp((char*)key1, (char*)key2);
  if (ret_value>0)
    ret_value=1;
  else if (ret_value<0)
    ret_value=-1;
  return ret_value;
}

static void destroy_key (void* key, void* dictionary) {
  free((char*) key);
  key=NULL;
}

static void destroy_value (void* value, void* dictionary) {
  remove_node(value);
  value=NULL;
}

static bool print_dictionary_node (void *key, void *value, void *user_data) {
  printf("[Path: %s]:\n", (char*) key);
  print_node(value);
  return false;
}

static void set_n(struct counter_state *state) {
  int p_cnt_diff = state->n - list_size(state->p_list);
  unitnos_p *p;
  if (p_cnt_diff > 0) {
    log_debug("Creating %d \"p\"", p_cnt_diff);
    // create new p
    while (p_cnt_diff--) {
      p = unitnos_p_create();
      if (p != NULL) {
        list_push_back(state->p_list, &p);
        if (state->m != 0) {
          unitnos_p_set_m(p, state->m);
        }
      }
    }
  } else if (p_cnt_diff < 0) {
    log_debug("Destroying %d \"p\"", -p_cnt_diff);
    unitnos_p **p;
    // remove p
    while (p_cnt_diff++) {
      p = list_back(state->p_list);
      unitnos_p_destroy(*p);
      list_pop_back(state->p_list);
    }
  }
}

static void set_m(struct counter_state *state) {
  list_node *node;
  unitnos_p **p;
  log_debug("Updating m of %lu \"p\"", list_size(state->p_list));
  list_for_each_data(p, unitnos_p *, node, state->p_list) {
    unitnos_p_set_m(*p, state->m);
  }
}

static void add_new_path(struct counter_state *state, const char *new_path) {
  list_node *node;
  unitnos_p **p;
  list_for_each_data(p, unitnos_p *, node, state->p_list) {
    unitnos_p_add_new_file(*p, new_path);
  }
}

int unitnos_counter_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  FILE *fin = fdopen(in_pipe, "r");

  struct counter_state state = {0};
  state.p_list = list_create(sizeof(unitnos_p *));
  state.paths = unitnos_dictionary_create(compare_key,*destroy_key, *destroy_value, NULL);

  char *message = NULL;
  size_t message_size = 0;

  while (1) {
    if (getline(&message, &message_size, fin) >= 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_SET_N)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        log_verbose("Received n: %u", n);
        state.n = n;
        set_n(&state);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        log_verbose("Received m: %u", m);
        state.m = m;
        set_m(&state);
      }

      /* Persistono errori riguardo l'aggiunta consecutiva di più path.
         Il comportamento è strano: command_value del secondo comando di
         add_new_path rimane del valore della precedente chiamata. Viene inoltre
         stampato, ma non risulta esserci alcuna funzione di stampa.
      */

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_ADD_NEW_PATH)) {
        log_verbose("Received path: %s", command.value);

        char *path = malloc(strlen(command.value)+1);
        strcpy(path, command.value);
        unitnos_path_node* tmp= create_path_node();
        if (tmp!=NULL) {
          if ((fill_path_node(tmp, path)) == 0)
            unitnos_dictionary_insert(state.paths, path, tmp);
          else
            log_verbose("Error on filling path: %s\n", path);
        }
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_LIST_PATHS)) {
        unitnos_dictionary_foreach(state.paths, print_dictionary_node, NULL);
        printf("Path memorizzati: %ld", unitnos_dictionary_size(state.paths));
      }
      log_verbose("Terminated command: %s", command.command);
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      break;
    }
  }

  return 0;
}
