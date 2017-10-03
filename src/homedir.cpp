#include "homedir.h"
#include "filesystem.h"
#include "results.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <map>
#include <string>

#define SETTINGS_MAX (PATH_MAX*2)

using namespace std;

static char homepath[PATH_MAX];
static map<string, string> settings;

char const *get_homedir() {
    if (!homepath[0]) {
        char const *e = getenv("CLIDEHOME");
        if (e) {
            if (fs_concat(homepath, sizeof(homepath), e, "")) {
                if (access(homepath, R_OK|W_OK|X_OK) < 0) {
                    result("CLIDEHOME environment variable is not accessible", StatusError);
                    homepath[0] = 0;
                }
            } else {
                result("CLIDEHOME path is too long", StatusError);
                homepath[0] = 0;
            }
        }
    }
    if (!homepath[0]) {
        char const *e = getenv("HOME");
        if (e) {
            if (fs_concat(homepath, sizeof(homepath), e, ".clide")) {
                //  always attempt to create it, but check failures 
                //  only with access()
                mkdir(homepath, 0750);
                if (access(homepath, R_OK|W_OK|X_OK) < 0) {
                    result("HOME .clide is not accessible", StatusError);
                    homepath[0] = 0;
                }
            } else {
                result("HOME .clide path is too long", StatusError);
                homepath[0] = 0;
            }
        }
    }
    if (!homepath[0]) {
        result("Defaulting to '.' for homepath", StatusWarning);
        strcpy(homepath, ".");
    }
    return homepath;
}

static bool bad_setting_name(std::string const &name) {
    if (!name.size()) {
        return true;
    }
    for (auto const p : name) {
        if (((p >= 0) && (p <= 32))
            || (p == '#')
            || (p == '=')) {
            return true;
        }
    }
    return false;
}

static bool bad_setting_value(std::string const &value) {
    for (auto const p : value) {
        if ((p >= 0) && (p < 32)) {
            return true;
        }
    }
    return false;
}

bool read_settings(char const *name) {
    char setfile[PATH_MAX];
    settings.clear();
    bool ok = false;
    if (fs_concat(setfile, sizeof(setfile), get_homedir(), name)) {
        FILE *f = fopen(setfile, "rb");
        if (f) {
            char line[SETTINGS_MAX];
            ok = true;
            while (fgets(line, SETTINGS_MAX, f)) {
                if (line[0] == '\n' || line[0] == '#') {
                    continue;
                }
                char *end = &line[strlen(line)];
                if (end != line && end[-1] == '\n') {
                    end[-1] = 0;
                }
                char *eq = strchr(line, '=');
                if (eq) {
                    std::string key(line, eq);
                    std::string value(eq+1);
                    if (!bad_setting_name(key) && !bad_setting_value(value)) {
                        settings[key] = value;
                    } else {
                        result("settings file contains bad key", StatusWarning);
                        ok = false;
                    }
                } else {
                    result("settings file contains line without value", StatusWarning);
                    ok = false;
                }
            }
            fclose(f);
        } else {
            result("settings file not found", StatusError);
        }
    } else {
        result("settings file path too long", StatusError);
    }
    return ok;
}

bool save_settings(char const *name) {
    char pname[PATH_MAX];
    std::string tmp(name);
    tmp += "_";
    bool ok = false;
    if (fs_concat(pname, PATH_MAX, get_homedir(), tmp.c_str())) {
        tmp = pname;
        fs_concat(pname, PATH_MAX, get_homedir(), name);
        FILE *f = fopen(tmp.c_str(), "wb");
        if (f) {
            for (auto const &kv : settings) {
                fprintf(f, "%s=%s\n", kv.first.c_str(), kv.second.c_str());
            }
            fclose(f);
            if (0 == rename(tmp.c_str(), pname)) {
                ok = true;
            } else {
                result("Could not rename temporary file when saving settings", StatusError);
            }
        } else {
            result("Could not create temporary file when saving settings", StatusError);
        }
    } else {
        result("Settings file path is too long when saving settings", StatusError);
    }
    return ok;
}

bool set_setting(char const *name, char const *value) {
    std::string sn(name);
    if (!name || bad_setting_name(sn)) {
        result("Attempt to set setting with bad name", StatusError);
        return false;
    }
    std::string sv(value);
    if (bad_setting_value(value)) {
        result("Attempt to set setting with bad value", StatusError);
        return false;
    }
    settings[sn] = sv;
    return true;
}

char const *get_setting(char const *name, char const *dflt) {
    auto ptr = settings.find(name);
    if (ptr == settings.end()) {
        result("setting not found", StatusInfo);
        return dflt;
    }
    return (*ptr).second.c_str();
}

bool get_setting_long(char const *name, long *oval, long dflt) {
    auto ptr = settings.find(name);
    if (ptr == settings.end()) {
        result("setting not found", StatusInfo);
        *oval = dflt;
        return false;
    }
    char *oot = NULL;
    *oval = strtol((*ptr).second.c_str(), &oot, 10);
    if (!oot || oot == (*ptr).second.c_str()) {
        *oval = dflt;
        result("setting is not a valid integer", StatusWarning);
        return false;
    }
    return true;
}



