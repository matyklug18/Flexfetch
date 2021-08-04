//usr/bin/gcc flexfetch-nolib.c -IX11 -lX11 -lm -g -o flexfetch-nolib; exec ./flexfetch-nolib

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pwd.h>
#include <unistd.h>
#include <libgen.h>

#include <sys/utsname.h>
#include <sys/types.h>

#include "xlibwrap.h"

/*----------------*\
| HELPER FUNCTIONS |
\*----------------*/

char* read_grep(char* file, char* pattern, int offset, int cut) {
	FILE *fp;
	fp = fopen(file, "r");
	char* line = "";
	char* cmp = pattern;
	size_t len = 0;
	while(strstr(line, cmp) == NULL) {
		if(getline(&line, &len, fp) == -1)
			break;
	}

	if(strstr(line, cmp) == NULL) return "";
	line += strlen(pattern) + offset;
	line[strlen(line)-cut-1] = 0;
	char* rtn = malloc (sizeof (char) * strlen(line));
	strcpy(rtn, line);
	return rtn;
}

char* read_all(char* file) {
	FILE* fp = fopen(file, "r");
	char* line = NULL;
	char* lines = malloc(1);
	strcpy(lines, "");
	size_t len;
	while(getline(&line, &len, fp) != -1) {
		lines = realloc(lines, strlen(lines)+strlen(line));
		strcat(lines, line);
	}
	return lines;
}

char* read_grep_last_match_only(char* file, char* pattern) {
	FILE *fp;
	fp = fopen(file, "r");
	char* line_n = "";
	char line[256] = "";
	char* cmp = pattern;
	size_t len = 0;
	while(1) {
		if(getline(&line_n, &len, fp) == -1)
			break;

		if(strstr(line_n, cmp) != NULL)
			strcpy(line, line_n);
	}
	line[strlen(line)-1] = 0;
	char* rtn = malloc (sizeof (char) * strlen(line));
	strcpy(rtn, line);
	return rtn;
}

int get_ppid_from_pid(int pid) {
	char path[255] = "";
	char pid_str[255] = "";
	sprintf(pid_str, "%d", pid);
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/stat");
	char str[255] = "";
	strcpy(str, read_all(path));
  const char s[255] = " ";
	char *token = strtok(str, s);

	for(int i = 0; i < 3; i++) {
		token = strtok(NULL, s);
	}
	return atoi(token);
}

char* get_pid_name(int pid) {
	char path[255] = "";
	char pid_str[255] = "";
	sprintf(pid_str, "%d", pid);
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/comm");
	return read_all(path);
}

int get_term_pid(int pid) {
	char path[255] = "";
	char pid_str[255] = "";
	sprintf(pid_str, "%d", pid);
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/maps");
	FILE *fp;
	fp = fopen(path, "r");
	char* line = "";
	char* cmp = "/usr/share/fonts";
	size_t len = 0;
	while(strstr(line, cmp) == NULL) {
		if(getline(&line, &len, fp) == -1)
			break;
	}
	if(strstr(line, cmp) != NULL) {
		return pid;
	}
	else {
		return get_term_pid(get_ppid_from_pid(pid));
	}
}

/*------------------*\
| FETCHING FUNCTIONS |
\*------------------*/

char* fetch_wm_name() {
	Window root = DefaultRootWindow(xwrap_dpy);
	Window wm_check = prop_get_window(root, "_NET_SUPPORTING_WM_CHECK");
	char* name = prop_get_str(wm_check, "_NET_WM_NAME");
	return name;
}

char* fetch_comp_name() {
	int screen = XDefaultScreen(xwrap_dpy);
	char* prop_name = malloc((int)log10(screen + (screen == 0 ? 1 : 0))+1+12);
	sprintf(prop_name, "_NET_WM_CM_S%d", screen);
	Atom CM_SCREEN = XInternAtom(xwrap_dpy, prop_name, False);
	Window cm_screen_w = XGetSelectionOwner(xwrap_dpy, CM_SCREEN);
	free(prop_name);
	if(cm_screen_w == 0) {
		return "none";
	} else {
		return prop_get_str(cm_screen_w, "_NET_WM_NAME");
	}
}

char* fetch_shell() {
	struct passwd* passwd_entry = getpwuid(getuid());
	return basename(passwd_entry->pw_shell);
}

char* fetch_distro() {
	char* distro_name = read_grep("/etc/os-release", "PRETTY_NAME=", 1, 1);
	struct utsname uts_name;
	uname(&uts_name);
	char* arch = uts_name.machine;
	char* distro_line = malloc(strlen(distro_name)+strlen(arch));
	sprintf(distro_line, "%s %s", distro_name, arch);
	return distro_line;
}

