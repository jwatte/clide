#if !defined(clide_homedir_h)
#define clide_homedir_h

char const *get_homedir();
bool read_settings(char const *name);
bool save_settings(char const *name);
bool set_setting(char const *name, char const *value);
char const *get_setting(char const *name, char const *dflt);
bool get_setting_long(char const *name, long *oval, long dflt);

#endif  //  clide_homedir_h
