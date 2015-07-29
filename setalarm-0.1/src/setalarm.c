/*
 *      Real Time Clock Driver Test/Example Program
 *      Heavily modified by Mark Rosier 2015
 *      Compile with:
 *		     gcc -s -Wall -Wstrict-prototypes setalarm.c -o setalarm
 *
 *      Copyright (C) 1996, Paul Gortmaker.
 *
 *      Released under the GNU General Public License, version 2,
 *      included herein by reference.
 *
 */

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static const char default_rtc[] = "/dev/rtc0";

int main(int argc, char **argv)
{
	int fd, retval, hours, minutes, seconds, day ;
	struct rtc_wkalrm alarm;
	struct rtc_time rtc_tm;
	struct tm tmp;
	const char *rtc = default_rtc;
	char str[80];

	switch (argc) {
	case 2:
		 strcpy(str, argv[1]);
		/* FALLTHROUGH */
	case 1:
		break;
	default:
		fprintf(stderr, "usage:  setalarm HH:MM:SS D\n");
		fprintf(stderr, "   D is weekday 0= SUN 6 = SAT\n");
		return 1;
	}

	fprintf(stderr, "input string = %s\n", str);

//	sscanf(str, "%02d:%02d:%02d %d", &hours, &minutes, &seconds, &day);

	if (	!(   (hours >= 0 && hours < 24 ) 
			&& (minutes >= 0 && minutes < 60) 
			&& (seconds >= 0 && minutes < 60) 
			&& (day >= 0 && day < 7) ) )
	{
		fprintf(stderr,"%02d:%02d:%02d %d\n", hours, minutes, seconds, day);
		fprintf(stderr, "Invalid time format \n");
		return 1;
	}

	fd = open(rtc, O_RDONLY);

	if (fd ==  -1) {
		perror(rtc);
		exit(errno);
	}

	/* Read the RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	}

	tmp.tm_year = rtc_tm.tm_year;
	tmp.tm_mon = rtc_tm.tm_mon;
	tmp.tm_mday = rtc_tm.tm_mday;
	tmp.tm_hour = rtc_tm.tm_hour;
	tmp.tm_min = rtc_tm.tm_min;
	tmp.tm_sec = rtc_tm.tm_sec;
    	mktime(&tmp);			// this will fill up the tmp.tm_wday field

	int current_secs = rtc_tm.tm_hour * 3600 +rtc_tm.tm_min * 60 + rtc_tm.tm_sec;
	int alarm_secs = hours * 3600 + minutes * 60 + seconds;

	if ((tmp.tm_wday - day == 0) && (current_secs >= alarm_secs) )
		day = 7;
	else if (tmp.tm_wday > day)
		day = day + 7 - tmp.tm_wday;
	else
		day = day - tmp.tm_wday;

	tmp.tm_mday += day;
    	mktime(&tmp);			// this will add the number of days to 
					// get correct mday 

	/* Set the alarm */
	alarm.enabled = 1;
	alarm.time.tm_sec = seconds;
	alarm.time.tm_min = minutes;
	alarm.time.tm_hour = hours;
	alarm.time.tm_year = tmp.tm_year;
	alarm.time.tm_mon =  tmp.tm_mon;
	alarm.time.tm_mday = tmp.tm_mday;
	alarm.time.tm_wday = -1;

//	fprintf(stderr,"setting alarm to %02d:%02d:%02d mday=%02d\n", alarm.time.tm_hour, alarm.time.tm_min, alarm.time.tm_sec, alarm.time.tm_mday);

	retval = ioctl(fd, RTC_WKALM_SET, &alarm);
	if (retval == -1) {
		perror("RTC_WKALM_SET ioctl");
		exit(errno);
	}

	/* Read the current alarm settings */
	retval = ioctl(fd, RTC_WKALM_RD, &alarm);
	if (retval == -1) {
		perror("RTC_ALM_READ ioctl");
		exit(errno);
	}

	fprintf(stderr, "Alarm time now set to %02d:%02d:%02d mday = %02d.\n",
		alarm.time.tm_hour, alarm.time.tm_min, alarm.time.tm_sec, alarm.time.tm_mday);

	/* Enable alarm interrupts */
	retval = ioctl(fd, RTC_AIE_ON, 0);
	if (retval == -1) {
		perror("RTC_AIE_ON ioctl");
		exit(errno);
	}

	close(fd);

	return 0;
}
