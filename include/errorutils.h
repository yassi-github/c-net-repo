typedef char * error;

// create error.
error errors_new(char *text);

// compare error.
// return true if same else false
bool errors_is(error err, error target);
