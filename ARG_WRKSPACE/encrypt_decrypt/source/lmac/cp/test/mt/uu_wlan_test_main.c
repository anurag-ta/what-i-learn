#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_test_main.h" 

#define MAX_FAILED_CASES 200


uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g;

typedef struct module_test_case
{
    /* Name of the function under test (FUT), in string form */
    uu_char  *fun_to_be_tested;

    /* Test case input file in string form, for user understanding */
    uu_char  *test_input_file;

    /* Function pointer, to the test driver. Input is test-case-id,
       so that the FUT will be given suitable inputs for this test case */ 
    uu_void   (*test_driver)(uu_char*);
} module_test_case_t;

/* Structure to maintain failed test input id and the file name contains that input */
struct failed_test_list
{
    uu_char failed_case[100];
    uu_char failed_input_id[100];
}failed_test_list_g[MAX_FAILED_CASES];


/* Function to read test-input file_name and handover to testinput fun. to process
 * @param[in] file_name is a file contains set of test-input file names
 */
static uu_void tester_fun_testcategory(uu_char *file_name);

/* Function to read test_input from the file and handover to cp_test_handler 
 * @param[in] file_name is a file contains list of test_inputs to process
 */
static uu_void tester_fun_testinput(uu_char *file_name);


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
        printf("ERROR: file \"%s\" to get frame doesn't exist\n", file_name);
        uu_wlan_test_failure_cnt_g++;
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
        }
        else
        {
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
} /* uu_wlan_test_get_frame_from_file */


/** Reads file_names and handover to tester_fun_testcategory function to process.
 * @param[in] file_name is main_script contains sequence of file names based on categories.
 */
static uu_void uu_wlan_tester_fun(uu_char *file_name)
{
    FILE *fp;
    uu_char buf[FREAD_BUFF_SIZE + 1];

    fp = fopen(file_name,"r");

    if(fp == NULL)
    {
        strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_case, file_name);
        uu_wlan_test_failure_cnt_g++;
    }
    else
    {
        /* Reading line-by-line from file till end*/
        while(fgets(buf , FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else// if(strncmp(buf, "@TEST ", strlen("@TEST ")) == 0)/* Test the given file */
            {
                uu_char  str[FREAD_BUFF_SIZE+1];
                uu_char  filename[FREAD_BUFF_SIZE+1];

                sscanf(buf,"%s %s ", str, filename);
                printf("\nTesting another file %s \n", filename);
                tester_fun_testcategory(filename);
            }
        }//while
        fclose(fp);
    }
} /* uu_wlan_tester_fun */


static uu_void tester_fun_testcategory(uu_char *file_name)
{
    FILE *fp;
    uu_char buf[FREAD_BUFF_SIZE + 1];

    fp = fopen(file_name,"r");
    if(fp == NULL)
    {
        strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_case, file_name);
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: file(test_chain) for testing does not exist: %s\n", file_name);
    }
    else
    {
        /* Here we read line by line from file(test input file name) */
        while(fgets(buf , FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
            {
                uu_char  str[FREAD_BUFF_SIZE+1];
                uu_char  filename[FREAD_BUFF_SIZE+1];
                sscanf(buf,"%s %s ", str, filename);
                tester_fun_testinput(filename);
            }
        }//while
        fclose(fp);
    }
}/* tester_fun_testcategory */     


static uu_void tester_fun_testinput(uu_char *file_name)
{
    uu_int32 rq_type;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_char input_id[30];
    FILE *fp;

    printf("testinput file for testing is: %s\n", file_name);

    fp = fopen(file_name,"r");
    if(fp == NULL)
    {
        strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_case, file_name);
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: file(test_input) for testing does not exist: %s\n", file_name);
    }
    else
    {
        /* Here we read line by line from file(individual testcase id and parameters) */
        while(fgets(buf, FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else
            {
                sscanf(buf, "%s %d ", input_id, &rq_type);
                printf("test_case id %s, and rq_type %d\n", input_id, rq_type);
                if(uu_wlan_cp_test_handler(rq_type, buf) != UU_SUCCESS)
                {
                    printf("failed file name and input id are: %s %s", file_name, input_id);
                    strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g-1].failed_case, file_name);
                    strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g-1].failed_input_id, input_id);
                }
            }
        }
        fclose(fp);
    }
}/* tester_fun_testinput */

module_test_case_t  uu_wlan_test_cases_g[] =
{
    {"UURMI LMAC CP Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun},
    {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
    for (int i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
    {
        /* This is for each individual test case id in perticular function(coressponding test_input_file) */
        if (argc == 3)
        {
            if((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested)==0) &&
                    (strcmp(argv[2], uu_wlan_test_cases_g[i].test_input_file)==0))
            {
                printf("Testing function: %s, case: %s\n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested,
                    uu_wlan_test_cases_g[i].test_input_file);
                uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);
            }
        }

        /* With this we can run all testcase ids in perticular funtion(test_input_file))
           argumet 2 is function under test.
           argument 1 is exe file
         */
        else if( argc == 2)
        {  
            if((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0)
            {
                printf("Testing function: %s, case: %s\n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested,
                    uu_wlan_test_cases_g[i].test_input_file);
                uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);

            }
        }
        /* This is case run all function under test files which are maintained in queue. */
        else
        {
            printf("Testing function: %s, case: %s\n",
                uu_wlan_test_cases_g[i].fun_to_be_tested,
                uu_wlan_test_cases_g[i].test_input_file);
            uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);

            printf(" Success cases:  %d \n Failed cases:  %d\n Total no of cases:  %d \n",
                uu_wlan_test_success_cnt_g,uu_wlan_test_failure_cnt_g,
                uu_wlan_test_success_cnt_g+uu_wlan_test_failure_cnt_g);

            /* Printing of failed teest cases names at the end of execution */
            if(uu_wlan_test_failure_cnt_g)
            {
                printf("failed cases are:\n");
            }
            for(int k=0; k<uu_wlan_test_failure_cnt_g; k++)
            {
                printf("%s %s\n",
                    failed_test_list_g[k].failed_case, failed_test_list_g[k].failed_input_id);
            }
        }
    }
    return 1;
} /* main */


/* EOF */

