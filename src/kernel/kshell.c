#include <kernel/cpuid.h>
#include <kernel/kbd.h>
#include <kernel/kpanic.h>
#include <kernel/kshell.h>
#include <kernel/pit.h>
#include <kernel/power.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <kernel/colors.h>
#include <kernelinf.h>
#include <macros.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void prompt() { printf("PROMPT> "); }

int colorscheme(int argc, const char *argv[]){
  if(argc == 1){
    fg_color = RED;
    printf("You must specify a theme.\r\ntype ctheme help to see all themes\r\n");
    fg_color = FG;
    return 2;
  } 
  if(argc >= 3){
    fg_color = YELLOW;
    printf("Too much arguments, only the first will be used\r\n");
    fg_color = FG;
  }
  for(int i = 0; i <= sizeof(*themes); i++){ 
    if(strcmp(themes[i], (char*)argv[1])) {
      currentThemes = i;
      printf("The theme is now %s\r\n", themes[currentThemes]);
      init_color(theme_red[currentThemes], theme_dred[currentThemes], theme_green[currentThemes], theme_dgreen[currentThemes],
          theme_yellow[currentThemes], theme_dyellow[currentThemes], theme_blue[currentThemes], theme_dblue[currentThemes],
          theme_magenta[currentThemes], theme_dmagenta[currentThemes], theme_cyan[currentThemes], theme_dcyan[currentThemes], 
          theme_white[currentThemes], theme_black[currentThemes], theme_gray[currentThemes], theme_dgray[currentThemes], 
          theme_bg[currentThemes], theme_fg[currentThemes]);
      init_text(5);
      return 0;
    }
  }
  if(strcmp((char*)argv[1], "help")) {
    printf("syntax = ctheme [theme] \r\nAvaible theme = \r\n-legacy (on boot)\r\n-dark\r\n-light\r\n-witchcraft\r\n-nightsky\r\n");
    return 0;
  }
  fg_color = RED;
  printf("error\r\n");
  fg_color = FG;
  printf("the selected theme doesn't exist");
  return 1;
}

void sysfetch() {
  printf("%s\r\n", KERNEL_ARTS);

  // Value chosen to be 1 hour, 1 minute, 1 second, and 1 millisecond
  long milli = timer_ticks;
  // 3600000 milliseconds in an hour
  long hr = milli / 3600000;
  milli = milli - 3600000 * hr;
  // 60000 milliseconds in a minute
  long min = milli / 60000;
  milli = milli - 60000 * min;

  // 1000 milliseconds in a second
  long sec = milli / 1000;
  milli = milli - 1000 * sec;

  printf("\r\nKernel name: %s\r\nKernel version: %s\r\nKernel build date: %s "
         "%s\r\nUptime: %u hours %u minutes %u seconds\r\nCPU model number: "
         "%u\r\n",
         KERNEL_NAME, KERNEL_VERS, KENREL_DATE, KERNEL_TIME, hr, min, sec,
         get_model());

#ifdef __clang__
  printf("Compiler version: clang %d.%d.%d\r\n", __clang_major__,
         __clang_minor__, __clang_patchlevel__);
#else
  printf("Compiler version: gcc %d.%d.%d\r\n", __GNUC__, __GNUC_MINOR__,
         __GNUC_PATCHLEVEL__);
#endif

  bg_color = RED;
  printf(" ");
  bg_color = GREEN;
  printf(" ");
  bg_color = YELLOW;
  printf(" ");
  bg_color = BLUE;
  printf(" ");
  bg_color = MAGENTA;
  printf(" ");
  bg_color = CYAN;
  printf(" ");
  bg_color = WHITE;
  printf(" ");
  bg_color = GRAY;
  printf(" ");
  bg_color = FG;
  printf(" ");

  printf("\r\n");

  bg_color = DARK_RED;
  printf(" ");
  bg_color = DARK_GREEN;
  printf(" ");
  bg_color = DARK_YELLOW;
  printf(" ");
  bg_color = DARK_BLUE;
  printf(" ");
  bg_color = DARK_MAGENTA;
  printf(" ");
  bg_color = DARK_CYAN;
  printf(" ");
  bg_color = BLACK;
  printf(" ");
  bg_color = DARK_GRAY;
  printf(" ");
  bg_color = BG;
  printf(" ");

  bg_color = BG;
  printf("\r\n\n");
}

int kshell(void *mbi, unsigned long magic) {
  while (true) {
    prompt();

    char *args = gets();
    char line[strlen(args) + 1];
    const char *argv[wspaceamount(args) + 1];
    int argc = 0;

    for (int i = 0; i < strlen(args) + 1; i++) {
      line[i] = args[i];
    }

    argv[argc] = strtok(line, " ");
    while (argv[argc]) {
      argc++;
      argv[argc] = strtok(0, " ");
    }

    if (check_cmd("hello")) {
      printf("Hello, world!\r\n");
    } else if (check_cmd("mandelbrot")) {
      if (argc == 5) {
        // int inval = false;
        // for (int i = 1; i != 4; i++)
        // {
        //    if (isdigit(argv[i]) == 0)
        //    {
        //       inval = true;
        //    }
        // }
        // if (inval)
        // {
        //    printf("Invalid input! Requires float!\r\n");
        // }
        // else
        // {
        mandelbrot(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]),
                   0x000000);
        //    }
      } else if (argc > 1 && argc < 5 || argc > 5) {
        printf("%s: Invalid amount of args!\r\n", argv[0]);
      } else if (argc == 1) {
        mandelbrot(-1.95, -1.2, 2.5, 2.5, 0x000000);
      }
    } else if (check_cmd("bsod")) {
      printf("Warning this function will BSOD (Blue Screen of Death) your "
             "system. This is usually bad.\r\nContinue y/[N]: ");
      char *confirm = gets();
      for (int i = 0; confirm[i]; i++) {
        confirm[i] = tolower(confirm[i]);
      }

      if (strcmp(confirm, "yes") || strcmp(confirm, "y")) {
        kpanic("User induced BSOD", 20);
      } else {
        printf("Aborted BSOD!\r\n");
      }
    } else if (check_cmd("cls")) {
      cls();
    } else if (check_cmd("hi")) {
      for (int p = 0; p != 20; p++) {
        printf("hi\r\n");
      }
    } else if (check_cmd("cpuid")) {
      get_vendor();
    } else if (check_cmd("sysfetch")) {
      sysfetch();
    } else if (check_cmd("reboot")) {
      reboot();
    } else if (check_cmd("echo")) {
      if (argc <= 0) {
        pass;
      } else {
        for (int i = 1; i < argc; i++) {
          printf(argv[i]);
          printf(" ");
        }
        printf("\r\n");
      }
    } else if (check_cmd("ctheme")){
      colorscheme(argc, argv);
    } else if (check_cmd(0)) {
      pass;
    } else {
      fg_color = RED;
      printf("%s", argv[0]);
      fg_color = FG;
      printf(": Not a valid command!\r\n");
    }
  }

  return 0;
}
