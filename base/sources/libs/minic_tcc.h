#pragma once

// fn takes and returns the types of the sig; a sig without arguments, like "f", registers the address of a global variable
void minic_tcc_register(const char *name, const char *sig, void *fn);
void minic_tcc_register_builtins(void); // Provided by the host, called before the first script
void minic_tcc_eval(char *text, char *name);
