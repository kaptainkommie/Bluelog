/*
 *  livelog - Simple CGI module for Bluelog Live mode
 * 
 *  Written by Tom Nardi (MS3FGX@gmail.com), released under the GPLv2.
 *  For more information, see: www.digifail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>

// Defines
#define APPNAME "livelog.cgi"
#define VERSION "0.6"
#define MAXNUM 4096
#define MAXLINE 500
#define INFO "/tmp/info.txt"
#define LOG "/tmp/live.log"
#define PID_FILE "/tmp/bluelog.pid"

// Conditionals
#if defined OPENWRT || PWNPLUG
#define CSSPREFIX "/bluelog/"
#else
#define CSSPREFIX "/"
#endif

// Found device struct
struct btdev
{
	// Probably do something more with this eventually...
	char line[MAXLINE];
};

// Global variables
// Device file
FILE *logfile;
// Status file
FILE *infofile; 
// Init device cache, index
struct btdev dev_cache[MAXNUM];
int device_index = 0;

void PrintHeader(char *CSSFILE)
{
	// CGI header
	puts("Content-type: text/html\n\n");
	// Boilerplate
	printf("<!--This file created with %s (v%s) by MS3FGX-->\n", APPNAME, VERSION);
	// HTML head
	printf("<html><head><link href=\"%s%s\" type=\"text/css\" rel=\"stylesheet\" /></head><body>\n", CSSPREFIX,CSSFILE);
}
void SetupTable(int mobile)
{
	// Table setup
	puts("<table border=\"1\" cellpadding=\"5\" cellspacing=\"5\" width=\"100%\">\n");
	
	if (!mobile)
	{
		puts("<tr>\n"\
		"<th>Time Discovered</th>\n"\
		"<th>MAC Address</th>\n"\
		"<th>Device Name</th>\n"\
		"<th>Device Class</th>\n"\
		"<th>Capabilities</th>\n"\
		"</tr>\n");
	}
	else
	{
		puts("<tr>\n"\
		"<th>Time</th>\n"\
		"<th>MAC</th>\n"\
		"<th>Name</th>\n"\
		"<th>Class</th>\n"\
		"</tr>\n");
	}
}

void PrintInfo(void)
{
	// Line length
	char line [128];
	
	while (fgets(line,sizeof line,infofile))
		fputs(line,stdout);
}

void ReadResults(void)
{
	// Line length
	char line [MAXLINE];
	
	while (fgets(line,sizeof line,logfile))
	{
		strcpy(dev_cache[device_index].line,line);
		device_index++;
	}
}

void PrintResults(void)
{
	while (device_index > 0)
	{	
		printf("%s\n",dev_cache[device_index-1].line);
		device_index--;
	}
}

int ReadPID (void)
{
	// Any error will return 0
	FILE *pid_file;
	int pid;

	if (!(pid_file=fopen(PID_FILE,"r")))
		return 0;
		
	if (fscanf(pid_file,"%d", &pid) < 0)
		pid = 0;

	fclose(pid_file);	
	return pid;
}

void TopBar (void)
{
	// Discovered devices display
	puts("<div id=\"content\">\n");
	printf("Discovered Devices: %i</div>\n",device_index);
	// Close up info pane
	puts("</div>\n");	
}

void SideBar(void)
{	
	// Start sidebar, Info pane
	puts("<div id=\"sidebar\">\n"\
	"<div id=\"sideobject\">\n"\
	"<div id=\"boxheader\">Info</div>\n"\
	"<div id=\"sidebox\">\n"\
	"<div id=\"sidecontent\">");
	
	// Populate sidebar from files
	PrintInfo();
	
	// Close up info pane
	puts("</div>\n"\
	"</div>\n"\
	"</div>\n");
	
	// Status pane
	puts("<div id=\"sideobject\">\n"\
	"<div id=\"boxheader\">Status</div>\n"\
	"<div id=\"sidebox\">\n"\
	"<div id=\"sidecontent\">");
	
	// Print current PID status
	puts("<div class=\"sideitem\">");
	puts("Bluelog: ");	
	if (ReadPID() != 0)
		puts("<span style=\"color: #00ff00;\"><b>RUNNING</b></span></div>\n");
	else
		puts("<span style=\"color: #ff0000;\"><b>STOPPED</b></span></div>\n");	
	
	// Print number of discovered devices
	puts("<div class=\"sideitem\">");
	printf("Discovered Devices: %i</div>\n",device_index);
	
	// Close up info pane
	puts("</div>\n"\
	"</div>\n"\
	"</div>\n"\
	"</div>\n");
}

void ShutDown(void)
{
	// Close files
	fclose(infofile);
	fclose(logfile);
	exit(1);
}

static void help(void)
{
	printf("%s (v%s) by MS3FGX\n", APPNAME, VERSION);
	printf("----------------------------------------------------------------\n");
	printf("This is a CGI module used to create a live webpage of Bluelog\n"
		"results. This module simply parses the log files created by\n"	
		"Bluelog, it has no scanning or logging capability of its own.\n");
	printf("\n");
	printf("For more information, see www.digifail.com\n");
	printf("\n");
	printf("Options:\n"
		"\t-m              Mobile format\n"
		"\t-h              Display help\n"
		"\t-d              Print Debug Info\n"
		"\t-v              Print Version Info\n"		
		"\n");
}

static void DebugInfo(void)
{
printf("System Variables\n");
printf("--------------------------\n");
printf("Module Version: %s\n", VERSION);
printf("Max Devices: %i\n",MAXNUM);
printf("Max Line Length: %i\n",MAXLINE);
printf("\n");
printf("File Locations\n");
printf("--------------------------\n");
printf("Info File: %s\n",INFO);
printf("Log File: %s\n", LOG);
printf("CSS Prefix: %s\n",CSSPREFIX);
}

static struct option main_options[] = {
	{ "help", 0, 0, 'h' },
	{ "debug", 0, 0, 'd' },
	{ "version", 0, 0, 'v' },
	{ "mobile", 0, 0, 'm' },
	{ 0, 0, 0, 0 }
};
 
int main(int argc, char *argv[])
{		
	// Variables	
	// Default CSS
	char CSSFILE[12]="style.css";

	// Handle arguments
	int opt;
	int mobile = 0;
	while ((opt=getopt_long(argc, argv, "dvhm", main_options, NULL)) != EOF)
	{
		switch (opt)
		{
		case 'h':
			help();
			exit(0);
		case 'd':
			DebugInfo();
			exit(0);
		case 'v':
			printf("%s (v%s) by MS3FGX\n", APPNAME, VERSION);
			exit(0);
			break;
		case 'm':
			strcpy(CSSFILE, "mobile.css");
			mobile = 1;
			break;
		default:
			printf("Unknown option.\n");
			exit(0);
		}
	}
	
	// Bail out if we are root, except on WRT
	#ifndef OPENWRT
	if(getuid() == 0)
	{
		syslog(LOG_ERR,"CGI module refusing to run as root!");
		printf("Server attempting to run CGI module as root, bailing out!\n");
		printf("Check your web server configuration and try again.\n");
		exit(1);
	}
	#endif

	// Pointers to filenames
	char *infofilename = INFO;
	char *logfilename = LOG;
	
	// File header
	PrintHeader(CSSFILE);
	
	// Start container div
	if (!mobile)
		puts("<div id=\"container\">\n");

	// Open files
	if ((infofile = fopen(infofilename, "r")) == NULL)
	{
		syslog(LOG_ERR,"Error while opening %s!",infofilename);
		puts("<div id=\"content\">");
		printf("Error while opening %s!\n",infofilename);
		puts("</body></html>");
		exit(1);
	}
	
	if ((logfile = fopen(logfilename, "r")) == NULL)
	{
		syslog(LOG_ERR,"Error while opening %s!",logfilename);
		puts("<div id=\"content\">");
		printf("Error while opening %s!\n",logfilename);
		puts("</body></html>");
		exit(1);
	}
	
	// Draw sidebar\topbar
	ReadResults();
	if (!mobile)
		SideBar();
	else
		TopBar();
	
	// Content window
	puts("<div id=\"content\">");
	
	if (device_index > 0)
	{
		// Print results
		SetupTable(mobile);
		PrintResults();
		puts("</table>\n");
	}
	
	// Close content, body, and HTML
	puts("</div></body></html>");
	
	// Close files and exit
	ShutDown();
	return 0;
}