char* fetch_kernel() {
	char* kernel = read_all("/proc/sys/kernel/osrelease");
	kernel[strlen(kernel)-1] = 0;
	return kernel;
}

char* fetch_system_name() {
	char* product_name = read_all("/sys/class/dmi/id/product_name");
	char* product_version = read_all("/sys/class/dmi/id/product_version");
	product_name[strlen(product_name)-1] = 0;
	product_version[strlen(product_version)-1] = 0;
	int length = strlen(product_name)+strlen(product_version)+1;
	char* buff = malloc(length);
	snprintf(buff, length, "%s %s", product_name, product_version);
	return buff;
}

char* fetch_terminal() {
	char* term_name = get_pid_name(get_term_pid(getppid()));
	term_name[strlen(term_name)-1] = 0;
	return term_name;
}

char* fetch_font() {
	char path[4096] = "";
	char pid_str[6] = "";
	sprintf(pid_str, "%d", get_term_pid(getppid()));
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/maps");
	char* str = malloc(4096);
	strcpy(str, basename(read_grep_last_match_only(path, "/usr/share/fonts")));
	str[strrchr(str, '.')-str]=0;
	return str;
}

char* fetch_gtk_theme() {
	char* home = getpwuid(getuid())->pw_dir;
	char* path_gtk_2 = malloc(11+strlen(home));
	strcpy(path_gtk_2, home);
	strcat(path_gtk_2, "/.gtkrc-2.0");
	return read_grep(path_gtk_2, "gtk-theme-name=", 1, 1);
}

/*----*\
| MAIN |
\*----*/

#define LOGO_OFFSET 2

int main(int argc, char* argv[]) {
	xwrap_init();
	char logo_name[4096];
	char* distro_id = read_grep("/etc/os-release", "ID=", 0, 0);
	strcpy(logo_name, "/usr/share/flexfetch/logos/");
	strcat(logo_name, distro_id);
	strcat(logo_name, ".logo");
	FILE* logo = fopen(logo_name, "r");
	if(logo == NULL) {
		printf("Logo file for %s not found! Make it at \e[96m/usr/share/flexfetch/logos/%s.logo\e[39m.\n", distro_id, distro_id);
		return 1;
	}
	char* line = NULL;
	ssize_t size;
	getline(&line, &size, logo);
	char* color = malloc(strlen(line)+3);
	sprintf(color, "\e[%dm", atoi(line));
	int logo_width = 0;
	bool file_read = false;
	bool run = true;
	printf("\n");
	for(int i = 0; run; i++) {
		if(file_read || getline(&line, &size, logo) == -1) {
			sprintf(line, "%*s", logo_width,"");
			file_read = true;
		}

		line[strlen(line)-1] = 0;
		logo_width = fmax(strlen(line), logo_width);
		printf("\e[39m%s%s", color, line);
		printf("%*s", logo_width - (int)strlen(line) + LOGO_OFFSET, "");
		switch(i) {
			case 0:
				printf("\e[34m\e[1mWindow Manager\e[36m \e[22m%s%s\n", fetch_wm_name(), color);
				break;
			case 1:
				printf("\e[34m\e[1mCompositor    \e[36m \e[22m%s%s\n", fetch_comp_name(), color);
				break;
			case 2:
				printf("\e[34m\e[1mShell         \e[36m \e[22m%s%s\n", fetch_shell(), color);
				break;
  		case 3:
  			printf("\e[34m\e[1mDistribution  \e[36m \e[22m%s%s\n", fetch_distro(), color);
  			break;
			case 4:
				printf("\e[34m\e[1mKernel        \e[36m \e[22m%s%s\n", fetch_kernel(), color);
				break;
			case 5:
				printf("\e[34m\e[1mSystem Model  \e[36m \e[22m%s%s\n", fetch_system_name(), color);
				break;
			case 6:
				printf("\e[34m\e[1mTerminal      \e[36m \e[22m%s%s\n", fetch_terminal(), color);
				break;
			case 7:
				printf("\e[34m\e[1mTerminal Font \e[36m \e[22m%s%s\n", fetch_font(), color);
				break;
			case 8:
				printf("\e[34m\e[1mGTK Widget    \e[36m \e[22m%s%s\n", fetch_gtk_theme(), color);
				break;
    	case 9:
    		printf("\n");
    		break;
    	case 10:
    		printf("\e[40m   \e[41m   \e[42m   \e[43m   \e[44m   \e[45m   \e[46m   \e[47m   \e[49m\n");
    		break;
    	case 11:
    		printf("\e[100m   \e[101m   \e[102m   \e[103m   \e[104m   \e[105m   \e[106m   \e[107m   \e[49m\n");
    		break;
			case 12:
				if(file_read) run = false;
				break;
			default:
				printf("\n");
				break;
		}
	}
	printf("\n");
	free(color);
}
