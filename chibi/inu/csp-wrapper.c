#include "stdlib.h"
#include "assert.h"
#include "pthread.h"
#include "unistd.h"

static pthread_mutex_t autoincrement_mutex = PTHREAD_MUTEX_INITIALIZER;
static int autoincremented = 0;
static int autoincrementing_id () {
  assert(pthread_mutex_lock(&autoincrement_mutex) == 0);

  int new_id = autoincremented;
  autoincremented++;

  assert(pthread_mutex_unlock(&autoincrement_mutex) == 0);

  return new_id;
}

static pthread_mutex_t spawn_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct spawn_element {
  char *data;
  struct spawn_element *prev;
  struct spawn_element *next;
} spawn_element;

typedef struct spawn_datum {
  int id;
  sexp callback;
  sexp parent_data;
  spawn_element *queue;
  sexp ctx;
  pthread_t *thread;
  pthread_mutex_t *mutex;
} spawn_datum;

sexp csp_spawn (sexp ctx, sexp self, sexp callback);
sexp csp_reclaim (sexp ctx, sexp self, sexp boxed_id);
sexp csp_channel_push (sexp ctx, sexp self, sexp boxed_id, sexp value);
sexp csp_channel_pop_non_block (sexp ctx, sexp self, sexp boxed_id);
sexp csp_channel_pop (sexp ctx, sexp self, sexp boxed_id);

void *spawn (void *param) {
  spawn_datum *data = (spawn_datum *)param;

  sexp ctx = sexp_make_eval_context(NULL, NULL, NULL, 0, 0);
  sexp_load_standard_ports(ctx, NULL, stdin, stdout, stderr, 0);

  // TODO: Find a workaround for this.
  sexp_eval_string(ctx, "(import (scheme small))", -1, NULL);

  sexp boxed_id = sexp_make_cpointer(ctx, SEXP_CPOINTER, data, SEXP_FALSE, 0);
  sexp_env_define(ctx, sexp_context_env(ctx), sexp_intern(ctx, "thread-id", 9), boxed_id);

  sexp boxed_parent_id = sexp_make_cpointer(ctx, SEXP_CPOINTER, data->parent_data, SEXP_FALSE, 0);

  sexp_apply(ctx, data->callback, sexp_list2(ctx, boxed_parent_id, boxed_id));

  data->ctx = ctx;

  return NULL;
}

sexp csp_spawn (sexp ctx, sexp self, sexp callback) {
  assert(pthread_mutex_lock(&spawn_mutex) == 0);

  sexp maybe_boxed_id = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "thread-id", 9), SEXP_NULL);
  sexp equal_function = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "equal?", 6), SEXP_NULL);
  sexp value = sexp_apply(ctx, equal_function, sexp_list2(ctx, maybe_boxed_id, SEXP_NULL));
  bool has_no_thread_id = sexp_unbox_boolean(value);

  if (has_no_thread_id == 1) {
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    spawn_datum *data = malloc(sizeof(spawn_datum));
    int id = autoincrementing_id();
    sexp boxed_id = sexp_make_cpointer(ctx, SEXP_CPOINTER, data, SEXP_FALSE, 0);
    data->id = id;
    data->callback = NULL;
    data->parent_data = NULL;
    data->queue = NULL;

    assert(pthread_mutex_init(mutex, NULL) == 0);

    data->thread = NULL;
    data->mutex = mutex;

    // TODO: Find a workaround for this.
    sexp_eval_string(ctx, "(import (scheme small))", -1, NULL);

    sexp_env_define(ctx, sexp_context_env(ctx), sexp_intern(ctx, "thread-id", 9), boxed_id);
  }

  assert(pthread_mutex_unlock(&spawn_mutex) == 0);

  pthread_t *thread = malloc(sizeof(pthread_t));
  pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
  spawn_datum *data = malloc(sizeof(spawn_datum));
  sexp_preserve_object(ctx, callback);
  int id = autoincrementing_id();
  sexp boxed_id = sexp_make_cpointer(ctx, SEXP_CPOINTER, data, SEXP_FALSE, 0);
  data->id = id;
  data->callback = callback;

  sexp parent_boxed_id = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "thread-id", 9), SEXP_NULL);
  data->parent_data = sexp_cpointer_value(parent_boxed_id);
  data->queue = NULL;

  assert(pthread_create(thread, NULL, spawn, data) == 0);
  assert(pthread_mutex_init(mutex, NULL) == 0);

  data->thread = thread;
  data->mutex = mutex;

  return boxed_id;
}

