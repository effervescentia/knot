open Operators;

/** these errors should never be possible, endeavour to remove the cases under which they can exist */
type invariant =
  | /** import statement references a non-existent module */
    ImportedModuleDoesNotExist(
      string,
    )
  | /** found a type reference whose value has not been determined */
    UnanalyzedTypeReference
  | /** module resolved to an invalid type */
    InvalidModuleType(string)
  | /** module path was blank */
    EmptyModulePath
  | /** sidecar scope was not provided */
    MissingSidecarScope
  | /** configuration has not been initialized */
    ConfigurationNotInitialized
  | /** did not find a boundary scope for NestedHashtbl */
    BoundaryScopeMissing;

exception Invariant(invariant);

let invariant = e => raise(Invariant(e));

let _print_inv = error =>
  ANSITerminal.(
    sprintf(
      [red],
      "[INVARIANT]: an invalid state was reached, this indicates a bug with this program and should be reported\n\n%s",
      String.capitalize_ascii(error),
    )
  )
  |> print_endline;

let print_invariant =
  (
    fun
    | ImportedModuleDoesNotExist(name) =>
      Printf.sprintf("compiler has no record of the module '%s'", name)
    | UnanalyzedTypeReference => "found a type reference whose value has not been determined"
    | InvalidModuleType(module_) =>
      Printf.sprintf("module '%s' resolved to an non-module type", module_)
    | EmptyModulePath => "empty path provided to find module"
    | MissingSidecarScope => "unable to find sidecar scope"
    | ConfigurationNotInitialized => "singleton configuration has not been initialized"
    | BoundaryScopeMissing => "NestedHashtbl did not contain a boundary scope"
  )
  % _print_inv;
