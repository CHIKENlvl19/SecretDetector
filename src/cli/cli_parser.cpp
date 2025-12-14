#include "cli/cli_parser.h"
#include "utils/logger.h"
#include <iostream>
#include <algorithm>

CLIOptions CLIParser::parse(int argc, char* argv[]) {
    CLIOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            options.help = true;
        }
        else if (arg == "--version" || arg == "-v") {
            options.version = true;
        }
        else if (arg == "--path" && i + 1 < argc) {
            options.scan_path = argv[++i];
        }
        else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) {
                options.output_path = argv[++i];
            }
        }
        else if (arg == "--config" && i + 1 < argc) {
            options.config_path = argv[++i];
        }
        else if (arg == "--format" && i + 1 < argc) {
            options.format = argv[++i];
        }
        else if (arg == "--verbose") {
            options.verbose = true;
        }
        else if (arg == "--strict") {
            options.strict = true;
        }
        else if (arg == "--no-recursive") {
            options.recursive = false;
        }
        else if (arg == "--no-gitignore") {
            options.respect_gitignore = false;
        }
        else if (arg == "--threads" && i + 1 < argc) {
            try {
                options.num_threads = std::stoi(argv[++i]);
            } catch (...) {
                LOG_WARN("Invalid thread count");
            }
        }
        else if (arg == "--exclude" && i + 1 < argc) {
            options.exclude_patterns.push_back(argv[++i]);
        }
        else if (arg == "--include-ext" && i + 1 < argc) {
            options.include_extensions.push_back(argv[++i]);
        }
        else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
        }
        else if (options.scan_path.empty()) {
            // Первый позиционный аргумент - это путь для сканирования
            options.scan_path = arg;
        }
    }
    
    return options;
}

void CLIParser::printHelp() {
    std::cout << R"(
Secret Detector v1.0.0 - Find secrets in your code

USAGE:
    secret_detector [OPTIONS] [PATH]

OPTIONS:
    --path <PATH>               Path to scan (can also be positional argument)
    --output, -o <PATH>        Output file path
    --config <PATH>            Path to patterns.json config
    --format <FORMAT>          Output format: text, json, csv, html (default: text)
    --verbose                  Enable verbose logging
    --strict                   Fail on ANY match (not just CRITICAL/HIGH)
    --no-recursive             Don't scan subdirectories
    --no-gitignore             Don't respect .gitignore
    --threads <NUM>            Number of threads (0 = auto)
    --exclude <PATTERN>        Exclude pattern (can be used multiple times)
    --include-ext <EXT>        Include only these extensions (can be used multiple times)
    --help, -h                 Show this help message
    --version, -v              Show version

EXAMPLES:
    # Scan current directory
    secret_detector .

    # Scan with JSON output
    secret_detector --path /repo --format json --output report.json

    # Strict mode (fail on any match)
    secret_detector --strict /repo

    # Exclude node_modules and .git
    secret_detector --exclude node_modules --exclude .git /repo

PATTERNS SUPPORTED:
    - AWS Keys (AKIA...)
    - GitHub Tokens
    - Private Keys (RSA, DSA, EC)
    - Database URLs
    - API Keys (generic and service-specific)
    - High-entropy strings

For more info: https://github.com/CHIKENlvl19/SecretDetector
)" << std::endl;
}

void CLIParser::printVersion() {
    std::cout << "Secret Detector version " << VERSION << std::endl;
}

bool CLIParser::validate(const CLIOptions& options) {
    if (options.scan_path.empty()) {
        std::cerr << "Error: scan path is required" << std::endl;
        return false;
    }
    
    if (options.format != "text" && options.format != "json" && 
        options.format != "csv" && options.format != "html") {
        std::cerr << "Error: invalid format. Must be one of: text, json, csv, html" << std::endl;
        return false;
    }
    
    return true;
}