sexp csp_reclaim (sexp ctx, sexp self, sexp boxed_id) {
  spawn_datum *data = sexp_cpointer_value(boxed_id);
  pthread_t *thread = data->thread;

  assert(pthread_join(*thread, NULL) == 0);

  sexp_release_object(ctx, data->callback);
  // ...
  free(data->mutex);
  free(data->thread);
  free(data);

  return SEXP_NULL;
}

// TODO: Serialize and deserialize data going to queues.
// TODO: Queues may need to live in an associative array of queues outside of other VMs.
// TODO: Don't forget to clean up the queue when done.

sexp csp_channel_push (sexp ctx, sexp self, sexp boxed_id, sexp value) {
  printf("wtf a\n");
  spawn_datum *data = sexp_cpointer_value(boxed_id);
  pthread_mutex_t *mutex = data->mutex;

  assert(pthread_mutex_lock(mutex) == 0);

  sexp output_string = sexp_eval_string(ctx, "(open-output-string)", -1, NULL);

  sexp write_function = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "write", 5), SEXP_NULL);
  sexp_apply(ctx, write_function, sexp_list2(ctx, value, output_string));

  sexp get_output_string_function = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "get-output-string", 17), SEXP_NULL);
  sexp string = sexp_apply(ctx, get_output_string_function, sexp_list1(ctx, output_string));

  sexp display_function = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "display", 7), SEXP_NULL);
  printf("pushing \"");
  sexp_apply(ctx, display_function, sexp_list1(ctx, string));
  printf("\"\n");

  char *cstring = sexp_string_data(string);

  spawn_element *queue = data->queue;
  spawn_element *element = malloc(sizeof(spawn_element));
  element->data = cstring;
  DL_APPEND(queue, element);
  data->queue = queue;

  assert(pthread_mutex_unlock(mutex) == 0);

  return SEXP_NULL;
}

sexp csp_channel_pop_non_block (sexp ctx, sexp self, sexp boxed_id) {
  printf("wtf b\n");
  spawn_datum *data = sexp_cpointer_value(boxed_id);
  pthread_mutex_t *mutex = data->mutex;

  assert(pthread_mutex_lock(mutex) == 0);

  spawn_element *queue = data->queue;
  if (queue == NULL) {
    printf("bailed\n");
    return SEXP_NULL;
  }

  char *cstring = queue->data;
  printf("popped \"%s\"\n", cstring);
  sexp string = sexp_c_string(ctx, cstring, strlen(cstring));
  sexp open_input_string_function = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "open-input-string", 17), SEXP_NULL);
  sexp input_string = sexp_apply(ctx, open_input_string_function, string);

  sexp read_function = sexp_env_ref(ctx, sexp_context_env(ctx), sexp_intern(ctx, "read", 4), SEXP_NULL);
  sexp value = sexp_apply(ctx, read_function, sexp_list1(ctx, input_string));

  assert(pthread_mutex_unlock(mutex) == 0);

  /* spawn_element *next = queue->next; */
  /* #<{(| DL_DELETE(data->queue, queue); |)}># */
  /* #<{(| free(queue); |)}># */
  /* data->queue = next; */

  return value;
}

sexp csp_usleep (sexp ctx, sexp self, sexp t) {
  usleep(sexp_unbox_fixnum(t));
  return SEXP_NULL;
}

sexp csp_sleep (sexp ctx, sexp self, sexp t) {
  sleep(sexp_unbox_fixnum(t));
  return SEXP_NULL;
}
