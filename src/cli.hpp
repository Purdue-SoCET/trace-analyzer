#ifndef CLI_HPP
#define CLI_HPP

class Cli {
  public:
    typedef enum {
        format_normal_e,
        format_json_e,
        format_matlab_e,
    } format_e;

    const char *trace_file;
    const char *binary_file;
    // TODO: function
    format_e format;

    Cli(int argc, const char **argv);
    ~Cli() {}
};

#endif
