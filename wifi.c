#include <stdio.h>
#include <time.h>
#include <iwlib.h>
#include <signal.h>

static volatile int run = 1 ;

void free_tree(wireless_scan *list);


void terminate(){
  run = 0;
}

int main(int argc, char **argv) {

  wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock, count, choice = -1, i, strenght ;
  char *ap_name;
  /* Open socket to kernel */
  sock = iw_sockets_open();

  if(argc < 2){
    puts("usage ./wifi iface");
    exit(2);
  }

  /* Get some metadata to use for scanning */
  if (iw_get_range_info(sock, argv[1], &range) < 0) {
    printf("Error during iw_get_range_info. Aborting.\n");
    exit(2);
  }


  /* Perform the scan */
  if (iw_scan(sock, argv[1], range.we_version_compiled, &head) < 0) {
    printf("Error during iw_scan. Aborting.\n");
    exit(2);
  }

  /* Traverse the results */
  result = head.result;
  count = 0;

  puts("ESSID\t\t\tPOWER");

  while (NULL != result && ++count) {
    printf("%d - %s\t\t\t%d\n",count , result->b.essid, result->stats.qual.qual);
    result = result->next;
  }
  printf("\nTotal : %d networks.", count);


  while(choice < 0 || choice > count){
    printf("\nSelect target network (1-%d)", count);
    scanf("%d",&choice);
  }
  
  result = head.result ;
  for(i = 1 ; i <choice ; i++){
    result = result->next ;
  }

  printf("Selected AP : %s", result->b.essid );
  ap_name = calloc(strlen(result->b.essid), sizeof(char) + 1);
  strcpy(ap_name, result->b.essid);

  signal(SIGINT, terminate);

  while(run){
    
    free_tree(head.result);

    if (iw_scan(sock, argv[1], range.we_version_compiled, &head) < 0){
      printf("Error during iw_scan. Aborting.\n");
      exit(2);
    }
    result = head.result ;
    while (NULL != result && ++count) {
      if(strcmp(result->b.essid, ap_name) == 0){
        printf("\n%s  %d%%", ap_name, result->stats.qual.qual);
        break;
      }
      result = result->next;
    }

  }
  
  puts("Exit...");
  free_tree(head.result);
  free(ap_name);

  exit(0);
}

void free_tree(wireless_scan *list){
  if(list->next) free_tree(list->next);
  free(list);
}
