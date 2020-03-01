
#pragma region GLOBAL VARIABLES
struct job
{
    int pid;
    int run_status; // 1 is running, 0 is stopped // 2 is done
    int job_order;
    int is_background; 
    char args[2000];
};
int pid_ch1, pid_ch2, ppid, job_num, status;
struct job job_array[20];
char *inString;
#pragma endregion

#pragma region FUNCTION DECLARATIONS
void setjobAsBackground(int pid);
void removeJobFromLog(int rem_index);
static void sig_ignore(int signo);
static void sig_int(int signo);
static void sig_tstp(int signo);
static void sig_int(int signo);
int addJobToLog(int is_background);
void executeChildProcess(char** args, int argc, int input_index, int output_index, int error_index, int background_index);
void processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index, int background_index, char* output_content); 
void processPipeCommand(char** init_args, int argc, int pipe_index, int background_index, char * output_content);
int getMostRecentBackground(int is_background);
void processForegroundCommand(char * output_content);
void processBackgroundCommand(char * output_content);
void processJobsCommand(char * output_content);
void printJob(int index, int is_bg, char * output_content);
void findAndPrintCompletedJobs(char * output_content);
void processStarter(char * inString, struct  job job_array[], char * process_output);
void initializeJobs(struct  job job_array[]);
void copy_arrays(struct job main_arr[], struct job to_copy_arr[]);
#pragma endregion
