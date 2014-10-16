
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_reg.h"

#include "uu_wlan_test_main.h" 
/*macros here*/

uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g;

typedef struct module_test_case
{
   char  *fun_to_be_tested;  /* Name of the function under test (FUT), in string form */

   char  *test_input_file;      /* Test case input file in string form, for user understanding */
   int   (*test_driver)(char*); /* Function pointer, to the test driver. Input is test-case-id, so that the FUT will be given suitable inputs for this test case */
} module_test_case_t;



static FILE *duration_fp_g = NULL;
static FILE* CAPTURE_OPEN()
{
   duration_fp_g = fopen(OUTPUT_CAP_FILE, "a+");
   return duration_fp_g;
}

static uu_void CAPTURE_CLOSE()
{
   if (!duration_fp_g)
       return;
   fclose(duration_fp_g);
}

uu_void CAPTURE_STRING(uu_char *string)
{
    if (!duration_fp_g)
       return;
    fputs(string , duration_fp_g);
    fputs("  ", duration_fp_g);
    fflush(duration_fp_g);
}

uu_void CAPTURE_FRAME(uu_uchar *mpdu, uu_int32 length)
{
     uu_int32 jj;
    if (!duration_fp_g)
       return;
//return;
     for(jj = 0 ; jj < length ; jj++)
     {
          fprintf(duration_fp_g,"%x", mpdu[jj]);
          fputc('|' , duration_fp_g);
     }
     //fputc('\n' , duration_fp_g);
   fflush(duration_fp_g);
}



uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame)
{
    FILE *fp;
    uu_uchar xdigit=0;
    uu_int32 num=0;
    uu_int32 len=0;
    uu_int32 ch,frame_end=0;

    fp = fopen(file_name, "r");
    if(fp == NULL)
    {
       return UU_FAILURE;
    }

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == FRAME_END_DELIMITER) 
        {
           if (frame_end)
           {
               break;
           }
           frame_end = 1;
           continue;
        }
        if (ch == '\n') 
        {
           continue;
        }
        frame_end = 0;

        if (ch == FRAME_HEX_DELIMITER)
        {
           frame[len++] = xdigit;
           xdigit=0;
           //printf("Xdigits %x \n", frame[len-1]);
        }
        else
        {
           //sscanf(ch, "%x", &num);
           if ((ch >= 0x30) && (ch < 0x3A))
               num = ch - 0x30;
           else if ((ch == 0x41) || (ch == 0x61))
               num = 0x0A;
           else if ((ch == 0x42) || (ch == 0x62))
               num = 0x0B;
           else if ((ch == 0x43) || (ch == 0x63))
               num = 0x0C;
           else if ((ch == 0x44) || (ch == 0x64))
               num = 0x0D;
           else if ((ch == 0x45) || (ch == 0x65))
               num = 0x0E;
           else if ((ch == 0x46) || (ch == 0x66))
               num = 0x0F;
           else 
               num = 0x00;

           xdigit = (xdigit<<4) | (num&0x0f);
        }
    }
    fclose(fp);
    return len;
}



int uu_wlan_tester_fun(char *file_name)
{
    FILE *fp;
    uu_int32 test_category;
    uu_char case_id[FREAD_BUFF_SIZE+1];
    uu_int32 ret = UU_SUCCESS;
    uu_int32 ii;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_char *c;

    fp = fopen(file_name,"r");
    c = (char *)buf;
    if(fp == NULL)
    {
        printf("its at fp\n");
        return UU_FAILURE;
    }

    for(ii=0; c != NULL ; ii++)
    {
        c= fgets(buf , FREAD_BUFF_SIZE, fp);/*here we read line by line from file(individual testcase id and parameters) */
        if(c == NULL)/*till the end of file we are reading*/
        {
           break;
        }

            if(buf[0] == '#')/*if line from file starting with # then its treated as comment*/
            {
                //printf("comment:%s \n",buf);
            }
            else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
            {
		        uu_char  str[FREAD_BUFF_SIZE+1];
        		uu_char  filename[FREAD_BUFF_SIZE+1];

               sscanf(buf,"%s %s ", str, filename);
               printf("\nTesting another file %s \n", filename);
               ret = uu_wlan_tester_fun(filename); 
            }
            else
            {
               sscanf(buf,"%s %d ",case_id, &test_category);
                printf("\n\n**********************\n");
                printf("START OF TESTING CASE %s\n", case_id);
                ret = uu_wlan_duration_test_handler(test_category, buf);
                if(ret >= 0)
                {
                    printf("TEST CASE IS SUCCESS\n");
                }
                else
                {
                    printf("TEST CASE IS FAILURE\n");
                }
                printf("END OF TESTING CASE %s\n", case_id);
                printf("**************************\n\n");
            }
    }

    return ret;
}



module_test_case_t  uu_wlan_test_cases_g[] =
{
     {"UURMI LMAC DURATION Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun},
   {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
   int i;

   if(CAPTURE_OPEN() == NULL)
   {
      printf("Error: Capture file %s Open failed.\n", OUTPUT_CAP_FILE);
      return 1;
   }
   CAPTURE_STRING("\n\n************************* Duration Test Capture START *************************\n\n");
   for (i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
   {
      /*this is for each individual test case id in perticular function(coressponding test_input_file)*/
      if (argc == 3)
      {
         if(((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0)&&((strcmp(argv[2], uu_wlan_test_cases_g[i].test_input_file))==0))
         {
            printf("Testing function: %s, case: %s, result: %d\n",
                  uu_wlan_test_cases_g[i].fun_to_be_tested,
                  uu_wlan_test_cases_g[i].test_input_file,
                  uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file));

         }
      }
      /*with this we can run all testcase ids in perticular funtion(test_input_file))
        argumet 2 is function under test.
        argument 1 is exe file
       */
      
      else if( argc == 2)
      {  
         if((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0)
         {
            printf("Testing function: %s, case: %s, result: %d\n",
                  uu_wlan_test_cases_g[i].fun_to_be_tested,
                  uu_wlan_test_cases_g[i].test_input_file,
                  uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file));

         }
      }
       /*this is case run all function under test files which are maintained in queue.*/
      else
      {
         printf("Testing function: %s, case: %s, return_value %d \n",
               uu_wlan_test_cases_g[i].fun_to_be_tested,
               uu_wlan_test_cases_g[i].test_input_file,
               uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file)
               );
        printf(" Success cases:  %d \n Failed cases:  %d\n Total no of cases:  %d \n", uu_wlan_test_success_cnt_g,uu_wlan_test_failure_cnt_g, uu_wlan_test_success_cnt_g+uu_wlan_test_failure_cnt_g);
      }
   }
   CAPTURE_CLOSE();
   return 1;
} /* main */


/* EOF */

