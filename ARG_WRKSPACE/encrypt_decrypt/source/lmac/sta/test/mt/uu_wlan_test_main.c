#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_cp_ba_sb.h"
#include "uu_wlan_sta_test_main.h"


/* Counters for number of success & failed test cases. */
static uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g;

/*
 * Added here, as 'lmac_init' is initializing this lock.
 * This is defined in CP module of LMAC, which is not included in our testing.
 */
uu_wlan_spin_lock_type_t uu_wlan_ba_ses_context_lock_g;


uu_int32 uu_wlan_beacon_timer_start( uu_void)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_beacon_timer_stop(uu_void)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_cap_init(uu_void)
{
    return UU_SUCCESS;
}

uu_void uu_wlan_cap_shutdown(uu_void)
{
}



typedef struct module_test_case
{
    char  *fun_to_be_tested;  /* Name of the function under test (FUT), in string form */

    char  *test_input_file;      /* Test case input file in string form, for user understanding */
    int   (*test_driver)(char*); /* Function pointer, to the test driver. Input is test-case-id, so that the FUT will be given suitable inputs for this test case */
} module_test_case_t;


int uu_wlan_tester_fun(char *file_name)
{
    FILE *fp;
    uu_int32 rq_type;
    uu_int32 case_id;
    uu_int32 ret = UU_SUCCESS;
    uu_int32 ii, jj;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_char *c;
    unsigned char string[3][30];

    printf("-------------Starting of test case-----------\n");

    fp = fopen(file_name,"r");
    if(fp == NULL)
    {
        uu_wlan_test_failure_cnt_g++;
        printf("its at fp\n");
        return UU_FAILURE;
    }

    /* here we read line by line from file(individual testcase id and parameters) */
    for(; c= fgets(buf, FREAD_BUFF_SIZE, fp); )
    {
        if(c == NULL)/*till the end of file we are reading*/
        {
            break;
        }

        if(buf[0] == '#')/*if line from file starting with # then its treated as comment*/
        {
            //printf("comment:%s \n", buf);
        }
        else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
        {
            uu_char  str[FREAD_BUFF_SIZE+1];
            uu_char  filename[FREAD_BUFF_SIZE+1];

            sscanf(buf,"%s %s ", str, filename);
            printf("\nTesting another file %s \n", filename);
            ret = uu_wlan_tester_fun(filename);
            //printf("return value after tester fun %d \n", ret);
        }
        else
        {
            //sscanf(buf,"%s %d %s %d ", string[0], &case_id, string[1], &rq_type);
            //printf("test_case id %d, and rq_type %d\n", case_id, rq_type);
            ret = uu_wlan_sta_test_handler(buf);
        }
    } /* for */

    if(strncmp(file_name, "sta_testfiles/", 13) == 0)
    {
        if (ret != 0)
        {
            uu_wlan_test_failure_cnt_g++;
            printf("-------------End of test case-----------\n");
        }
        else
        {
            uu_wlan_test_success_cnt_g++;
            printf("-------------End of test case-----------\n");

        }
    }
    return ret;
} /* uu_wlan_tester_fun */


module_test_case_t  uu_wlan_test_cases_g[] =
{
    {"UURMI LMAC STA Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun},
    {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
    int i, ret;
    uu_wlan_test_failure_cnt_g = 0;
    uu_wlan_test_success_cnt_g = 0;

    for (i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
    {
        /*this is for each individual test case id in perticular function(coressponding test_input_file)*/
        if (argc == 3)
        {
            if(((strcmp(argv[1], uu_wlan_test_cases_g[i].fun_to_be_tested)) == 0) && ((strcmp(argv[2], uu_wlan_test_cases_g[i].test_input_file)) == 0))
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
            if((strcmp(argv[1], uu_wlan_test_cases_g[i].fun_to_be_tested)) == 0)
            {
                printf("Testing function: %s, case: %s, result: %d\n",                                                                                                                         uu_wlan_test_cases_g[i].fun_to_be_tested,
                        uu_wlan_test_cases_g[i].test_input_file,
                        uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file));
            }
        }
        /*this is case run all function under test files which are maintained in queue.*/
        else
        {
            printf("ELSE Testing function: %s, case: %s, return_value %d \n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested,
                    uu_wlan_test_cases_g[i].test_input_file,
                    uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file));
        }
        printf(" Success cases:  %d \n Failed cases:  %d\n Total no of cases:  %d \n", uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g, uu_wlan_test_success_cnt_g+uu_wlan_test_failure_cnt_g);
    }

    return 1;
} /* main */


