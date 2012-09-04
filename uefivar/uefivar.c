/*
 * Copyright (C) 2012 Canonical
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>

#define OPT_RM		0x00000001
#define OPT_LS		0x00000002

#define OK	0
#define ERROR	1

typedef struct {
        uint16_t        varname[512];
        uint8_t         guid[16];
        uint64_t        datalen;
        uint8_t         data[1024];
        uint64_t        status;
        uint32_t        attributes;
} __attribute__((packed)) uefi_var;

#define VAR_NON_VOLATILE 	0x00000001
#define VAR_BOOTSERVICE_ACCESS	0x00000002
#define VAR_RUNTIME_ACCESS	0x00000004

static inline void uefi_set_filename(
	char *filename,
	const int len,
	const char *varname)
{
	snprintf(filename, len, "/sys/firmware/efi/vars/%s/raw_var", varname);
}

static void guid_to_str(
	const uint8_t *guid,
	char *guid_str,
	size_t guid_str_len)
{
	if (!guid_str)
		return;

	if (guid_str_len > 36)
		snprintf(guid_str, guid_str_len, 
			"%02X%02X%02X%02X-%02X%02X-%02X%02X-"
			"%02X%02X-%02X%02X%02X%02X%02X%02X",
			guid[3], guid[2], guid[1], guid[0], guid[5], guid[4], guid[7], guid[6],
		guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);
	else
		*guid_str = '\0';
}

/*
 *  uefi_str16_to_str()
 *	convert 16 bit string to 8 bit C string.
 */
static void uefi_str16_to_str(
	char *dst,
	const size_t len,
	const uint16_t *src)
{
	size_t i = len;

	while ((*src) && (i > 1)) {
		*dst++ = *(src++) & 0xff;
		i--;
	}
	*dst = '\0';
}

/*
 *  uefi_get_varname()
 *	fetch the UEFI variable name in terms of a 8 bit C string
 */
static void uefi_get_varname(
	char *varname,
	const size_t len,
	const uefi_var *var)
{
	uefi_str16_to_str(varname, len, var->varname);
}

/*
 *  uefi_get_variable()
 *	fetch a UEFI variable given its name.
 */
static int uefi_get_variable(
	const char *varname,
	uefi_var *var)
{
	int  fd;
	int  n;
	int  ret = OK;
	char filename[PATH_MAX];

	if ((!varname) || (!var))
		return ERROR;

	uefi_set_filename(filename, sizeof(filename), varname);

	if ((fd = open(filename, O_RDONLY)) < 0)
		return ERROR;

	memset(var, 0, sizeof(uefi_var));

	if ((n = read(fd, var, sizeof(uefi_var))) != sizeof(uefi_var))
		ret = ERROR;

	close(fd);

	return ret;
}

static void uefi_var_del(const char *varname)
{
	uefi_var var;
	int fd;

	if (uefi_get_variable(varname, &var) == ERROR) {
		fprintf(stderr, "No such variable %s\n", varname);
		exit(EXIT_FAILURE);
	}

	if ((fd = open("/sys/firmware/efi/vars/del_var", O_WRONLY)) < 0) {
		fprintf(stderr, "Cannot open del_var\n");
		exit(EXIT_FAILURE);
	}
	
	write(fd, &var, sizeof(var));
	
	close(fd);
}

static int true_filter(const struct dirent *d)
{
	if (strcmp(d->d_name, "del_var") == 0)
		return 0;
	if (strcmp(d->d_name, "new_var") == 0)
		return 0;
	if (strcmp(d->d_name, ".") == 0)
		return 0;
	if (strcmp(d->d_name, "..") == 0)
		return 0;
        return 1;
}

static void uefi_var_ls(void)
{
	struct dirent **names = NULL;
	int n;
	int i;

	n = scandir("/sys/firmware/efi/vars", &names, true_filter, alphasort);
	if (n == 0)
		return;

	for (i = 0; i < n; i++) {
		uefi_var var;
		char varname[513];
		char guid_str[37];

		uefi_get_variable(names[i]->d_name, &var);
		if (var.attributes) {
			uefi_get_varname(varname, sizeof(varname), &var);
			guid_to_str(var.guid, guid_str, sizeof(guid_str));
			printf("%c%c%c %4u %s %s\n",
				var.attributes & VAR_NON_VOLATILE ? 'n' : '-',
				var.attributes & VAR_BOOTSERVICE_ACCESS ? 'b' : '-',
				var.attributes & VAR_RUNTIME_ACCESS ? 'r' : '-',
				(unsigned int)var.datalen,
				guid_str, varname);
		}
		free(names[i]);
	}
	free(names);
}

static void syntax(const char *prog)
{
	fprintf(stderr, "%s options:\n", prog);
	fprintf(stderr, "\t-l\t\t\tlist variables\n");
	fprintf(stderr, "\t-r varname [varname..]\tremove variables\n");
}

int main(int argc, char **argv)
{
	uint32_t	opts = 0;
	int		opt;
	int		i;

	while ((opt = getopt(argc, argv, "lr")) != -1) {
		switch(opt) {
		case 'l':
			opts |= OPT_LS;
			break;
		case 'r':
			opts |= OPT_RM;
			break;
		default:
			syntax(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (__builtin_popcount(opts) != 1) {
		fprintf(stderr, "Must specify one of -r or -l\n");
		exit(EXIT_FAILURE);
	}

	if (opts & OPT_LS) {
		uefi_var_ls();
		exit(EXIT_SUCCESS);
	} else if (opts & OPT_RM) {
		for (i = optind; i < argc; i++)
			uefi_var_del(argv[i]);
	}

	exit(EXIT_SUCCESS);
}
