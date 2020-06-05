#include "../p/p.h"
#include "counter.h"

#define LOG_TAG "counter"
#include "../../logger.h"
#include "../../protocol.h"
#include "../../path_node.h"
#include "../../containers/list.h"
#include "../../containers/tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct counter_state {
  list *p_list;
  unsigned int n;
  unsigned int m;
  unitnos_tree *paths;
};

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
  state.paths = unitnos_tree_create((*compare),(*remove_node));

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
         Il comportamento è strano: command_value del secondo comando di add_new_path rimane del valore della precedente chiamata.
         Viene inoltre stampato, ma non risulta esserci alcuna funzione di stampa.
      */
      
      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_ADD_NEW_PATH)) {
        log_verbose("Received path: %s", command.value);
        unitnos_path_node* tmp = create_path_node();

        if(fill_path_node(tmp, command.value)){
          //print_node(tmp);
          ////remove_node(tmp);
          unitnos_tree_add_node(state.paths,tmp);
          add_new_path(&state, command.value);  // not sure
          //free(tmp); ---> ?????
        }
        else{
          log_error("<path> not valid");  
          free(tmp);
        }
        
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_LIST_PATHS)) {

        /*
        Temporaneamente stampo solo la lunghezza della lista di tutti i path
        */
        printf("%lu\n",list_size(state.p_list));
        // TODO procedura list paths.. on working
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      break;
    }
  }

  return 0;
}
