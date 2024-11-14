
struct env_exception : public std::exception
{
    what()
};

struct env_context
{
    void parse(const char* const * env_raw)
    {
        for(; env_raw; env_raw++) {}
    }

    char** raw()
    {
        return nullptr;
    }

    void get(std::string_view variable_name)
    {}

    void set(std::string_view variable_name, std::string_view variable_value)
    {}

private:
    std::unordered_map<std::string_view, std::string_view> m_env_map;
    std::vector<std::string> m_env_strings;
    bool m_initialized {};
};
