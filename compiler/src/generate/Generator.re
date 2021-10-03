/**
 Converts an AST into code based on the output format
 */
open Kore;

let generate = (target: Target.t, resolve: resolve_t) =>
  switch (target) {
  | JavaScript(module_type) =>
    JavaScript_Generator.generate(resolve)
    % JavaScript_Formatter.format(module_type)

  | Knot => ~@Grammar.Formatter.format % Pretty.string
  };
