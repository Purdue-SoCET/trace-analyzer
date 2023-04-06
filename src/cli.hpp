#ifndef CLI_HPP
#define CLI_HPP

/*
 * Class to handle parsing CLI arguments.
 */
class Cli {
  public:
    // List of output formats which are supported.
    typedef enum {
        format_normal_e,
        format_json_e,
        format_matlab_e,
    } format_e;

    // Path to the trace log
    const char *trace_file;
    // Path to the binary file
    const char *binary_file;
    // TODO: function
    // Format to be printed, defaults to normal
    format_e format;

    // Constructor for CLI. Should be passed argc and argv straight from main.
    // int argc: number of arguments
    // const char **argv: List of arguments
    Cli(int argc, const char **argv);
    ~Cli() {}
};

#endif
