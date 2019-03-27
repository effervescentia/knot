include Knot.Core;
include Knot.Fiber;

module M = Matchers;

type module_member =
  | ModuleImport(module_import)
  | ModuleStatement(module_statement);

let (|=) = (x, default) => opt(default, x);
