#include "ast.h"
#include "driver.hh"
#include "interpeter.h"
#include "semantic_analyzer.h"
#include "utils.h"
#include <graphviz/gvc.h>
#include <iostream>
#include <locale.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

using interp::Interpreter;

wint_t mygetwch() {
  // Save the current terminal details
  struct termios echo_allowed;
  tcgetattr(STDIN_FILENO, &echo_allowed);

  /* Change the terminal to disallow echoing - we don't
     want to see anything that we don't explicitly allow. */
  struct termios echo_disallowed = echo_allowed;
  echo_disallowed.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &echo_disallowed);

  // Get a wide character from keyboard
  wint_t wc = getwchar();

  // Allow echoing again
  tcsetattr(STDIN_FILENO, TCSANOW, &echo_allowed);

  // Return the captured character
  return wc;
}

int main(int argc, char *argv[]) {
  int res = 0;
  Driver drv;
  SemanticAnalyzer semantic_analyzer;
  Interpreter interpreter;

  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string("-p"))
      drv.trace_parsing = true;
    else if (argv[i] == std::string("-s"))
      drv.trace_scanning = true;
    else if (argv[i] == std::string("-repl")) {
      std::wcout << "Flang REPL 0.0.1" << '\n';
      std::wcout << "Type \"exit\" to exit" << '\n';
      vector<std::string> history;
      int current_history = 0;
      int cell = 0;

      setlocale(LC_ALL, "");

      while (true) {
        std::wcout << "\033[93m[" << cell++ << "]>\033[0m ";
        fflush(stdout);
        cout.flush();
        std::string input;

        while (1) {
          wint_t c = mygetwch();
          if (c == WEOF)
            break;

          else if (c == L'\n') {
            wprintf(L"\n");
            break;
          }

          // if arrow up pressed and history is not empty
          else if (c == L'\033') {
            mygetwch(); // skip [
            switch (mygetwch()) {
            case 'A':
              if (history.size() > 0 && current_history > 0) {
                --current_history;
                // remove current input
                for (int i = 0; i < input.size(); ++i)
                  wprintf(L"\b \b");
                std::wcout << history[current_history].c_str();
                input = history[current_history];
              }
              break;

            case 'B':
              if (history.size() > 0 && current_history < history.size() - 1) {
                ++current_history;
                // remove current input
                for (int i = 0; i < input.size(); ++i)
                  wprintf(L"\b \b");
                std::wcout << history[current_history].c_str();
                input = history[current_history];
              } else if (current_history == history.size() - 1) {
                ++current_history;
                // remove current input
                for (int i = 0; i < input.size(); ++i)
                  wprintf(L"\b \b");
                input = "";
              }
              break;
            }
          }

          // if \ pressed
          else if (c == 92) {
            wprintf(L"\n");
            input += '\n';
          }

          // if ctrl + c pressed
          else if (c == 3) {
            wprintf(L"Bye\n");
            remove("tmp.flang");
            return 0;
          }

          // if backspace pressed
          else if (c == 127) {
            if (input.size() > 0) {
              input.pop_back();
              wprintf(L"\b \b");
            }
          } else {

            input += c;
            wprintf(L"%lc", c);
          }
        }

        if (input == "exit") {
          std::wcout << "Bye" << '\n';
          remove("tmp.flang");
          return 0;
        }

        if (input == "")
          continue;

        history.push_back(input);
        current_history = history.size();

        FILE *tmp = fopen("tmp.flang", "w");
        fprintf(tmp, "%s", input.c_str());
        fclose(tmp);

        try {
          if (drv.parse("tmp.flang")) {
            throw std::runtime_error("Parsing failed");
          }

          semantic_analyzer.analyze(drv.ast);
          generate_graph_svg(drv.ast, "repl.svg");
          interpreter.interpret(drv.ast);
        } catch (std::exception &e) {
          std::cout << e.what() << '\n';
          semantic_analyzer.clear_stack(drv.ast);
        }

        std::cout << '\n';
      }
    } else if (!drv.parse(argv[i])) {
      std::cout << "Parsing successful" << '\n';
      semantic_analyzer.analyze(drv.ast);
      std::cout << "Semantic analysis successful" << '\n';
      generate_graph_svg(drv.ast);
      std::cout << "Graphviz file generated" << '\n';
      interpreter.interpret(drv.ast);
      std::cout << '\n';
      std::cout << "Done" << '\n';
    } else
      res = 1;
  }
  return res;
}
